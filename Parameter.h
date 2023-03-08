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
 * @file Parameter.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 *
 * @todo Parameter name changed:
 * To check if a parameter's name changed due to a different firmware revision
 * there could be a checksum of the parameter name saved in the non-volatile
 * memory next to the parameter and its index.
 * Since parameter uses 32 bits and index 16 bits it could be handy to use
 * a 16 bit or 8 bit checksum for the parameter name.
 * Possible reasons for change:
 *   - Same meaning different name --> stored value could be right
 *   - Different meaning different name --> stored value is definitely wrong
 * How could a minor change influence behavior in such a case?
 *
 * @todo Unit respectively Representation changed:
 * Possible reasons for change:
 *   - Unit changed
 *     - Different meaning --> stored value could be wrong
 *     - Different scaling --> stored value is definitely wrong
 *     - unit_t enumeration changed --> compatibility totally broken
 *   - Representation changed
 *     - Probably same scaling (DEC, HEX) --> stored value could be right
 *     - Different scaling (HEX, DEC vs. Q15) --> stored value is definitely wrong
 *     - repr_t enumeration changed --> compatibility totally broken
 * How could a minor change influence behavior in such a case?
 */

#ifndef __PARAMETER_H__
#define __PARAMETER_H__

#include "ESTL_Defines.h"
#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include "Error.h"
#include "Unit.h"


/**
 * @ingroup  ESTL
 * @defgroup PARAMETER  Parameter
 * @brief    Parameter module
 *
 * This module implements basic parameter functionality,
 * where the parameter-set is located in a dedicated table.
 * The main features are:
 *   - Read/Write parameter
 *   - Holds several parameter properties like
 *     - parameter name
 *     - physical unit
 *     - value's representation
 *     - minimum, maximum, and nominal value
 *     - info text
 *   - Interface to non-volatile storage
 *   - Access by parameter name
 *   - Write-access according to level
 * @{
 */


/**
 * System Parameter Table's indices
 */
enum {
  ESTL_PARAM_SN = 0,                         //!< @def ESTL_PARAM_SN system's serial number
  ESTL_PARAM_SYS_INFO,                       //!<
  ESTL_PARAM_SYS_KEY,                        //!<
  ESTL_PARAM_SYS_CMD,                        //!<
#ifdef ESTL_ENABLE_RF
  ESTL_PARAM_RF_FREQ,                        //!<
  ESTL_PARAM_RF_NODEID,                      //!<
  ESTL_PARAM_RF_NETID,                       //!<
  ESTL_PARAM_RF_AESKEY_1,                    //!<
  ESTL_PARAM_RF_AESKEY_2,                    //!<
  ESTL_PARAM_RF_AESKEY_3,                    //!<
  ESTL_PARAM_RF_AESKEY_4,                    //!<
#endif
#ifdef ESTL_ENABLE_DEBUG
  ESTL_PARAM_D_INDEX,                        //!< debug set index
  ESTL_PARAM_D_ADDR,                         //!< debug address at specified index
  ESTL_PARAM_D_MASK,                         //!< mask the debug variable
  ESTL_PARAM_D_DATA,                         //!< access the debug variable
#endif
#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
  ESTL_PARAM_S_CMD,                          //!< scope commands and read back current status
  ESTL_PARAM_S_DIV,                          //!< sample division
  ESTL_PARAM_S_PRE,                          //!< data to be recorded before trigger (% of buffer)
  ESTL_PARAM_S_TRIGC,                        //!< trigger channel where the sign represents the edge
  ESTL_PARAM_S_TRIGL,                        //!< trigger level
#endif
};

/**
 * System Parameters' access indices
 */
enum {
  PARAM_SN               = (-1 - ESTL_PARAM_SN),
  PARAM_SYS_INFO         = (-1 - ESTL_PARAM_SYS_INFO),
  PARAM_SYS_KEY          = (-1 - ESTL_PARAM_SYS_KEY),                        //!<
  PARAM_SYS_CMD          = (-1 - ESTL_PARAM_SYS_CMD),
#ifdef ESTL_ENABLE_RF
  PARAM_RF_FREQ          = (-1 - ESTL_PARAM_RF_FREQ),
  PARAM_RF_NODEID        = (-1 - ESTL_PARAM_RF_NODEID),
  PARAM_RF_NETID         = (-1 - ESTL_PARAM_RF_NETID),
  PARAM_RF_AESKEY_1      = (-1 - ESTL_PARAM_RF_AESKEY_1),
  PARAM_RF_AESKEY_2      = (-1 - ESTL_PARAM_RF_AESKEY_2),
  PARAM_RF_AESKEY_3      = (-1 - ESTL_PARAM_RF_AESKEY_3),
  PARAM_RF_AESKEY_4      = (-1 - ESTL_PARAM_RF_AESKEY_4),
#endif
#ifdef ESTL_ENABLE_DEBUG
  PARAM_D_INDEX          = (-1 - ESTL_PARAM_D_INDEX),
  PARAM_D_ADDR           = (-1 - ESTL_PARAM_D_ADDR),
  PARAM_D_MASK           = (-1 - ESTL_PARAM_D_MASK),
  PARAM_D_DATA           = (-1 - ESTL_PARAM_D_DATA),
#endif
#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )
  PARAM_S_CMD            = (-1 - ESTL_PARAM_S_CMD),
  PARAM_S_DIV            = (-1 - ESTL_PARAM_S_DIV),
  PARAM_S_PRE            = (-1 - ESTL_PARAM_S_PRE),
  PARAM_S_TRIGC          = (-1 - ESTL_PARAM_S_TRIGC),
  PARAM_S_TRIGL          = (-1 - ESTL_PARAM_S_TRIGL),
#endif
};

/**
 * @name  Parameter flags
 *
 * Access levels are additive, which means that access to a dedicated level
 * also grants access to all lower levels, e.g. one who has access to level 3
 * has also access to levels 2, 1 and 0.
 * Higher level parameter entries are by default read-only to to lower level
 * users.
 * To explicitly prohibit read-access additionally the HIDE-flag has to be set.
 * @{
 */
#define LEVEL_MASK      (0x07)  //!< Access level mask, needs to be aligned with LSB
#define LEVEL_0         (0x00)  //!< Accessible for all
#define LEVEL_1         (0x01)  //!< User access
#define LEVEL_2         (0x02)  //!< Service access
#define LEVEL_3         (0x03)  //!< Production access
#define LEVEL_4         (0x04)  //!< Developer access
#define HIDE            (0x08)  //!< Hide the parameter in case of too low access level
#define R_O             (0x00)  //!< Read only parameter access
#define R_W             (0x10)  //!< Read and write parameter access
#define NVMEM           (0x20)  //!< Store parameter in non-volatile memory
#define INFO            (0x40)  //!< Show parameter information in help context
/** @} */


/**
 * @deprecated
 * Just for backward compatibility with previous type definition
 * @{
 */
#define parameter_function_t    function_call_t
#define PARAMETER_INIT          FUNCTION_INIT
#define PARAMETER_SAVE          FUNCTION_SAVE
#define PARAMETER_READ          FUNCTION_READ
#define PARAMETER_WRITE         FUNCTION_WRITE
/** @} */


/**
 * Define the parameter table's entry structure type
 * Do not change the structure since Parameter module relies on it.
 */
typedef struct
{
  const char * const    name;           //!< A name that describes the parameter
  const unit_t          unit;           //!< The physical unit of this parameter's value
  const repr_t          repr;           //!< The representation of this parameter's value
  const uint16_t        flags;          //!< Some flags to set access and storage
  const int32_t         minimum;        //!< Parameter's minimum value
  const int32_t         nominal;        //!< Parameter's nominal value
  const int32_t         maximum;        //!< Parameter's maximum value
  error_code_t          (* const parameterFunction)(function_call_t, int32_t*);    //!< Optional parameter call function
  const char * const    help;           //!< Additional information related to this particular parameter
} parameter_table_entry_t;


/**
 * This data-structure holds the representative content of a parameter entry
 */
typedef struct
{
  const char *          name;           //!< A name that describes the parameter
  unit_t                unit;           //!< The physical unit of this parameter's value
  repr_t                repr;           //!< The representation of this parameter's value
  uint16_t              flags;          //!< Some flags to set access and storage
  int32_t               minimum;        //!< Parameter's minimum value
  int32_t               nominal;        //!< Parameter's nominal value
  int32_t               maximum;        //!< Parameter's maximum value
} parameter_data_t;


/**
 * Initialize the parameter module.
 * At first Parameter_Init() tries to load the values from non-volatile memory,
 * if this fails the default values are loaded.
 * This function has to be called prior to any other parameter function.
 *
 * @return  Error code depending on what went wrong during initialization.
 *          The return values are similar to Parameter_LoadNvData().
 */
error_code_t Parameter_Init(void);


/**
 * Set a serial-number callback function, which is called every time the
 * related system parameter "SN" is written or initialized.
 * The callback's only argument holds the serial-number which has to fit into
 * the uint32_t range.
 *
 * @param       serialNumberCallback    Serial-number callback function
 */
void Parameter_SetSerialNumberCallback( void (* serialNumberCallback)(uint32_t) );


/**
 * Check if the currently activated parameter access level is developer access.
 *
 * @return
 *   @retval    TRUE    developer access is activated
 *   @retval    FALSE   developer access is deactivated
 */
bool_t Parameter_CurrentAccessLevelIsDeveloper(void);


/**
 * Get the valid range to access parameters.
 *
 * @return  Parameters' accessible index range.
 */
range_t Parameter_GetIndexRange(void);


/**
 * Check if a parameter with desired index exists.
 *
 * @param     parameter_index  Parameter index to be checked.
 * @return
 *   @retval  TRUE    Parameter index exists.
 *   @retval  FALSE   Parameter index does not exist.
 */
bool_t Parameter_IndexExists(int16_t parameter_index);


/**
 * Get the parameter table's checksum
 *
 * @return  Checksum over the whole parameter table
 */
uint32_t Parameter_GetTableCrc(void);


/**
 * Get serial number
 *
 * @return  Serial number
 */
uint32_t Parameter_GetSerialNumber(void);


/**
 * Find a parameter's index by its name.
 * If the parameter could not be found, an index is returned which does not fit the range
 * according to Parameter_GetIndexRange().
 *
 * @param   parameter_name  A string that contains the parameter name that should be looked for
 * @return  The parameter's index.
 */
int16_t Parameter_FindIndexByName(char * parameter_name);


/**
 * Check if a parameter is writable.
 *
 * @return
 *   @retval  TRUE                   Parameter is writable.
 *   @retval  FALSE                  Parameter is not writable.
 */
bool_t Parameter_IsWritable(int16_t parameter_index);


/**
 * Write a parameter entry's value.
 * Before writing, the value is checked if it is within specified limits,
 * and if the parameter entry allows write access.
 * If the parameter entry holds a parameter function, it is called immediately
 * with the new value.
 * If the parameter function fails, the previous value is kept in parameter table.
 *
 * @param     parameter_index        The parameter entry's index.
 * @param     value                  The value to be written as new parameter.
 * @return                           Error code depending on accessibility of an entry.
 *   @retval  OK                     On success.
 */
error_code_t Parameter_WriteValue(int16_t parameter_index, int32_t value);


/**
 * Read a parameter entry's value.
 * If the parameter entry holds a parameter function, it is called immediately
 * to get the current value
 *
 * @param     parameter_index        The parameter entry's index.
 * @param     value                  Pointer to the variable where value is to be written.
 * @return                           Error code depending on accessibility of an entry.
 *   @retval  OK                     On success.
 */
error_code_t Parameter_ReadValue(int16_t parameter_index, int32_t * value);


/**
 * Get a parameter's value.
 * It is up to the caller to use a valid index,
 * otherwise the function will return 0
 *
 * @param       parameter_index        The parameter entry's index.
 * @return                             The parameter's value.
 */
int32_t Parameter_GetValue(int16_t parameter_index);


/**
 * Get a parameter's help text.
 * It is up to the caller to use a valid index,
 * otherwise the function will return an empty string.
 *
 * @param       parameter_index        The parameter entry's index.
 * @return                             The parameter's help text.
 */
const char * Parameter_GetHelp(int16_t parameter_index);


/**
 * Read the whole data of a parameter entry.
 *
 * @param       parameter_index        The parameter entry's index.
 * @param[out]  parameter_data         The pointer to the parameter_data_t structure.
 * @return                             Error code depending on accessibility of an entry.
 *   @retval    OK                     On success.
 */
error_code_t Parameter_ReadData(int16_t parameter_index, parameter_data_t * parameter_data);


/**
 * Save the current parameter values to non-volatile memory.
 *
 * @return        Error code depending on what went wrong during saving.
 *   @retval  OK  On success.
 */
error_code_t Parameter_Save(void);


/**
 * @} end of PARAMETER
 */

#endif // __PARAMETER_H__
