//----------------------------------------------------------------------------//
//  Copyright 2021 Clemens Treichler                                          //
//                                                                            //
//  This file is part of ESTL - Embedded Systems Tiny Library,                //
//  see <https://github.com/treichler/ESTL>                                   //
//                                                                            //
//  ESTL is free software: you can redistribute it and/or modify              //
//  it under the terms of the GNU Lesser General Public License as published  //
//  by the Free Software Foundation, either version 3 of the License, or      //
//  (at your option) any later version.                                       //
//                                                                            //
//  ESTL is distributed in the hope that it will be useful,                   //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of            //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              //
//  GNU Lesser General Public License for more details.                       //
//                                                                            //
//  You should have received a copy of the GNU Lesser General Public License  //
//  along with ESTL. If not, see <http://www.gnu.org/licenses/>.              //
//----------------------------------------------------------------------------//

/**
 * @file Terminal.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Types.h"
#include "ESTL_Config.h"
#include <string.h>
#include "Error.h"
#include "Parse.h"
#include "Print.h"
#include "Parameter.h"
#include "Debug.h"
#include "Scope.h"
#include "Terminal.h"


#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
#include "Target.h"  // XXX needed by CANopen.h due to CAN_MSG_OBJ_t type
#include "CANopen.h"
#include "Parameter_CANopen.h"
#endif


/**
 * Structure containingTerminal's local static variables
 */
struct {
  const terminal_t * terminals;
  uint8_t number_of_terminals;
#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
  uint8_t can_open_buffer[ESTL_TERMINAL_PARAMETER_CANOPEN_BUFFER_SIZE];
  uint8_t can_open_node_id;
  range_t can_open_index_range;
#endif
} Terminal_Data;


//-------------------------------------------------------------------------------------------------
//  Local function prototypes
//-------------------------------------------------------------------------------------------------

void Terminal_PrintParameterDetails( const terminal_t * terminal, parameter_data_t * parameter_data, int32_t value, const char * info );
void Terminal_ParameterNotFoundMessage( const terminal_t * terminal, char *rx_buffer );
void Terminal_ReadErrorMessage( const terminal_t * terminal, error_code_t error );
#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
error_code_t Terminal_InitCanOpenTable( uint8_t node_id );
int16_t Terminal_CanOpenFindIndexByName( char * parameter_name );
#endif


void Terminal_Init( const terminal_t * terminals, uint8_t number_of_terminals )
{
  Terminal_Data.terminals = terminals;
  Terminal_Data.number_of_terminals = number_of_terminals;
}


/**
 * This function is similar to a standard printf() function.
 * It also takes the currently processed terminal as an argument to forward
 * the output to the related stream.
 *
 * @param     terminal         The terminal where the message has to be printed.
 * @param     fmt              Format string.
 * @param     va_list          variables list.
 */
void Terminal_printf(const terminal_t * terminal_data, const char *fmt, ...)
{
  va_list va;
  va_start( va, fmt );
  Print_Format( NULL, terminal_data->transmitFunction, NULL, fmt, va );
  va_end( va );
}


void Terminal_Task(void)
{
  uint16_t terminal_index;
  for( terminal_index = 0; terminal_index < Terminal_Data.number_of_terminals; terminal_index ++ )
  {
    const terminal_t * terminal = &(Terminal_Data.terminals[terminal_index]);
//    char * rx_buffer;
    char *rx_buffer, *argument;

    // check if new data is available
    if( terminal->ReceivedNewLine( &rx_buffer ) )
//    if( terminal->receivedNewData() )
    {
      int16_t parameter_index;
//      char *rx_buffer, *argument;

      // split string to command and argument
//      rx_buffer = argument = terminal->receive_buffer;
      argument = rx_buffer;
      while( (*argument != ' ') && (*argument != '\0') )
        argument ++;
      if( *argument == ' ' )
      {
        *argument = '\0';
        argument ++;
      }
//      Terminal_printf(terminal, "command: %s\targument: %s\r\n", rx_buffer, argument);

#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
      // check if we received 'CANopen-ID'
      if( 0 == strcmp(rx_buffer, "CANopen-ID") )
      {
        int32_t value;
        error_code_t error_code;
        if( *argument == '\0' )
          Terminal_printf(terminal, "0x%02X\r\n", Terminal_Data.can_open_node_id);
        else
        {
          value = Parse_StrToValue(argument);
          if( value == 0 )
          {
            Terminal_Data.can_open_node_id = 0;
            Terminal_printf(terminal, "OK\r\n");
          }
          else if( value <= 127 )
          {
            // try to fetch parameter table
            error_code = Terminal_InitCanOpenTable( (uint8_t)value );
            if( OK != error_code )
            {
#ifdef ESTL_ENABLE_ERROR_MESSAGES
              Terminal_printf(terminal, "Could not fetch parameter from node 0x%02X: %s (error %d)\r\n", value, Error_GetMessage(error_code), error_code);
#else
              Terminal_printf(terminal, "Could not fetch parameter from node %0x%02X (error %d)\r\n", value, error_code);
#endif
            }
            else
              Terminal_printf(terminal, "OK\r\n");
          }
          else
            Terminal_ReadErrorMessage(terminal, ABOVE_LIMIT);
        }
        return;
      } // we received 'CANopen-ID'

      // check if we received 'CANopen-scan'
      if( 0 == strcmp(rx_buffer, "CANopen-scan") )
      {
        uint8_t i;
        char node_seg_buffer[32];
        Terminal_printf(terminal, "Scanning 127 CANopen nodes...\r\n");
        for( i = 1; i <= 127; i ++ )
        {
          // read SDO CANopen name
          CANopen_SegRead( i, 0x1008, 0x00, node_seg_buffer, sizeof(node_seg_buffer) );
          while( !(CANopen_getState() == CANopen_SDOC_Succes || CANopen_getState() == CANopen_SDOC_Fail) );
          if( CANopen_getState() == CANopen_SDOC_Succes )
          {
            Terminal_printf(terminal, "0x%02X: %s", i, node_seg_buffer);
            // read SDO CANopen Firmware revision
            CANopen_SegRead( i, 0x100A, 0x00, node_seg_buffer, sizeof(node_seg_buffer) );
            while( !(CANopen_getState() == CANopen_SDOC_Succes || CANopen_getState() == CANopen_SDOC_Fail) );
            if( CANopen_getState() == CANopen_SDOC_Succes )
              Terminal_printf(terminal, ", %s", node_seg_buffer);
            Terminal_printf(terminal, "\r\n");
          }
        }
        Terminal_printf(terminal, "...done.\r\n");
        return;
      } // we received 'CANopen-scan'
#endif // ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER

      // check if we received 'help'
      if( 0 == strcmp(rx_buffer, "help") )
      {
        if( *argument == '\0' )
        {
          int16_t i;
          range_t index_range;
#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
          Terminal_printf(terminal, "CANopen-scan: Scan CANopen network for available nodes\r\n");
          Terminal_printf(terminal, "CANopen-ID:   Connect to ID's parameter interface\r\n");
#endif
#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
          Terminal_printf(terminal, "scope: print scope's buffer content\r\n");
#endif
#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
          if( 0 == Terminal_Data.can_open_node_id )
          {
#endif
            Terminal_printf(terminal, "Built in parameters -- type 'help <parameter>' to get detailed information\r\n");
            index_range = Parameter_GetIndexRange();
            for( i = index_range.min; i <= index_range.max; i ++ )
            {
              parameter_data_t parameter_data;
              if( OK == Parameter_ReadData(i, &parameter_data) )
                Terminal_printf(terminal, "%s\r\n", parameter_data.name);
            }
#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
          }
          else
          {
            Terminal_printf(terminal, "CANopen node 0x%02X parameters -- type 'help <parameter>' to get detailed information\r\n", Terminal_Data.can_open_node_id);
            parameter_data_t * parameter_data = (parameter_data_t*)Terminal_Data.can_open_buffer;
            for( i = Terminal_Data.can_open_index_range.min; i <= Terminal_Data.can_open_index_range.max; i ++ )
            {
              // TODO print parameter according to access level and visibility
              printf( "%s\r\n", parameter_data->name );
              parameter_data ++;
            }
          }
#endif
        }
        else
        {
          // print help for dedicated parameter
          parameter_data_t parameter_data;
          error_code_t parameter_read_status;

#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
          if( 0 == Terminal_Data.can_open_node_id )
          {
#endif
            // try to find argument in local parameter list
            parameter_index = Parameter_FindIndexByName(argument);
            if( ! Parameter_IndexExists(parameter_index) )
              Terminal_ParameterNotFoundMessage(terminal, argument);
            else
            {
              parameter_read_status = Parameter_ReadData(parameter_index, &parameter_data);
              if( parameter_read_status != OK )
                Terminal_ReadErrorMessage(terminal, parameter_read_status);
              else
                Terminal_PrintParameterDetails( terminal, &parameter_data, Parameter_GetValue(parameter_index), Parameter_GetHelp(parameter_index) );
              Terminal_printf(terminal, "\r\n");
            }
#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
          }
          else
          {
            // try to find argument in CANopen parameter list
            parameter_index = Terminal_CanOpenFindIndexByName( argument );
            if( ! ValueInRange(parameter_index, Terminal_Data.can_open_index_range) )
              Terminal_ParameterNotFoundMessage(terminal, argument);
            else
            {
//              parameter_read_status = Parameter_ReadData(parameter_index, &parameter_data);
//              if( parameter_read_status != OK )
//                Terminal_ReadErrorMessage(terminal, parameter_read_status);
//              else
              char info[256];
              error_code_t error_code;
              int32_t value;
              parameter_data_t * parameter_data = &((parameter_data_t*)Terminal_Data.can_open_buffer)[parameter_index - Terminal_Data.can_open_index_range.min];
              error_code = Parameter_CANopen_ReadInfo( Terminal_Data.can_open_node_id, parameter_index, info, sizeof(info) );
              if( OK != error_code )
                Terminal_ReadErrorMessage(terminal, error_code);
              error_code = Parameter_CANopen_ReadValue( Terminal_Data.can_open_node_id, parameter_index, &value );
              if( OK != error_code )
                Terminal_ReadErrorMessage(terminal, error_code);

              // TODO print parameter according to access level and visibility and read-success
              Terminal_PrintParameterDetails( terminal, parameter_data, value, info );
              Terminal_printf(terminal, "\r\n");
            }
          }
#endif
        }
        return;
      } // we received 'help'

#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
      // check if we received 'scope'
      if ( 0 == strcmp(rx_buffer, "scope") )
      {
        uint16_t i, sample;
        scope_sample_t * scope_sample;
        Terminal_printf(terminal, "Scope content:\r\n");
        for( sample = 0; sample < ESTL_SCOPE_NR_OF_SAMPLES; sample ++ )
        {
          scope_sample = Scope_GetSample(sample);
          for(i = 0; i < ESTL_DEBUG_NR_OF_ENTRIES; i ++)
          {
            if( i > 0 )
              Terminal_printf( terminal, "\t" );
            Terminal_printf(terminal, "%d", scope_sample->channel[i] );
//            Parameter_WriteValue(PARAM_D_INDEX, i);
//            Parameter_ValueToString(PARAM_D_DATA, parameter_data.value, value_str);
          }
          Terminal_printf( terminal, "\r\n" );
        }
        Terminal_printf( terminal, "\r\n" );
        return;
      } // we received 'scope'
#endif // ESTL_ENABLE_SCOPE && ESTL_ENABLE_DEBUG

#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
      range_t range;
      if( 0 == Terminal_Data.can_open_node_id )
      {
        // find command in parameter list
        parameter_index = Parameter_FindIndexByName( rx_buffer );
        range = Parameter_GetIndexRange();
      }
      else
      {
        // find command in CANopen parameter list
        parameter_index = Terminal_CanOpenFindIndexByName( rx_buffer );
        range = Terminal_Data.can_open_index_range;
      }
      // check index range
      if( ! ValueInRange(parameter_index, range) )
        Terminal_ParameterNotFoundMessage(terminal, rx_buffer);
#else
      // try to find command in parameter list
      parameter_index = Parameter_FindIndexByName(rx_buffer);
      if( ! Parameter_IndexExists(parameter_index) )
        Terminal_ParameterNotFoundMessage(terminal, rx_buffer);
#endif
      else
      {
        error_code_t parameter_access_status;
        // check if we have an argument
        if( *argument != '\0' )
        {
          // write parameter
          int32_t value;
          value = Parse_StrToValue( argument );
#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
          if( 0 == Terminal_Data.can_open_node_id )
            parameter_access_status = Parameter_WriteValue( parameter_index, value );
          else
            parameter_access_status = Parameter_CANopen_WriteValue( Terminal_Data.can_open_node_id, parameter_index, value );
#else
          parameter_access_status = Parameter_WriteValue( parameter_index, value );
#endif
          // print error on write fail
          if( parameter_access_status != OK )
#ifdef ESTL_ENABLE_ERROR_MESSAGES
            Terminal_printf(terminal, "Could not write parameter: %s (error %d)\r\n",
                Error_GetMessage(parameter_access_status), parameter_access_status);
#else
            Terminal_printf(terminal, "Could not write parameter (error %d)\r\n", parameter_access_status);
#endif
          else
            Terminal_printf(terminal, "OK\r\n");
        }
        else
        {
          // read parameter
          int32_t value;
          parameter_data_t parameter_data;
          parameter_data_t * parameter_data_ptr = &parameter_data;
#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )
          if( 0 == Terminal_Data.can_open_node_id )
          {
            Parameter_ReadData(parameter_index, &parameter_data);
            parameter_access_status = Parameter_ReadValue(parameter_index, &value);
          }
          else
          {
            parameter_data_ptr = &((parameter_data_t*)Terminal_Data.can_open_buffer)[parameter_index - Terminal_Data.can_open_index_range.min];
            parameter_access_status = Parameter_CANopen_ReadValue( Terminal_Data.can_open_node_id, parameter_index, &value );
          }
#else
           Parameter_ReadData(parameter_index, &parameter_data);
           parameter_access_status = Parameter_ReadValue(parameter_index, &value);
#endif
          // print error on write fail otherwise show value
          if( parameter_access_status != OK )
            Terminal_ReadErrorMessage(terminal, parameter_access_status);
          else
          {
            char value_str[16];
            Unit_PhysicalValueToString( value_str, sizeof(value_str), value, parameter_data_ptr->repr, parameter_data_ptr->unit );
            Terminal_printf(terminal, "%s\r\n", value_str);
          }
        }
      } // we received a valid parameter

    }
  }
}


/**
 * Print detailed information of a parameter to the terminal.
 *
 * @param     terminal         The terminal where the message has to be printed.
 * @param     parameter_data   Parameter properties.
 * @param     value            Current parameter value.
 * @param     info             String containing textual information to the parameter.
 */
void Terminal_PrintParameterDetails( const terminal_t * terminal, parameter_data_t * parameter_data, int32_t value, const char * info )
{
  // Print parameter info and help
  char value_str[16], minimum_str[16], maximum_str[16];
  uint16_t len, l;
  Unit_PhysicalValueToString( value_str, sizeof(value_str), value, parameter_data->repr, parameter_data->unit );
  len = strlen(parameter_data->name);
  Terminal_printf(terminal, "\r\n+");
  for( l = 0; l < (len + 4); l ++ )
    Terminal_printf(terminal, "-");
  Terminal_printf(terminal, "+\r\n|  %s  |\r\n+", parameter_data->name);
  for( l = 0; l < (len + 4); l ++ )
    Terminal_printf(terminal, "-");
  Terminal_printf(terminal, "+\r\nValue:   %s\r\n", value_str);
  if( parameter_data->flags & INFO )
  {
    Unit_PhysicalValueToString( value_str, sizeof(value_str), parameter_data->nominal, parameter_data->repr, parameter_data->unit );
    Unit_PhysicalValueToString( minimum_str, sizeof(minimum_str), parameter_data->minimum, parameter_data->repr, parameter_data->unit );
    Unit_PhysicalValueToString( maximum_str, sizeof(maximum_str), parameter_data->maximum, parameter_data->repr, parameter_data->unit );
    Terminal_printf(terminal, "Default: %s\r\nRange:   %s .. %s\r\nFlags:   0x%04X\r\n",
        value_str, minimum_str, maximum_str, parameter_data->flags);
  }
  if( info )
    Terminal_printf(terminal, "%s\r\n", info);
}


/**
 * Print a parameter-not-found message to the terminal.
 *
 * @param     terminal         The terminal where the message has to be printed.
 * @param     str              Pointer to the parameter name.
 */
void Terminal_ParameterNotFoundMessage( const terminal_t * terminal, char *str )
{
  Terminal_printf(terminal, "Parameter not found [%s]\r\n", str);
}


/**
 * Print an error message to the terminal.
 *
 * @param     terminal         The terminal where the message has to be printed.
 * @param     error            The error code to be printed.
 */
void Terminal_ReadErrorMessage( const terminal_t * terminal, error_code_t error )
{
#ifdef ESTL_ENABLE_ERROR_MESSAGES
  Terminal_printf(terminal, "Could not read parameter: %s (error %d)\r\n", Error_GetMessage(error), error);
#else
  Terminal_printf(terminal, "Could not read parameter (error %d)\r\n", error);
#endif
}


#if( defined(ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER) )

/**
 * Initialize the parameter table of a device which is connected via CANopen.
 *
 * @param     node_id          CANopen node ID.
 * @return                     Error code.
 */
error_code_t Terminal_InitCanOpenTable( uint8_t node_id )
{
  int32_t parameter_table_size, str_len, remaining_str_len;
  char * str;
  parameter_data_t * parameter_data;
  int32_t i;
  error_code_t error;
  Terminal_Data.can_open_node_id = node_id;
  error = Parameter_CANopen_ReadTableIndexRange(node_id, &Terminal_Data.can_open_index_range);
  if( OK != error )
  {
    Terminal_Data.can_open_node_id = 0;
    return error;
  }
  parameter_table_size = (Terminal_Data.can_open_index_range.max - Terminal_Data.can_open_index_range.min + 1) * sizeof(parameter_data_t);
  if( parameter_table_size > sizeof(Terminal_Data.can_open_buffer) )
  {
    Terminal_Data.can_open_node_id = 0;
    return UNKNOWN_ERROR;
  }
  // load parameter table
//  Parameter_CANopen_Data.buffer = Terminal_Data.can_open_buffer;
  parameter_data = (parameter_data_t*)Terminal_Data.can_open_buffer;
  for( i = Terminal_Data.can_open_index_range.min; i <= Terminal_Data.can_open_index_range.max; i ++)
  {
    error = Parameter_CANopen_ReadTableEntry( Terminal_Data.can_open_node_id, i, parameter_data );
    if( OK != error )
    {
      Terminal_Data.can_open_node_id = 0;
      return error;
    }
    parameter_data ++;
  }
  // load parameter names
  remaining_str_len = sizeof(Terminal_Data.can_open_buffer) - parameter_table_size;
  str = (char*)(Terminal_Data.can_open_buffer + parameter_table_size);
  parameter_data = (parameter_data_t*)Terminal_Data.can_open_buffer;
  for( i = Terminal_Data.can_open_index_range.min; i <= Terminal_Data.can_open_index_range.max; i ++)
  {
    error = Parameter_CANopen_ReadName( Terminal_Data.can_open_node_id, i, str, remaining_str_len );
    if( OK != error )
    {
      Terminal_Data.can_open_node_id = 0;
      return error;
    }
    parameter_data->name = str;
    str_len = strlen(str) + 1;
    str += str_len;
    if( remaining_str_len < str_len )
    {
      Terminal_Data.can_open_node_id = 0;
      return UNKNOWN_ERROR;
    }
    remaining_str_len -= str_len;
    parameter_data ++;
  }
  return OK;
}


/**
 * Find a parameter's index of a device which is connected via CANopen.
 * After the call of this function the returned index has to be checked
 * against can_open_index_range.
 * It is out of boundary if the parameter name could not be found.
 *
 * @param     parameter_name   Physical unit
 * @return                     Parameter's index.
 */
int16_t Terminal_CanOpenFindIndexByName( char * parameter_name )
{
  int16_t i;
  parameter_data_t * parameter_data = (parameter_data_t*)Terminal_Data.can_open_buffer;
  if( 0 == Terminal_Data.can_open_node_id )
    return INT16_MIN;
  for( i = Terminal_Data.can_open_index_range.min; i <= Terminal_Data.can_open_index_range.max; i ++ )
  {
    if( 0 == strcmp(parameter_data->name, parameter_name) )
      return i;
    parameter_data ++;
  }
  return i;
}

#endif
// ESTL_ENABLE_TERMINAL_CANOPEN_PARAMETER
