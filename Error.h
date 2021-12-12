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
 * @file Error.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __ERROR_H__
#define __ERROR_H__


/**
 * @ingroup ESTL
 * @defgroup ERROR  Error codes
 * @brief Error code Module
 *
 * Error provides a set of enumerated codes for several classes of errors
 * to simplify error tracking.
 * Additionally there exists functionality to to get human readable text
 * strings describing most of the error codes.
 * The latter is useful if error codes are forwarded to user interfaces.
 * @{
 */


/**
 *  Define error return codes
 */
typedef enum {
  OK                    = 0,    //!<  okay, no error
  UNKNOWN_ERROR         = -128, //!<  To be used if error cannot be classified
  INDEX_OUT_OF_BOUNDARY,        //!<  Index is out of boundary
  BELOW_LIMIT,                  //!<  Value is below its defined limit
  ABOVE_LIMIT,                  //!<  Value is above its defined limit
  VALUE_INVALID,                //!<  Value is invalid
  VALUE_UNAVAILABLE,            //!<  Value is unavailable
  NOT_INITIALIZED,              //!<  Call to an uninitialized module's function
  FUNCTION_UNAVAILABLE,         //!<  Function is unavailable
  ADDRESS_NOT_ACCESSIBLE,       //!<  Desired address is not accessible
  TIMEOUT,                      //!<  A general timeout occurred
  RESOURCE_BUSY,                //!<  The requested resource is currently busy
  BUFFER_TOO_SMALL,             //!<  Buffer is too small

  STORAGE_NOT_INITIALIZED,      //!<  Storage module is not initialized
  STORAGE_ENUM_MISMATCH,        //!<  Number of entries in storage and enumerator do not match
  STORAGE_CRC_MISMATCH,         //!<  CRC checksum in non volatile memory does not match
  STORAGE_NVMEM_TOO_SMALL,      //!<  Non-volatile memory is too small
  STORAGE_INDEX_MISMATCH,       //!<  Index in storage table and NV-memory do not match
  STORAGE_DATA_TOO_BIG,         //!<  Data does not fit into reserved non-volatile memory area
  STORAGE_NOT_ACCESSIBLE,       //!<  Data is not accessible

  PARAMETER_STORAGE_MISSING,    //!<  Parameter storage is missing
  PARAMETER_WRITE_PROTECTED,    //!<  Accessing write protected parameter value
  PARAMETER_ACCESS_DENIED,      //!<  Access is denied due to wrong access level
  PARAMETER_HIDDEN,             //!<  Parameter is hidden due to wrong access level
  PARAMETER_KEY_COLLISION,      //!<  Secret creates key collision
  PARAMETER_ENUM_MISMATCH,      //!<  Number of entries in parameter and enumerator do not match
  PARAMETER_CONTENT_CHANGE,     //!<  Content has changed due to new or gone non-volatile parameter
  PARAMETER_REV_MINOR_CHANGE,   //!<  Minor revision in parameter image has changed
  PARAMETER_REV_MAJOR_CHANGE,   //!<  Major revision in parameter image has changed
  PARAMETER_INDEX_MISMATCH,     //!<  Index in parameter table and NV-memory do not match
  PARAMETER_ENTRIES_MISMATCH,   //!<  Number of non-volatile parameter entries does not match non-volatile data

  DISPLAY_NO_CONTENT,           //!<  There is nothing to be displayed
  DISPLAY_CONTENT_TOO_LONG,     //!<  The content to be displayed is too long
  DISPLAY_DYNAMIC_ENTRIES_FULL, //!<  There are no more dynamic display update entries left

  RF_INVALID_DATA_SIZE,         //!<  Data size violates RF-modules restrictions
  RF_PLAUSIBILITY_CHECK_FAILED, //!<  Data plausibility check failed
  RF_TOKEN_MISMATCH,            //!<  At least two token within received package do not match
  RF_NO_DATA_RECEIVED,          //!<  There is no new data or received package does not contain data
  RF_CANNOT_SEND,               //!<  Sending is currently not possible

  SCOPE_IS_BUSY,                //!<  Scope is already running

  CAN_SDO_CONNECTION_FAILED,    //!<  CANopen SDO connection failed
  CAN_TX_MAILBOX_NOT_EMPTY,     //!<  The accessed CAN mailbox is not yet sent
  CAN_NO_TX_MAILBOX,            //!<  The accessed CAN mailbox does not exist

  I2C_TIMEOUT,                  //!<  I2C timeout
  I2C_ERROR,                    //!<  I2C general error

  NR_OF_ERRORS,
} error_code_t;


/**
 * Get a human readable textual representation of the error code
 * @param       error   The error code
 * @return              A pointer to the string containing the error's text
 */
const char * Error_GetMessage(error_code_t error);


/**
 * @} end of ERROR
 */


#endif // __ERROR_H__
