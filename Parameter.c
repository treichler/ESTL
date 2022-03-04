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

// scope's help text
#define SCOPE_HELP_STR          ("Scope command:\n" \
                                 "0 rw: stop\n" \
                                 "1 rw: start/armed\n" \
                                 "2 ro: ready\n" \
                                 "3 ro: triggered\n" \
                                 "4 ro: complete\n" \
                                 "5 rw: read buffer\n" \
                                 "6 rw: DAQ mode")


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
  error_code_t  init_error;     //!<  Parameter initialization error code
  int8_t        access_level;   //!<  Currently activated parameter access level
  uint32_t      access_secret;  //!<  Current access level's secret key
  uint32_t      table_crc;      //!<  Checksum over the whole parameter table
} Parameter_Data;

/**
 * Array containing parameter table access secret keys
 */
const uint32_t access_secrets[] = {USER_ACCESS_SECRET, SERVICE_ACCESS_SECRET, PRODUCTION_ACCESS_SECRET, DEVELOPER_ACCESS_SECRET};

#define NR_OF_ACCESS_SECRETS    (sizeof(access_secrets)/sizeof(uint32_t))


/**
 * Check if a parameter entry is accessible due to access level.
 * @param[in]  parameter_table_entry    Pointer to the parameter_table_entry.
 * @return                              Accessibility of parameter entry
 *   @retval   TRUE                     Parameter is accessible
 *   @retval   FALSE                    Parameter is not accessible
 */
bool_t Parameter_EntryIsAccessible(const parameter_table_entry_t * parameter_table_entry)
{
  return( Parameter_Data.access_level >= (parameter_table_entry->flags & LEVEL_MASK) );
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
#define SYSTEM_CMD_HELP_STR     ("Value shows parameter table CRC\n" \
                                 "System commands:\n"                \
                                 "1: Save parameter\n"               \
                                 "2: Initialize parameter\n"         \
                                 "3: Load default parameter\n"       \
                                 "4: Parameter init status")

/**
 * This function is related to the system parameter "sys-cmd".
 * Depending on the value different system related functions can be called:
 *   - 1: Save current parameter set to non-volatile memory
 *   - 2: Initialize parameter, respectively load from non-volatile memory
 *   - 3: Load default/nominal parameter values. Requires at least production access level
 *   - 4: Return error code of parameter initialization status
 * On PARAMETER_READ the parameter table's checksum is returned via value.
 *
 * @param     parameter_function     How is this function called.
 * @param     value                  Pointer to related value in parameter array
 * @return                           Error code depending on internal function calls.
 *   @retval  OK                     On success.
 */
error_code_t Parameter_SysCmdFunction(parameter_function_t parameter_function, int32_t * value)
{
  error_code_t error_code = OK;
  if( parameter_function == PARAMETER_WRITE )
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
        if( Parameter_Data.access_level >= (LEVEL_3 & LEVEL_MASK) )
          error_code = Parameter_LoadDefaultData();
        else
          error_code = PARAMETER_ACCESS_DENIED;
        break;
      case 4:
        error_code = Parameter_Data.init_error;
        break;
      default:
        break;
    }
  }
  else if( parameter_function == PARAMETER_READ )
  {
    *value = Parameter_Data.table_crc;
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
  [ESTL_PARAM_SYS_KEY]     = {"sys-key",    UNIT_NONE,            REPR_DEC,  LEVEL_0|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,       &Parameter_SysKeyFunction,  HELP_TEXT("Parameter access key. The current value represents the access level.")},
  [ESTL_PARAM_SYS_CMD]     = {"sys-cmd",    UNIT_NONE,            REPR_HEX,  LEVEL_0|R_W,            INT32_MIN,          0,      INT32_MAX,       &Parameter_SysCmdFunction,  HELP_TEXT(SYSTEM_CMD_HELP_STR)},
#ifdef ESTL_ENABLE_RF
  // radio frequency
  [ESTL_PARAM_RF_FREQ]     = {"RF-freq",    UNIT_MEG_HERTZ,     REPR_Q15_4,  LEVEL_1|R_W|NVMEM,     Q15(863.0), Q15(868.0),     Q15(870.0),    &RfApp_FreqParameterFunction,  HELP_TEXT("Transmitter frequency.")},
  [ESTL_PARAM_RF_NODEID]   = {"RF-nodeID",  UNIT_NONE,            REPR_DEC,  LEVEL_1|R_W|NVMEM,              0,          0,            254,  &RfApp_NodeIdParameterFunction,  HELP_TEXT("Node ID")},
  [ESTL_PARAM_RF_NETID]    = {"RF-netID",   UNIT_NONE,            REPR_DEC,  LEVEL_1|R_W|NVMEM,              0,          0,            255,   &RfApp_NetIdParameterFunction,  HELP_TEXT("Network ID")},
  [ESTL_PARAM_RF_AESKEY_1] = {"RF-AES1",    UNIT_NONE,            REPR_HEX,  LEVEL_1|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,     &RfApp_AesParameterFunction,  HELP_TEXT("AES key for RF-link. Encryption is disabled if all AES-keys are 0.")},
  [ESTL_PARAM_RF_AESKEY_2] = {"RF-AES2",    UNIT_NONE,            REPR_HEX,  LEVEL_1|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,     &RfApp_AesParameterFunction,  HELP_TEXT("AES key for RF-link. Encryption is disabled if all AES-keys are 0.")},
  [ESTL_PARAM_RF_AESKEY_3] = {"RF-AES3",    UNIT_NONE,            REPR_HEX,  LEVEL_1|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,     &RfApp_AesParameterFunction,  HELP_TEXT("AES key for RF-link. Encryption is disabled if all AES-keys are 0.")},
  [ESTL_PARAM_RF_AESKEY_4] = {"RF-AES4",    UNIT_NONE,            REPR_HEX,  LEVEL_1|R_W|NVMEM,      INT32_MIN,          0,      INT32_MAX,     &RfApp_AesParameterFunction,  HELP_TEXT("AES key for RF-link. Encryption is disabled if all AES-keys are 0.")},
#endif
#ifdef ESTL_ENABLE_DEBUG
  // debug
  [ESTL_PARAM_D_INDEX]     = {"d-index",    UNIT_NONE,            REPR_DEC,  LEVEL_2|R_W|HIDE|INFO,  DEBUG_MIN,  DEBUG_MIN,      DEBUG_MAX,   &Debug_IndexParameterFunction,  HELP_TEXT("The selected channel of the debug module")},
  [ESTL_PARAM_D_ADDR]      = {"d-addr",     UNIT_NONE,            REPR_HEX,  LEVEL_2|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,    &Debug_AddrParameterFunction,  HELP_TEXT("The physical address that should be accessed.\nIf mask is 0, then this is the index of the debug lookup-table.")},
  [ESTL_PARAM_D_MASK]      = {"d-mask",     UNIT_NONE,            REPR_HEX,  LEVEL_2|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,    &Debug_MaskParameterFunction,  HELP_TEXT("This masks the variable's access.")},
  [ESTL_PARAM_D_DATA]      = {"d-data",     UNIT_NONE,            REPR_HEX,  LEVEL_2|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,    &Debug_DataParameterFunction,  HELP_TEXT("Access the variable.\nIf mask is 0, then the content of the debug lookup-table will be read.")},
#endif
#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
  [ESTL_PARAM_S_CMD]       = {"s-cmd",      UNIT_NONE,            REPR_DEC,  LEVEL_2|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,     &Scope_CmdParameterFunction,  HELP_TEXT(SCOPE_HELP_STR)},
  [ESTL_PARAM_S_DIV]       = {"s-div",      UNIT_NONE,            REPR_DEC,  LEVEL_2|R_W|HIDE|INFO,          1,          1,     UINT16_MAX,   &Scope_SetupParameterFunction,  HELP_TEXT("Sample divider - save every nth sample.")},
  [ESTL_PARAM_S_PRE]       = {"s-pre",      UNIT_PERCENT,         REPR_DEC,  LEVEL_2|R_W|HIDE|INFO,          0,          0,            100,   &Scope_SetupParameterFunction,  HELP_TEXT("Pre-trigger buffer size.")},
  [ESTL_PARAM_S_TRIGC]     = {"s-trigc",    UNIT_NONE,            REPR_DEC,  LEVEL_2|R_W|HIDE|INFO, -DEBUG_MAX,          0,      DEBUG_MAX,   &Scope_SetupParameterFunction,  HELP_TEXT("Trigger channel, where the sign represents the trigger-edge.")},
  [ESTL_PARAM_S_TRIGL]     = {"s-trigl",    UNIT_NONE,            REPR_DEC,  LEVEL_2|R_W|HIDE,       INT32_MIN,          0,      INT32_MAX,   &Scope_SetupParameterFunction,  HELP_TEXT("Trigger level.")},
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

inline bool_t Parameter_CurrentAccessLevelIsDeveloper(void)
{
  return (LEVEL_4 & LEVEL_MASK) == Parameter_Data.access_level;
}


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


inline uint32_t Parameter_GetTableCrc(void)
{
  return Parameter_Data.table_crc;
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
  parameter_data->repr    = parameter_table_entry->repr;
  parameter_data->flags   = parameter_table_entry->flags;
  parameter_data->nominal = parameter_table_entry->nominal;
  parameter_data->minimum = parameter_table_entry->minimum;
  parameter_data->maximum = parameter_table_entry->maximum;
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
 * The CRC is built over name, unit, representation, and flags respectively
 * minimal, maximal and nominal value. The help text is kept out since a
 * change on help should not have an impact on a parameter table entry's
 * property.
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
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->flags,   sizeof(parameter_table_entry->flags),   crc );
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->minimum, sizeof(parameter_table_entry->minimum), crc );
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->nominal, sizeof(parameter_table_entry->nominal), crc );
  crc = Crc_Crc32( (const uint8_t*)&parameter_table_entry->maximum, sizeof(parameter_table_entry->maximum), crc );
  return crc;
}


/**
 * Calculate checksum of parameter table entry for non-volatile entry.
 * The CRC is built over name, unit and representation, so it holds all
 * relevant information for an integrity check between non-volatile
 * entry and related parameter table entry
 *
 * @param[in]  parameter_table_entry  Pointer to parameter table entry structure
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
 *   @retval  PARAMETER_CONTENT_CHANGE
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
  bool_t is_content_changed = FALSE;

  // load and check data from non volatile memory
  storage_size = Storage_Read(STORAGE_PARAMETER_IMAGE, (uint8_t*)(nv_parameter_entry), sizeof(nv_parameter_entry));
  if( storage_size < 0 )
    return (error_code_t)storage_size;
  number_of_nv_entries = storage_size / sizeof(nv_parameter_entry_t);

  // copy values to parameter array
  for( i = -(int16_t)SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES; i < NR_OF_PARAMETER_TABLE_ENTRIES; i++ )
  {
    Parameter_GetEntry(&parameter_table_entry, i);

    // check if parameter entry needs to be loaded with non-volatile memory data
    if( parameter_table_entry->flags & NVMEM )
    {
      // increase nvmem_index just in case if there is non-volatile data from another minor parameter revision
      while( (i > nv_parameter_entry[nvmem_index].index) && (nvmem_index < number_of_nv_entries) )
      {
        nvmem_index ++;
        is_content_changed = TRUE;
      }

      if( (nv_parameter_entry[nvmem_index].index == i) && (nvmem_index < number_of_nv_entries) )
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
        is_content_changed = TRUE;
      }
    }
    else
    {
      // load nominal value
      value = parameter_table_entry->nominal;
    }
    // copy value to parameter array
    Parameter_array[SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES + i] = value;

    // check if parameter function is available and call it
    if( parameter_table_entry->parameterFunction != 0 )
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
  if( is_content_changed && (OK == init_status) )
    return PARAMETER_CONTENT_CHANGE;
  return init_status;
#else
  return PARAMETER_STORAGE_MISSING;
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
  int16_t i;
  const parameter_table_entry_t * parameter_table_entry = 0;

  // calculate parameter table CRC for identification
  Parameter_Data.table_crc = 0xFFFFFFFF;
  for( i = -(int16_t)SYSTEM_PARAMETER_TABLE_NR_OF_ENTRIES; i < NR_OF_PARAMETER_TABLE_ENTRIES; i++ )
  {
    Parameter_GetEntry(&parameter_table_entry, i);
    Parameter_Data.table_crc = Parameter_TableEntryCrc(parameter_table_entry, Parameter_Data.table_crc);
  }

  // try to initialize parameters with non-volatile memory data
  Parameter_Data.init_error = Parameter_LoadNvData();
  if( (OK == Parameter_Data.init_error) ||
      (PARAMETER_CONTENT_CHANGE == Parameter_Data.init_error) ||
      (PARAMETER_REV_MINOR_CHANGE == Parameter_Data.init_error) )
    return Parameter_Data.init_error;

  // loading non-volatile memory failed so initialize parameters with default values
  Parameter_LoadDefaultData();
  return Parameter_Data.init_error;
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
