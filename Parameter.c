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
 * @file Parameter.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */


#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include <limits.h>
#include <string.h>
#include "Error.h"
#include "Unit.h"
#include "Crc.h"
#ifdef ESTL_ENABLE_STORAGE
  #include "Storage.h"
#endif
#include "Parameter.h"
#include "Parameter_TableIndices.h"
#include "Parameter_Table.h"
#ifdef ESTL_ENABLE_DEBUG
  #include "Debug.h"
#endif
#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
  #include "Scope.h"
#endif
#ifdef ESTL_ENABLE_RF
  #include "RfApp.h"
#endif


/**
 * Define bit-length for parameter major respectively minor revision
 */
#define PARAMETER_REVISION_COUNTER_BIT_LENGTH   (16)


// Check major parameter revision
#ifndef PARAMETER_REVISION_MAJOR
#error "PARAMETER_REVISION_MAJOR is not defined."
#elif( PARAMETER_REVISION_MAJOR < 0 )
#error "PARAMETER_REVISION_MAJOR must be greater or equal zero."
#elif( PARAMETER_REVISION_MAJOR >= (1 << PARAMETER_REVISION_COUNTER_BIT_LENGTH) )
#error "PARAMETER_REVISION_MAJOR is too high."
#endif

// Check minor parameter revision
#ifndef PARAMETER_REVISION_MINOR
#error "PARAMETER_REVISION_MINOR is not defined."
#elif( PARAMETER_REVISION_MINOR < 0 )
#error "PARAMETER_REVISION_MINOR must be greater or equal zero."
#elif( PARAMETER_REVISION_MINOR >= (1 << PARAMETER_REVISION_COUNTER_BIT_LENGTH) )
#error "PARAMETER_REVISION_MINOR is too high."
#endif

// Prepare parameter revision for parameter table
#define PARAMETER_REVISION_MINOR_MASK           ((1 << PARAMETER_REVISION_COUNTER_BIT_LENGTH) - 1)
#define PARAMETER_REVISION_MAJOR_MASK           (((1 << PARAMETER_REVISION_COUNTER_BIT_LENGTH) - 1) << PARAMETER_REVISION_COUNTER_BIT_LENGTH)
#define PAR_REV_NR ( ((PARAMETER_REVISION_MAJOR << PARAMETER_REVISION_COUNTER_BIT_LENGTH) && PARAMETER_REVISION_MAJOR_MASK) | \
                     (PARAMETER_REVISION_MINOR & PARAMETER_REVISION_MINOR_MASK) )


// Check firmware name
#ifndef FIRMWARE_NAME
#warning "FIRMWARE_NAME is not defined - using a default name."
#define FIRMWARE_NAME "NO_FIRMWARE_NAME"
#endif

// Check firmware revision
#ifndef FIRMWARE_REVISION
#warning "FIRMWARE_REVISION is not defined."
#define FIRMWARE_REVISION ""
#endif

// Prepare Git information string
#ifdef GIT_INFO
#define GIT_INFO_STR "\nGit: " GIT_INFO
#else
#define GIT_INFO_STR ""
#endif

// Prepare SVN information string
#ifdef SVN_INFO
#define SVN_INFO_STR "\nSVN: " SVN_INFO
#else
#define SVN_INFO_STR ""
#endif

// Prepare open source information string
#ifndef OPEN_SOURCE_INFO
#define OPEN_SOURCE_INFO "This firmware is linked to GNU LGPL licensed source code.\n" \
                         "See https://github.com/treichler/ESTL for further information."
#endif

// prepare firmware information built
#define SERVICE_HELP_STR        (FIRMWARE_NAME " " FIRMWARE_REVISION \
                                 "\nBuilt: " __DATE__ ", " __TIME__ \
                                 GIT_INFO_STR \
                                 SVN_INFO_STR \
                                 "\n" OPEN_SOURCE_INFO)

#define DEBUG_MAX (ESTL_DEBUG_NR_OF_ENTRIES)
#define DEBUG_MIN (1)


//-------------------------------------------------------------------------------------------------
//  Local prototypes
//-------------------------------------------------------------------------------------------------
error_code_t Parameter_LoadDefaultData(void);


//-------------------------------------------------------------------------------------------------
//  parameter related service functions
//-------------------------------------------------------------------------------------------------

/**
 * Set default USER_ACCESS_SECRET
 */
#ifndef USER_ACCESS_SECRET
#define USER_ACCESS_SECRET (1)
#warning "USER_ACCESS_SECRET is set to default value. Define it outside the library to increase security."
#endif

/**
 * Set default SERVICE_ACCESS_SECRET
 */
#ifndef SERVICE_ACCESS_SECRET
#define SERVICE_ACCESS_SECRET (2)
#warning "SERVICE_ACCESS_SECRET is set to default value. Define it outside the library to increase security."
#endif

/**
 * Set default PRODUCTION_ACCESS_SECRET
 */
#ifndef PRODUCTION_ACCESS_SECRET
#define PRODUCTION_ACCESS_SECRET (3)
#warning "PRODUCTION_ACCESS_SECRET is set to default value. Define it outside the library to increase security."
#endif

/**
 * Set default DEVELOPER_ACCESS_SECRET
 */
#ifndef DEVELOPER_ACCESS_SECRET
#define DEVELOPER_ACCESS_SECRET (4)
#warning "DEVELOPER_ACCESS_SECRET is set to default value. Define it outside the library to increase security."
#endif


/**
 * Parameter module's local data structure, containing static variables.
 */
struct {
  int32_t  access_level;
  uint32_t access_secret;
  uint32_t sys_table_crc;
  uint32_t app_table_crc;
} Parameter_Data = {
    .access_level   = 0,
    .access_secret  = 0,
};

const uint32_t access_secrets[] = {USER_ACCESS_SECRET, SERVICE_ACCESS_SECRET, PRODUCTION_ACCESS_SECRET, DEVELOPER_ACCESS_SECRET};

#define NR_OF_ACCESS_SECRETS    (sizeof(access_secrets)/sizeof(uint32_t))


/**
 * Check if a parameter entry is accessible due to access level.
 * @param[in]  parameter_data           Pointer to the parameter_table_entry.
 * @return                              Accessibility of parameter entry
 *   @retval   TRUE                     Parameter is accessible
 *   @retval   FALSE                    Parameter is not accessible
 */
bool_t Parameter_EntryIsAccessible(const parameter_table_entry_t * parameter_table_entry)
{
  return (Parameter_Data.access_level >= (parameter_table_entry->flags & LEVEL_MASK) >> LEVEL_SHIFT);
}


/**
 * This function is related to the system parameter "sys-key".
 * On initialization or write it checks if a valid access-key is provided
 * and changes the access-level accordingly.
 * The secret value is substituted by the current access-level to prevent
 * read-out by parameter functions.
 *
 * @param     parameter_function          How is this function called.
 * @param     value                       Pointer to related value in parameter array
 * @return                                Always returns OK
 */
error_code_t Parameter_SysKeyFunction(parameter_function_t parameter_function, int32_t * value)
{
  if( (parameter_function == PARAMETER_INIT) || (parameter_function == PARAMETER_WRITE) )
  {
    uint8_t i;
    Parameter_Data.access_level = 0;
    // check if key-value fits to any access level
    for( i = 0; i < NR_OF_ACCESS_SECRETS; i ++ )
    {
      if( access_secrets[i] == (uint32_t)(*value) )
      {
        Parameter_Data.access_secret = (uint32_t)(*value);
        Parameter_Data.access_level = i + 1;
      }
    }
  }

  // Only when save is called, the value contains the secret
  // to be stored in non-volatile memory.
  // Otherwise value contains the current access level.
  if( parameter_function == PARAMETER_SAVE )
  {
    if( 0 == Parameter_Data.access_level )
      *value = 0;
    else
      *value = (int32_t)Parameter_Data.access_secret;
  }
  else
    *value = (int32_t)Parameter_Data.access_level;

  return OK;
}


/**
 * This function is related to the system parameter "sys-info".
 * It checks during parameter initialization if the parameter revision has changed
 *
 * @param     parameter_function          How is this function called.
 * @param     value                       Pointer to related value in parameter array
 * @return                                Error code.
 *   @retval  OK                          On success.
 *   @retval  PARAMETER_REV_MINOR_CHANGE  Parameter table's minor revision has changed.
 *   @retval  PARAMETER_REV_MAJOR_CHANGE  Parameter table's major revision has changed.
 */
error_code_t Parameter_SysInfoFunction(parameter_function_t parameter_function, int32_t * value)
{
  if( parameter_function == PARAMETER_INIT )
  {
    int32_t nvmem_parameter_revision = *value;
    *value = PAR_REV_NR;
    if( (PARAMETER_REVISION_MAJOR_MASK & nvmem_parameter_revision) != (PARAMETER_REVISION_MAJOR_MASK & PAR_REV_NR) )
      return PARAMETER_REV_MAJOR_CHANGE;
    if( (PARAMETER_REVISION_MINOR_MASK & nvmem_parameter_revision) != (PARAMETER_REVISION_MINOR_MASK & PAR_REV_NR) )
      return PARAMETER_REV_MINOR_CHANGE;
  }
  return OK;
}


/**
 * Help text for Parameter_SysCmdFunction()
 */
#define SYSTEM_CMD_HELP_STR     ("System commands:\n"        \
                                 "1: Save parameter\n"       \
                                 "2: Initialize parameter\n" \
                                 "3: Load default parameter")

/**
 * This function is related to the system parameter "sys-cmd".
 * Depending on the value different system related functions can be called:
 *   - 1: Save current parameter set to non-volatile memory
 *   - 2: Initialize parameter, respectively load from non-volatile memory
 *   - 3: Load default/nominal parameter values. Requires at least production access level
 *
 * @param     parameter_function     How is this function called.
 * @param     value                  Pointer to related value in parameter array
 * @return                           Error code depending on internal function calls.
 *   @retval  OK                     On success.
 */
error_code_t Parameter_SysCmdFunction(parameter_function_t parameter_function, int32_t * value)
{
  error_code_t error_code = OK;
  if( parameter_function == PARAMETER_INIT )
  {
  }
  else if( parameter_function == PARAMETER_WRITE )
  {
    switch( *value )
    {
      case 1:
        error_code = Parameter_Save();
        break;
      case 2:
        error_code = Parameter_Init();
        break;
      case 3:
        if( Parameter_Data.access_level >= ((LEVEL_3 & LEVEL_MASK) >> LEVEL_SHIFT) )
          error_code = Parameter_LoadDefaultData();
        else
          error_code = PARAMETER_ACCESS_DENIED;
        break;
      default:
        break;
    }
    *value = (int32_t)error_code;
  }
  else if( parameter_function == PARAMETER_READ )
  {
  }
  return error_code;
}


//-------------------------------------------------------------------------------------------------
//  Parameter table
//-------------------------------------------------------------------------------------------------

/**
 * System Parameter Table
 */
const parameter_table_entry_t System_Parameter_table[] = {
  //                           name         unit            representation       control flags         minimum     nominal         maximum                 function pointer   information/help
  [ESTL_PARAM_SYS_INFO]    = {"sys-info",   UNIT_NONE,            REPR_HEX,  LEVEL_0|R_O|NVMEM,      INT32_MIN, PAR_REV_NR,      INT32_MAX,      &Parameter_SysInfoFunction,  SERVICE_HELP_STR},
  [ESTL_PARAM_SYS_KEY]     = {"sys-key",    UNIT_NONE,            REPR_DEC,  LEVEL_0|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,       &Parameter_SysKeyFunction,  "Parameter access key. The current value represents the access level."},
  [ESTL_PARAM_SYS_CMD]     = {"sys-cmd",    UNIT_NONE,            REPR_DEC,  LEVEL_0|R_W,            INT32_MIN,          0,      INT32_MAX,       &Parameter_SysCmdFunction,  SYSTEM_CMD_HELP_STR},
#ifdef ESTL_ENABLE_RF
  // radio frequency
  [ESTL_PARAM_RF_FREQ]     = {"RF-freq",    UNIT_MEG_HERTZ,     REPR_Q15_4,  LEVEL_1|R_W|NVMEM,     Q15(863.0), Q15(868.0),     Q15(870.0),    &RfApp_FreqParameterFunction,  "Transmitter frequency."},
  [ESTL_PARAM_RF_NODEID]   = {"RF-nodeID",  UNIT_NONE,            REPR_DEC,  LEVEL_1|R_W|NVMEM,              0,          0,            254,  &RfApp_NodeIdParameterFunction,  "Node ID"},
  [ESTL_PARAM_RF_NETID]    = {"RF-netID",   UNIT_NONE,            REPR_DEC,  LEVEL_1|R_W|NVMEM,              0,          0,            255,   &RfApp_NetIdParameterFunction,  "Network ID"},
  [ESTL_PARAM_RF_AESKEY_1] = {"RF-AES1",    UNIT_NONE,            REPR_HEX,  LEVEL_1|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,     &RfApp_AesParameterFunction,  "AES key for RF-link. Encryption is disabled if all AES-keys are 0."},
  [ESTL_PARAM_RF_AESKEY_2] = {"RF-AES2",    UNIT_NONE,            REPR_HEX,  LEVEL_1|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,     &RfApp_AesParameterFunction,  "AES key for RF-link. Encryption is disabled if all AES-keys are 0."},
  [ESTL_PARAM_RF_AESKEY_3] = {"RF-AES3",    UNIT_NONE,            REPR_HEX,  LEVEL_1|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,     &RfApp_AesParameterFunction,  "AES key for RF-link. Encryption is disabled if all AES-keys are 0."},
  [ESTL_PARAM_RF_AESKEY_4] = {"RF-AES4",    UNIT_NONE,            REPR_HEX,  LEVEL_1|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,     &RfApp_AesParameterFunction,  "AES key for RF-link. Encryption is disabled if all AES-keys are 0."},
#endif
#ifdef ESTL_ENABLE_DEBUG
  // debug
  [ESTL_PARAM_D_INDEX]     = {"d-index",    UNIT_NONE,            REPR_DEC,  LEVEL_3|R_W|HIDE|INFO,  DEBUG_MIN,  DEBUG_MIN,      DEBUG_MAX,   &Debug_IndexParameterFunction,  "The selected channel of the debug module"},
  [ESTL_PARAM_D_ADDR]      = {"d-addr",     UNIT_NONE,            REPR_HEX,  LEVEL_3|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,    &Debug_AddrParameterFunction,  "The physical address that should be accessed.\nIf mask is 0, then this is the index of the debug lookup-table."},
  [ESTL_PARAM_D_MASK]      = {"d-mask",     UNIT_NONE,            REPR_HEX,  LEVEL_3|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,    &Debug_MaskParameterFunction,  "This masks the variable's access."},
  [ESTL_PARAM_D_TYPE]      = {"d-type",     UNIT_NONE,            REPR_DEC,  LEVEL_3|R_W|HIDE,               0,          0,  NR_OF_REPRS-1,    &Debug_ReprParameterFunction,  "Representation of the variable."},
  [ESTL_PARAM_D_DATA]      = {"d-data",     UNIT_NONE,            REPR_DEC,  LEVEL_3|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,    &Debug_DataParameterFunction,  "Access the variable.\nIf mask is 0, then the content of the debug lookup-table will be read."},
#endif
#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
  [ESTL_PARAM_S_CMD]       = {"s-cmd",      UNIT_NONE,            REPR_DEC,  LEVEL_3|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,     &Scope_CmdParameterFunction,  "Scope command:\n0: stop\n1: start/armed\n2: ready\n3: triggered\n4: complete"},
  [ESTL_PARAM_S_DIV]       = {"s-div",      UNIT_NONE,            REPR_DEC,  LEVEL_3|R_W|HIDE|INFO,          1,          1,     UINT16_MAX,   &Scope_SetupParameterFunction,  "Sample divider - save every nth sample."},
  [ESTL_PARAM_S_PRE]       = {"s-pre",      UNIT_PERCENT,         REPR_DEC,  LEVEL_3|R_W|HIDE|INFO,          0,          0,            100,   &Scope_SetupParameterFunction,  "Pre-trigger buffer size."},
  [ESTL_PARAM_S_TRIGC]     = {"s-trigc",    UNIT_NONE,            REPR_DEC,  LEVEL_3|R_W|HIDE|INFO, -DEBUG_MAX,          0,      DEBUG_MAX,   &Scope_SetupParameterFunction,  "Trigger channel, where the sign represents the trigger-edge."},
  [ESTL_PARAM_S_TRIGL]     = {"s-trigl",    UNIT_NONE,            REPR_DEC,  LEVEL_3|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,   &Scope_SetupParameterFunction,  "Trigger level."},
#endif
};

/**
 * Calculate the number of entries in the previously created system-parameter table
 */
#define SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES   (sizeof(System_Parameter_table) / sizeof(parameter_table_entry_t))


//-------------------------------------------------------------------------------------------------
//  Parameter array
//-------------------------------------------------------------------------------------------------

/**
 * Parameter array holds the dynamic value of every parameter entry
 */
int32_t Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + NR_OF_PARAMETER_TABLE_ENTRIES];


//-------------------------------------------------------------------------------------------------
//  parameter access functions
//-------------------------------------------------------------------------------------------------

inline range_t Parameter_GetIndexRange(void)
{
  const range_t parameter_index_range = {
      .min = (int16_t)(-SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES),
      .max = (int16_t)(NR_OF_PARAMETER_TABLE_ENTRIES - 1),
  };
  return parameter_index_range;
}


bool_t Parameter_IndexExists(int16_t parameter_index)
{
  range_t index_range = Parameter_GetIndexRange();
  return ValueInRange( parameter_index, index_range );
}


/**
 * Get the parameter table's entry according to index.
 * This function automatically recognizes if system- or application-table is addressed.
 *
 * @param     parameter_table_entry    Pointer to parameter table entry's address.
 * @param     parameter_index          Index to parameter entry.
 * @return                             Error code depending on accessibility of the entry.
 *   @retval  OK                       On success.
 *   @retval  INDEX_OUT_OF_BOUNDARY    Accessed parameter does not exist.
 *   @retval  PARAMETER_ENUM_MISMATCH  Something is wrong in application's parameter table.
 */
error_code_t Parameter_GetEntry(const parameter_table_entry_t ** parameter_table_entry, int16_t parameter_index)
{
  if( parameter_index >= 0 )
    return ParameterTable_GetEntry(parameter_table_entry, parameter_index);

  parameter_index = -parameter_index - 1;
  if( parameter_index >= SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES )
    return INDEX_OUT_OF_BOUNDARY;
  *parameter_table_entry = &System_Parameter_table[parameter_index];
  return OK;
}


int16_t Parameter_FindIndexByName(char * parameter_name)
{
  const parameter_table_entry_t * parameter_table_entry = 0;
  int16_t i = 0;
  for (i = -(int16_t)SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES; i < NR_OF_PARAMETER_TABLE_ENTRIES; i ++)
  {
    Parameter_GetEntry(&parameter_table_entry, i);
    if ( 0 == strcmp(parameter_table_entry->name, parameter_name) )
      break;
  }
  return i;
}


bool_t Parameter_IsWritable(int16_t parameter_index)
{
  const parameter_table_entry_t * parameter_table_entry = 0;
  error_code_t error_code = Parameter_GetEntry(&parameter_table_entry, parameter_index);
  if( (OK == error_code) && Parameter_EntryIsAccessible(parameter_table_entry) && (parameter_table_entry->flags & R_W) )
    return TRUE;

  return FALSE;
}


error_code_t Parameter_WriteValue(int16_t parameter_index, int32_t value)
{
  const parameter_table_entry_t * parameter_table_entry = 0;
  int32_t old_value;
  error_code_t error;
  error = Parameter_GetEntry(&parameter_table_entry, parameter_index);
  if( OK != error )
    return error;

  if( ! Parameter_EntryIsAccessible(parameter_table_entry) )
    return PARAMETER_ACCESS_DENIED;
  if ( ! (parameter_table_entry->flags & R_W) )
    return PARAMETER_WRITE_PROTECTED;
  if ( ! (value >= parameter_table_entry->minimum) )
    return BELOW_LIMIT;
  if ( ! (value <= parameter_table_entry->maximum) )
    return ABOVE_LIMIT;

  old_value = Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index];
  Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index] = value;
  if( parameter_table_entry->parameterFunction )
  {
    error = parameter_table_entry->parameterFunction( PARAMETER_WRITE, &(Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index]) );
    if( OK != error )
      Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index] = old_value;
  }
  return error;
}


error_code_t Parameter_ReadValue(int16_t parameter_index, int32_t * value)
{
  const parameter_table_entry_t * parameter_table_entry = 0;
  error_code_t error;
  error = Parameter_GetEntry(&parameter_table_entry, parameter_index);
  if( OK != error )
    return error;

  if( ! Parameter_EntryIsAccessible(parameter_table_entry) )
    return PARAMETER_ACCESS_DENIED;

  if (parameter_table_entry->parameterFunction != 0)
    error = parameter_table_entry->parameterFunction(PARAMETER_READ, &Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index]);
  *value = Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index];
  return error;
}


int32_t Parameter_GetValue(int16_t parameter_index)
{
  const parameter_table_entry_t * parameter_table_entry = 0;
  error_code_t error;
  error = Parameter_GetEntry(&parameter_table_entry, parameter_index);
  if( OK != error )
    return 0;

  if (parameter_table_entry->parameterFunction != 0)
  {
    error = parameter_table_entry->parameterFunction( PARAMETER_READ, &(Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index]) );
    if( OK != error )
      return 0;
  }
  return Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index];
}


const char * Parameter_GetHelp(int16_t parameter_index)
{
  const parameter_table_entry_t * parameter_table_entry = 0;
  error_code_t error;
  error = Parameter_GetEntry(&parameter_table_entry, parameter_index);
  if( OK != error )
    return "";
  return parameter_table_entry->help;
}


error_code_t Parameter_ReadData(int16_t parameter_index, parameter_data_t * parameter_data)
{
  const parameter_table_entry_t * parameter_table_entry = 0;
  error_code_t error;
  error = Parameter_GetEntry(&parameter_table_entry, parameter_index);
  if( OK != error )
    return error;

  parameter_data->name    = parameter_table_entry->name;
  parameter_data->unit    = parameter_table_entry->unit;
#ifdef ESTL_ENABLE_DEBUG
  // override data type in case of Debug data
  if( parameter_index == (-ESTL_PARAM_D_DATA - 1) )
  {
    int32_t repr;
    Debug_ReprParameterFunction( PARAMETER_READ, &repr);
    parameter_data->repr = (repr_t)repr;
  }
  else
    parameter_data->repr    = parameter_table_entry->repr;
#else
  parameter_data->repr    = parameter_table_entry->repr;
#endif
  parameter_data->flags   = parameter_table_entry->flags;
  parameter_data->nominal = parameter_table_entry->nominal;
  parameter_data->minimum = parameter_table_entry->minimum;
  parameter_data->maximum = parameter_table_entry->maximum;
//  parameter_data->help    = parameter_table_entry->help;
//  if (parameter_table_entry->parameterFunction != 0)
//    error = parameter_table_entry->parameterFunction( PARAMETER_READ, &(Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index]) );
//  parameter_data->value   = Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + parameter_index];
  if( (! Parameter_EntryIsAccessible(parameter_table_entry)) && (parameter_table_entry->flags & HIDE) )
    return PARAMETER_HIDDEN;
  return error;
}


//-------------------------------------------------------------------------------------------------
//  Parameter image
//-------------------------------------------------------------------------------------------------

#ifdef ESTL_ENABLE_STORAGE
/**
 * Define parameter entry for non-volatile storage
 */
typedef struct {
  int32_t  value;       //!<  Parameter's non-volatile value
  int16_t  index;       //!<  Value's related parameter table index
  uint16_t crc;         //!<  Integrity of non-volatile entry relative to parameter table entry
} nv_parameter_entry_t;
#endif


/**
 * Calculate checksum of parameter table entry.
 * The CRC is built over name, unit, representation, and help respectively
 * minimal, maximal and nominal value.
 * This checksum could be used to identify the parameter table's entries
 * by an external application.
 *
 * @param[in]  parameter_table_entry  Pointer to parameter table entry structure
 * @param[in]  previous_crc           Previously calculated CRC
 * @return                            The calculated CRC
 */
uint32_t Parameter_TableEntryCrc(const parameter_table_entry_t * parameter_table_entry, uint32_t previous_crc)
{
  uint32_t crc;
  crc = Crc_Crc32( (const uint8_t*)parameter_table_entry->name,     strlen(parameter_table_entry->name),    previous_crc );
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->unit,    sizeof(parameter_table_entry->unit),    crc );
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->repr,    sizeof(parameter_table_entry->repr),    crc );
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->minimum, sizeof(parameter_table_entry->minimum), crc );
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->nominal, sizeof(parameter_table_entry->nominal), crc );
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->maximum, sizeof(parameter_table_entry->maximum), crc );
  crc = Crc_Crc32( (const uint8_t*)parameter_table_entry->help,     strlen(parameter_table_entry->help),    crc );
  return crc;
}


/**
 * Calculate checksum of parameter table entry for non-volatile entry.
 * The CRC is built over name, unit and representation, so it holds all
 * relevant information for an integrity check between non-volatile
 * entry and related parameter table entry
 *
 * @param[in]  parameter_table_entry  Pointer to parameter table entry structure
 * @param[in]  previous_crc           Previously calculated CRC
 * @return                            The calculated CRC
 */
uint16_t Parameter_NvEntryCrc(const parameter_table_entry_t * parameter_table_entry)
{
  uint16_t crc;
  crc = Crc_Crc16( (const uint8_t*)parameter_table_entry->name,  strlen(parameter_table_entry->name), 0x0000 );
  crc = Crc_Crc16( (const uint8_t*)&parameter_table_entry->unit, sizeof(parameter_table_entry->unit), crc );
  crc = Crc_Crc16( (const uint8_t*)&parameter_table_entry->repr, sizeof(parameter_table_entry->repr), crc );
  return crc;
}


/**
 * Load the parameter image from non-volatile memory.
 * Only on return value OK or PARAMETER_REV_MINOR_CHANGE the parameters are successfully loaded.
 *
 * @return                                Error code depending on consistency
 *                                        and compatibility of non-volatile data.
 *   @retval  OK                          On success.
 *   @retval  PARAMETER_REV_MINOR_CHANGE  Minor change in parameter revision.
 *   @retval  PARAMETER_REV_MAJOR_CHANGE  Major change in parameter revision.
 *   @retval  PARAMETER_ENUM_MISMATCH     Something is wrong in application's parameter table.
 *   @retval  STORAGE_NOT_INITIALIZED
 *   @retval  STORAGE_ENUM_MISMATCH
 *   @retval  STORAGE_INDEX_MISMATCH
 *   @retval  STORAGE_SIZE_MISMATCH
 *   @retval  STORAGE_CRC_MISMATCH
 *   @retval  INDEX_OUT_OF_BOUNDARY       Accessed storage does not exist.
 */
error_code_t Parameter_LoadNvData(void)
{
#ifdef ESTL_ENABLE_STORAGE
  nv_parameter_entry_t nv_parameter_entry[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + NR_OF_PARAMETER_TABLE_ENTRIES];
  const parameter_table_entry_t * parameter_table_entry = 0;
  int32_t value, storage_size, number_of_nv_entries;
  int16_t i;
  uint16_t nvmem_index = 0;
  error_code_t init_status = OK;
  uint8_t content_changed = 0;

  // load and check data from non volatile memory
  storage_size = Storage_Read(STORAGE_PARAMETER_IMAGE, (uint8_t*)(nv_parameter_entry), sizeof(nv_parameter_entry));
  if( storage_size < 0 )
    return (error_code_t)storage_size;
  number_of_nv_entries = storage_size / sizeof(nv_parameter_entry_t);

  // copy values to parameter array
  for( i = -(int16_t)SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES; i < NR_OF_PARAMETER_TABLE_ENTRIES; i++ )
  {
    Parameter_GetEntry(&parameter_table_entry, i);

    // calculate parameter table CRC for identification
    Parameter_Data.sys_table_crc = Parameter_Data.app_table_crc = 0xFFFFFFFF;
    if( i < 0 )
      Parameter_Data.sys_table_crc = Parameter_TableEntryCrc(parameter_table_entry, Parameter_Data.sys_table_crc);
    else
      Parameter_Data.app_table_crc = Parameter_TableEntryCrc(parameter_table_entry, Parameter_Data.app_table_crc);

    // check if parameter entry needs to be loaded with non-volatile memory data
    if( parameter_table_entry->flags & NVMEM )
    {
      // increase nvmem_index just in case if there is non-volatile data from another minor parameter revision
      while( (i > nv_parameter_entry[nvmem_index].index) && (nvmem_index < number_of_nv_entries) )
      {
        nvmem_index ++;
        content_changed = 1;
      }

      if( (nv_parameter_entry[nvmem_index].index == i) && (nvmem_index < number_of_nv_entries) &&
          (nv_parameter_entry[nvmem_index].crc == Parameter_NvEntryCrc(parameter_table_entry)) )
      {
        // non-volatile entry found, load its value
        value = nv_parameter_entry[nvmem_index].value;
        // TODO eventually check value's limits, correct it or abort with error?
        nvmem_index ++;
      }
      else
      {
        // non-volatile entry not found, load nominal value
        value = parameter_table_entry->nominal;
        content_changed = 1;
      }
    }
    else
    {
      // load nominal value
      value = parameter_table_entry->nominal;
      if( parameter_table_entry->flags & NVMEM )
        content_changed = 1;
    }
    // copy value to parameter array
    Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + i] = value;

    // check if parameter function needs to be called
    if (parameter_table_entry->parameterFunction != 0)
    {
      error_code_t error;
      error = parameter_table_entry->parameterFunction(PARAMETER_INIT, &Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + i]);

      // abort if parameter function call fails
      if( error == PARAMETER_REV_MAJOR_CHANGE )
        return error;
      if( error == PARAMETER_REV_MINOR_CHANGE )
        init_status = error;
/*
      // TODO in case of IMAGE_REV_MAJOR_CHANGE call save() to store default values to nv-memory
      if (init_status == PARAMETER_REV_MAJOR_CHANGE)
      {
        // TODO save, or not...
        Parameter_data.init_status = init_status;
        return init_status;
      }
      if (init_status == PARAMETER_REV_MINOR_CHANGE)
        Parameter_data.init_status = init_status;
*/
    }
  }

//  Parameter_data.init_status = init_status;
//  return Parameter_data.init_status;
  if( content_changed && (OK == init_status) )
    return PARAMETER_CONTENT_CHANGE;
  return init_status;
#else
  return PARAMETER_NOT_INITIALIZED;
#endif
}


/**
 * Load default parameter values.
 *
 * @return                             Error code.
 *   @retval  OK                       On success.
 *   @retval  PARAMETER_ENUM_MISMATCH  Something is wrong in application's parameter table.
 */
error_code_t Parameter_LoadDefaultData(void)
{
  int16_t i;
  error_code_t error = OK;
  const parameter_table_entry_t * parameter_table_entry = 0;
  for (i = -(int16_t)SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES; i < NR_OF_PARAMETER_TABLE_ENTRIES; i++)
  {
    error = Parameter_GetEntry(&parameter_table_entry, i);
    if( OK != error )
      return error;
    Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + i] = parameter_table_entry->nominal;
    if (parameter_table_entry->parameterFunction != 0)
      parameter_table_entry->parameterFunction( PARAMETER_INIT, &(Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + i]) );
  }
  return error;
}


error_code_t Parameter_Init(void)
{
  error_code_t error;
  // try to initialize parameters with non-volatile memory data
  error = Parameter_LoadNvData();
  if( (OK == error) || (PARAMETER_CONTENT_CHANGE == error) || (PARAMETER_REV_MINOR_CHANGE == error) )
    return error;

  // loading non-volatile memory failed so initialize parameters with default values
  Parameter_LoadDefaultData();
  return error;
}


error_code_t Parameter_Save(void)
{
#ifdef ESTL_ENABLE_STORAGE
  nv_parameter_entry_t nv_parameter_entry[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + NR_OF_PARAMETER_TABLE_ENTRIES];
  const parameter_table_entry_t * parameter_table_entry = 0;
  int16_t i;
  uint16_t nvmem_index = 0;
  error_code_t error;

  for( i = -(int16_t)SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES; i < NR_OF_PARAMETER_TABLE_ENTRIES; i++ )
  {
    error = Parameter_GetEntry(&parameter_table_entry, i);
    if( OK != error )
      return error;
    // copy data from parameter table to nv-memory structure
    if( parameter_table_entry->flags & NVMEM )
    {
      if( parameter_table_entry->parameterFunction )
        parameter_table_entry->parameterFunction( PARAMETER_SAVE, &(Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + i]) );
      nv_parameter_entry[nvmem_index].value = Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + i];
      nv_parameter_entry[nvmem_index].index = i;
      nv_parameter_entry[nvmem_index].crc   = Parameter_NvEntryCrc( parameter_table_entry );
      nvmem_index ++;
    }
  }
  return Storage_Write( STORAGE_PARAMETER_IMAGE, (uint8_t*)nv_parameter_entry, nvmem_index * sizeof(nv_parameter_entry_t) );
#else
  return OK;
#endif
}