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

#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
#include "Sdo.h"
#include "Parameter_Sdo.h"
#include "ScopePdo.h"
#endif


#if( ESTL_TERMINAL_LINE_BREAK == ESTL_TERMINAL_LINE_BREAK_LF )
#define LINE_BREAK "\n"
#elif( ESTL_TERMINAL_LINE_BREAK == ESTL_TERMINAL_LINE_BREAK_CR )
#define LINE_BREAK "\r"
#elif( ESTL_TERMINAL_LINE_BREAK == ESTL_TERMINAL_LINE_BREAK_CRLF )
#define LINE_BREAK "\r\n"
#else
#error "ESTL_TERMINAL_LINE_BREAK not defined or invalid"
#endif


/**
 * Structure containingTerminal's local static variables
 */
struct {
  const terminal_t * terminals;
#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
  uint8_t can_open_buffer[ESTL_TERMINAL_REMOTE_PARAMETER_BUFFER_SIZE]; // this buffer needs to be aligned to 32-bit address
  uint32_t table_crc;
  range_t can_open_index_range;
  scope_pdo_sample_t * scope_pdo_sample;
  bool_t scope_pdo_has_new_sample;
  uint8_t node_id;
  bool_t  is_remote;
#endif
  bool_t  scope_has_new_sample;
  uint8_t number_of_terminals;
  uint16_t last_daq_index;
  uint16_t scope_sample_index;
  scope_sample_t * scope_sample;
} Terminal_Data;


//-------------------------------------------------------------------------------------------------
//  Local function prototypes
//-------------------------------------------------------------------------------------------------

void Terminal_PrintParameterDetails( const terminal_t * terminal, parameter_data_t * parameter_data, int32_t value, const char * info );
void Terminal_ParameterNotFoundMessage( const terminal_t * terminal, char *rx_buffer );
void Terminal_PrintErrorMessage( const terminal_t * terminal, error_code_t error );
bool_t Terminal_PrintScope( uint16_t index, scope_sample_t * scope_sample );


#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
bool_t Terminal_PrintPdoScope( scope_pdo_sample_t * scope_pdo_sample );
error_code_t Terminal_InitCanOpenTable( const terminal_t * terminal, uint8_t node_id );
int16_t Terminal_CanOpenFindIndexByName( char * parameter_name );
#endif


void Terminal_Init( const terminal_t * terminals, uint8_t number_of_terminals )
{
  Terminal_Data.terminals = terminals;
  Terminal_Data.number_of_terminals = number_of_terminals;

#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
  // connect scope data print function
  Scope_Init( Terminal_PrintScope );
#endif
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

  // iterate connected terminals
  for( terminal_index = 0; terminal_index < Terminal_Data.number_of_terminals; terminal_index ++ )
  {
    const terminal_t * terminal = &(Terminal_Data.terminals[terminal_index]);
    char *rx_buffer, *argument;

#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
    // check if DAQ to be printed
    if( Terminal_Data.scope_has_new_sample )
    {
      uint16_t i;
      Terminal_printf( terminal, "0x%04X", Terminal_Data.scope_sample_index );
      for( i = 0; i < ESTL_DEBUG_NR_OF_ENTRIES; i ++ )
      {
        Terminal_printf( terminal, "\t%d", Terminal_Data.scope_sample->channel[i] );
      }
      Terminal_printf( terminal, LINE_BREAK );
      Terminal_Data.scope_has_new_sample = FALSE;
    }
#endif

#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
    // check if remote DAQ to be printed
    if( Terminal_Data.scope_pdo_has_new_sample )
    {
      uint8_t i;
      Terminal_printf( terminal, "%d\t0x%04X", Terminal_Data.scope_pdo_sample->node_id, Terminal_Data.scope_pdo_sample->index );
      for( i = 0; (i < Terminal_Data.scope_pdo_sample->nr_channels) && (i < SCOPE_PDO_MAX_NR_OF_CHANNELS); i ++ )
      {
        if( (1 << i) & Terminal_Data.scope_pdo_sample->validity_bits )
          Terminal_printf( terminal, "\t%d", Terminal_Data.scope_pdo_sample->sample[i] );
        else
          Terminal_printf( terminal, "\t##" );
      }
      Terminal_printf( terminal, LINE_BREAK );
      Terminal_Data.scope_pdo_has_new_sample = FALSE;
    }
#endif

    // check if new data is available
    if( terminal->ReceivedNewLine( &rx_buffer ) )
    {
      int16_t parameter_index;

      // split string to command and argument
      argument = rx_buffer;
      while( (*argument != ' ') && (*argument != '\0') )
        argument ++;
      if( *argument == ' ' )
      {
        *argument = '\0';
        argument ++;
      }

#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
      // check if we received 'remote'
      if( 0 == strcmp(rx_buffer, "remote") )
      {
        error_code_t error_code;
        if( *argument == '\0' )
        {
          if( Terminal_Data.is_remote )
            Terminal_printf(terminal, "connected to node %d" LINE_BREAK, Terminal_Data.node_id);
          else
            Terminal_printf(terminal, "off" LINE_BREAK);
        }
        else
        {
          if( 0 == strcmp(argument, "scan") )
          {
            uint8_t i;
            uint8_t j = Sdo_GetNrOfNodes();
            char node_seg_buffer[32];
            Terminal_printf(terminal, "Scanning %d remote nodes..." LINE_BREAK, j);
            for( i = 0; i < j; i ++ )
            {
              int32_t device_type;
              uint8_t  device_type_len = 0;
              // read SDO CANopen device type
              Sdo_ExpRead( i, 0x1000, 0x00, &device_type, &device_type_len );
              while( Sdo_ReqIsBusy() );
              if( Sdo_ReqIsFinished() )
                Terminal_printf(terminal, "ID: %d\tType: 0x%08X", i, device_type);
              // read SDO CANopen name
              Sdo_SegRead( i, 0x1008, 0x00, node_seg_buffer, sizeof(node_seg_buffer) );
              while( Sdo_ReqIsBusy() );
              if( Sdo_ReqIsFinished() )
              {
                if( device_type_len )
                  Terminal_printf(terminal, "\tName: %s", node_seg_buffer);
                else
                  Terminal_printf(terminal, "ID: %d\tName: %s", i, node_seg_buffer);
                device_type_len = (uint8_t)TRUE;
                // read SDO CANopen Firmware revision
                Sdo_SegRead( i, 0x100A, 0x00, node_seg_buffer, sizeof(node_seg_buffer) );
                while( Sdo_ReqIsBusy() );
                if( Sdo_ReqIsFinished() )
                  Terminal_printf(terminal, "\tRev: %s", node_seg_buffer);
              }
              if( device_type_len )
                Terminal_printf(terminal, LINE_BREAK);
            }
            Terminal_printf(terminal, "...done." LINE_BREAK);
            return;
          }
/*
          else if( 0 == strcmp(argument, "save") )
          {
          }
*/
          else if( 0 == strcmp(argument, "off") )
          {
            Terminal_Data.is_remote = FALSE;
            Terminal_printf(terminal, "OK" LINE_BREAK);
          }
          else
          {
            uint8_t node_id = (uint8_t)Parse_StrToValue(argument);
            // try to fetch parameter table
            error_code = Terminal_InitCanOpenTable( terminal,  node_id );
            if( OK != error_code )
            {
              Terminal_Data.is_remote = FALSE;
#ifdef ESTL_ENABLE_ERROR_MESSAGES
              Terminal_printf(terminal, "Could not fetch parameter from node %d: %s (error %d)" LINE_BREAK, node_id, Error_GetMessage(error_code), error_code);
#else
              Terminal_printf(terminal, "Could not fetch parameter from node %d (error %d)" LINE_BREAK, value, error_code);
#endif
            }
            else
            {
              Terminal_Data.node_id = node_id;
              Terminal_Data.is_remote = TRUE;
              Terminal_printf(terminal, "OK" LINE_BREAK);
            }
          }
        }
        return;
      } // we received 'remote'
#endif // ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER

      // check if we received 'help'
      if( 0 == strcmp(rx_buffer, "help") )
      {
        if( *argument == '\0' )
        {
          int16_t i;
          range_t index_range;
#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
          Terminal_printf(terminal, "remote: access to remote parameter interface" LINE_BREAK \
                                    "  off:    turn off remote access" LINE_BREAK \
                                    "  scan:   look for available remote nodes" LINE_BREAK \
/*
                                    "  save:   save remote parameter to non-volatile memory" LINE_BREAK \
*/
                                    "  0..127: remote node ID to be connected to" LINE_BREAK );
#endif

#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
          if( Terminal_Data.is_remote )
          {
            Terminal_printf(terminal, "Remote node %d parameters -- type 'help <parameter>' to get detailed information" LINE_BREAK, Terminal_Data.node_id);
            parameter_data_t * parameter_data = (parameter_data_t*)Terminal_Data.can_open_buffer;
            for( i = Terminal_Data.can_open_index_range.min; i <= Terminal_Data.can_open_index_range.max; i ++ )
            {
              // TODO print parameter according to access level and visibility
              printf( "%s" LINE_BREAK, parameter_data->name );
              parameter_data ++;
            }
          }
          else
          {
#endif
            Terminal_printf(terminal, "Built in parameters -- type 'help <parameter>' to get detailed information" LINE_BREAK);
            index_range = Parameter_GetIndexRange();
            for( i = index_range.min; i <= index_range.max; i ++ )
            {
              parameter_data_t parameter_data;
              if( OK == Parameter_ReadData(i, &parameter_data) )
                Terminal_printf(terminal, "%s" LINE_BREAK, parameter_data.name);
            }
#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
          }
#endif
        }
        else
        {
          // print help for dedicated parameter
          parameter_data_t parameter_data;
          error_code_t parameter_read_status;

#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
          if( Terminal_Data.is_remote )
          {
            // try to find argument in CANopen parameter list
            parameter_index = Terminal_CanOpenFindIndexByName( argument );
            if( ! ValueInRange(parameter_index, Terminal_Data.can_open_index_range) )
              Terminal_ParameterNotFoundMessage(terminal, argument);
            else
            {
              char info[256];
              error_code_t error_code;
              int32_t value;
              parameter_data_t * parameter_data = &((parameter_data_t*)Terminal_Data.can_open_buffer)[parameter_index - Terminal_Data.can_open_index_range.min];
              error_code = ParameterSdo_ReadInfo( Terminal_Data.node_id, parameter_index, info, sizeof(info) );
              if( OK != error_code )
                Terminal_PrintErrorMessage(terminal, error_code);
              error_code = ParameterSdo_ReadValue( Terminal_Data.node_id, parameter_index, &value );
              if( OK != error_code )
                Terminal_PrintErrorMessage(terminal, error_code);

              // TODO print parameter according to access level and visibility and read-success
              Terminal_PrintParameterDetails( terminal, parameter_data, value, info );
              Terminal_printf(terminal, LINE_BREAK);
            }
          }
          else
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
                Terminal_PrintErrorMessage(terminal, parameter_read_status);
              else
                Terminal_PrintParameterDetails( terminal, &parameter_data, Parameter_GetValue(parameter_index), Parameter_GetHelp(parameter_index) );
              Terminal_printf(terminal, LINE_BREAK);
            }
#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
          }
#endif
        }
        return;
      } // we received 'help'
#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
      range_t range;
      if( Terminal_Data.is_remote )
      {
        // find command in CANopen parameter list
        parameter_index = Terminal_CanOpenFindIndexByName( rx_buffer );
        range = Terminal_Data.can_open_index_range;
      }
      else
      {
        // find command in parameter list
        parameter_index = Parameter_FindIndexByName( rx_buffer );
        range = Parameter_GetIndexRange();
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
#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
          if( Terminal_Data.is_remote )
            parameter_access_status = ParameterSdo_WriteValue( Terminal_Data.node_id, parameter_index, value );
          else
            parameter_access_status = Parameter_WriteValue( parameter_index, value );
#else
          parameter_access_status = Parameter_WriteValue( parameter_index, value );
#endif
          // print error on write fail
          if( parameter_access_status != OK )
            Terminal_PrintErrorMessage(terminal, parameter_access_status);
          else
            Terminal_printf(terminal, "OK" LINE_BREAK);
        }
        else
        {
          // read parameter
          int32_t value;
          parameter_data_t parameter_data;
          parameter_data_t * parameter_data_ptr = &parameter_data;
#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
          if( Terminal_Data.is_remote )
          {
            parameter_data_ptr = &((parameter_data_t*)Terminal_Data.can_open_buffer)[parameter_index - Terminal_Data.can_open_index_range.min];
            parameter_access_status = ParameterSdo_ReadValue( Terminal_Data.node_id, parameter_index, &value );
          }
          else
          {
            Parameter_ReadData(parameter_index, &parameter_data);
            parameter_access_status = Parameter_ReadValue(parameter_index, &value);
          }
#else
           Parameter_ReadData(parameter_index, &parameter_data);
           parameter_access_status = Parameter_ReadValue(parameter_index, &value);
#endif
          // print error on write fail otherwise show value
          if( parameter_access_status != OK )
            Terminal_PrintErrorMessage(terminal, parameter_access_status);
          else
          {
            char value_str[16];
            Unit_PhysicalValueToString( value_str, sizeof(value_str), value, parameter_data_ptr->repr, parameter_data_ptr->unit );
            Terminal_printf(terminal, "%s" LINE_BREAK, value_str);
          }
        }
      } // we received a valid parameter
    } // new data was available

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
  Terminal_printf(terminal, LINE_BREAK "+");
  for( l = 0; l < (len + 4); l ++ )
    Terminal_printf(terminal, "-");
  Terminal_printf(terminal, "+" LINE_BREAK "|  %s  |" LINE_BREAK "+", parameter_data->name);
  for( l = 0; l < (len + 4); l ++ )
    Terminal_printf(terminal, "-");
  Terminal_printf(terminal, "+" LINE_BREAK "Value:   %s" LINE_BREAK, value_str);
  if( parameter_data->flags & INFO )
  {
    Unit_PhysicalValueToString( value_str, sizeof(value_str), parameter_data->nominal, parameter_data->repr, parameter_data->unit );
    Unit_PhysicalValueToString( minimum_str, sizeof(minimum_str), parameter_data->minimum, parameter_data->repr, parameter_data->unit );
    Unit_PhysicalValueToString( maximum_str, sizeof(maximum_str), parameter_data->maximum, parameter_data->repr, parameter_data->unit );
    Terminal_printf(terminal, "Default: %s" LINE_BREAK "Range:   %s .. %s" LINE_BREAK "Flags:   0x%04X" LINE_BREAK,
        value_str, minimum_str, maximum_str, parameter_data->flags);
  }
  if( info )
    Terminal_printf(terminal, "%s" LINE_BREAK, info);
}


/**
 * Print a parameter-not-found message to the terminal.
 *
 * @param     terminal         The terminal where the message has to be printed.
 * @param     str              Pointer to the parameter name.
 */
void Terminal_ParameterNotFoundMessage( const terminal_t * terminal, char *str )
{
  Terminal_printf(terminal, "Parameter not found [%s]" LINE_BREAK, str);
}


/**
 * Print an error message to the terminal.
 *
 * @param     terminal         The terminal where the message has to be printed.
 * @param     error            The error code to be printed.
 */
void Terminal_PrintErrorMessage( const terminal_t * terminal, error_code_t error )
{
#ifdef ESTL_ENABLE_ERROR_MESSAGES
  Terminal_printf(terminal, "ERR: %s (error %d)" LINE_BREAK, Error_GetMessage(error), error);
#else
  Terminal_printf(terminal, "ERR: (error %d)" LINE_BREAK, error);
#endif
}


bool_t Terminal_PrintScope( uint16_t index, scope_sample_t * scope_sample )
{
  if( Terminal_Data.scope_has_new_sample )
    return FALSE;

  Terminal_Data.scope_sample         = scope_sample;
  Terminal_Data.scope_sample_index   = index;
  Terminal_Data.scope_has_new_sample = TRUE;
  return TRUE;
}


#if( defined(ESTL_ENABLE_TERMINAL_REMOTE_PARAMETER) )
bool_t Terminal_PrintPdoScope( scope_pdo_sample_t * scope_pdo_sample )
{
  if( Terminal_Data.scope_pdo_has_new_sample )
    return FALSE;

  Terminal_Data.scope_pdo_sample = scope_pdo_sample;
  Terminal_Data.scope_pdo_has_new_sample = TRUE;
  return TRUE;
}


/**
 * Initialize the parameter table of a device which is connected via CANopen.
 *
 * @param     terminal         The terminal where the message has to be printed.
 * @param     node_id          CANopen node ID.
 * @return                     Error code.
 */
error_code_t Terminal_InitCanOpenTable( const terminal_t * terminal, uint8_t node_id )
{
  int32_t nr_of_parameter_entries, parameter_table_size, str_len, remaining_str_len, i;
  uint32_t table_crc;
  char * str;
  parameter_data_t * parameter_data;
  error_code_t error;

  // get and check parameter table's CRC
  error = ParameterSdo_ReadTableCrc(node_id, &table_crc);
  if( OK != error )
    return error;
  if( Terminal_Data.table_crc == table_crc )
    return OK;

  // get parameter table index range
  error = ParameterSdo_ReadTableIndexRange(node_id, &Terminal_Data.can_open_index_range);
  if( OK != error )
    return error;

  nr_of_parameter_entries = Terminal_Data.can_open_index_range.max - Terminal_Data.can_open_index_range.min + 1;
  parameter_table_size = nr_of_parameter_entries * sizeof(parameter_data_t);
  if( parameter_table_size > sizeof(Terminal_Data.can_open_buffer) )
    return BUFFER_TOO_SMALL;

  // busy bar
  q15_t busy_inc = (15 * Q15_FACTOR) / nr_of_parameter_entries;
  q15_t busy_bar = Q15(0);
  Terminal_printf(terminal, "|");

  // load parameter table
//  Parameter_CANopen_Data.buffer = Terminal_Data.can_open_buffer;
  parameter_data = (parameter_data_t*)Terminal_Data.can_open_buffer;
  for( i = Terminal_Data.can_open_index_range.min; i <= Terminal_Data.can_open_index_range.max; i ++)
  {
    error = ParameterSdo_ReadTableEntry( node_id, i, parameter_data );
    if( OK != error )
      return error;

    parameter_data ++;

    busy_bar += busy_inc;
    while( q15_GetMantissa(busy_bar) )
    {
      Terminal_printf(terminal, "=");
      busy_bar -= Q15_FACTOR;
    }
  }

  // load parameter names
  remaining_str_len = sizeof(Terminal_Data.can_open_buffer) - parameter_table_size;
  str = (char*)(Terminal_Data.can_open_buffer + parameter_table_size);
  parameter_data = (parameter_data_t*)Terminal_Data.can_open_buffer;
  for( i = Terminal_Data.can_open_index_range.min; i <= Terminal_Data.can_open_index_range.max; i ++)
  {
    error = ParameterSdo_ReadName( node_id, i, str, remaining_str_len );
    if( OK != error )
      return error;

    parameter_data->name = str;
    str_len = strlen(str) + 1;
    str += str_len;
    if( remaining_str_len < str_len )
      return BUFFER_TOO_SMALL;

    remaining_str_len -= str_len;
    parameter_data ++;

    busy_bar += busy_inc;
    while( q15_GetMantissa(busy_bar) )
    {
      Terminal_printf(terminal, "=");
      busy_bar -= Q15_FACTOR;
    }
  }
  Terminal_printf(terminal, "|" LINE_BREAK);
  Terminal_Data.table_crc = table_crc;
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
