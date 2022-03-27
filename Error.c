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
 * @file Error.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Defines.h"
#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include "Error.h"


const char * Error_GetMessage(error_code_t error)
{
#ifdef ESTL_ENABLE_ERROR_MESSAGES
  static char const * const error_messages[] = {
    [UNKNOWN_ERROR                - UNKNOWN_ERROR] = "Unknown error",
    [INDEX_OUT_OF_BOUNDARY        - UNKNOWN_ERROR] = "Index out of boundary",
    [BELOW_LIMIT                  - UNKNOWN_ERROR] = "Below Limit",
    [ABOVE_LIMIT                  - UNKNOWN_ERROR] = "Above Limit",
    [VALUE_INVALID                - UNKNOWN_ERROR] = "Value invalid",
    [VALUE_UNAVAILABLE            - UNKNOWN_ERROR] = "Value unavailable",
    [FUNCTION_CALL_FAILED         - UNKNOWN_ERROR] = "Function call failed",
    [FUNCTION_UNAVAILABLE         - UNKNOWN_ERROR] = "Function unavailable",
    [NOT_INITIALIZED              - UNKNOWN_ERROR] = "Not accessible",
    [NOT_ACCESSIBLE               - UNKNOWN_ERROR] = "Not accessible",
    [TIMEOUT                      - UNKNOWN_ERROR] = "Timeout",
    [RESOURCE_BUSY                - UNKNOWN_ERROR] = "Resource busy",
    [BUFFER_TOO_SMALL             - UNKNOWN_ERROR] = "Buffer too small",

    [FLASH_WRITE_ERROR            - UNKNOWN_ERROR] = "FLASH write error",
    [FLASH_ERASE_ERROR            - UNKNOWN_ERROR] = "FLASH erase error",

    [STORAGE_NOT_INITIALIZED      - UNKNOWN_ERROR] = "Storage not initialized",
    [STORAGE_ENUM_MISMATCH        - UNKNOWN_ERROR] = "Storage enum mismatch",
    [STORAGE_CRC_MISMATCH         - UNKNOWN_ERROR] = "Storage CRC mismatch",
    [STORAGE_NVMEM_TOO_SMALL      - UNKNOWN_ERROR] = "Non-volatile memory too small",
    [STORAGE_INDEX_MISMATCH       - UNKNOWN_ERROR] = "Storage index mismatch",
    [STORAGE_DATA_TOO_BIG         - UNKNOWN_ERROR] = "Storage data too big",
    [STORAGE_DATA_UNAVAILABLE     - UNKNOWN_ERROR] = "Storage data unavailable",

    [PARAMETER_STORAGE_MISSING    - UNKNOWN_ERROR] = "Parameter storage missing",
    [PARAMETER_WRITE_PROTECTED    - UNKNOWN_ERROR] = "Parameter write protected",
    [PARAMETER_ACCESS_DENIED      - UNKNOWN_ERROR] = "Parameter access denied",
    [PARAMETER_HIDDEN             - UNKNOWN_ERROR] = "Parameter is hidden",
    [PARAMETER_ENUM_MISMATCH      - UNKNOWN_ERROR] = "Parameter enum mismatch",
    [PARAMETER_CONTENT_CHANGE     - UNKNOWN_ERROR] = "Parameter content change",
    [PARAMETER_REV_MINOR_CHANGE   - UNKNOWN_ERROR] = "Parameter revision minor change",
    [PARAMETER_REV_MAJOR_CHANGE   - UNKNOWN_ERROR] = "Parameter revision major change",

    [DISPLAY_NO_CONTENT           - UNKNOWN_ERROR] = "",
    [DISPLAY_CONTENT_TOO_LONG     - UNKNOWN_ERROR] = "",
    [DISPLAY_DYNAMIC_ENTRIES_FULL - UNKNOWN_ERROR] = "",

    [RF_INVALID_DATA_SIZE         - UNKNOWN_ERROR] = "",
    [RF_PLAUSIBILITY_CHECK_FAILED - UNKNOWN_ERROR] = "",
    [RF_TOKEN_MISMATCH            - UNKNOWN_ERROR] = "",
    [RF_NO_DATA_RECEIVED          - UNKNOWN_ERROR] = "",
    [RF_CANNOT_SEND               - UNKNOWN_ERROR] = "",

    [SCOPE_IS_BUSY                - UNKNOWN_ERROR] = "Scope is busy",

    [CAN_SDO_CONNECTION_FAILED    - UNKNOWN_ERROR] = "CAN SDO connection failed",
    [CAN_TX_MAILBOX_NOT_EMPTY     - UNKNOWN_ERROR] = "",
    [CAN_NO_TX_MAILBOX            - UNKNOWN_ERROR] = "",

    [I2C_TIMEOUT                  - UNKNOWN_ERROR] = "I2C timeout",
    [I2C_ERROR                    - UNKNOWN_ERROR] = "I2C general error",
  };
  if( OK == error )
    return "OK";
  error -= UNKNOWN_ERROR;
  if( error >= (sizeof(error_messages) / sizeof(const char*)) )
    error = 0;
  return error_messages[error];
#else
  return "";
#endif
}

