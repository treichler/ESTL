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
 * @file Storage.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __STORAGE_H__
#define __STORAGE_H__


/**
 * @ingroup  ESTL
 * @defgroup STORAGE  Storage
 * @brief    Storage module
 *
 * The storage module provides functionality to save data to some kind of
 * non-volatile memory, where the data is categorized according to its usage.
 * Every data-block holds its header containing data size and a checksum
 * to grant data integrity.
 * @{
 */

/**
 * Enumerate the different data-blocks
 */
typedef enum {
  STORAGE_PARAMETER_IMAGE,
  STORAGE_APPLICATION_IMAGE,
  STORAGE_ADAPTIVE_DATA_IMAGE,
  NR_OF_STORAGES
} storage_id_t;


/**
 * Initialize the storage module.
 * @return                           Error code.
 *   @retval  OK                     On success.
 *   @retval  STORAGE_ENUM_MISMATCH  Storage_table does not fit to data-block enumeration
 *   @retval  STORAGE_SIZE_MISMATCH  Requested data-block sizes do not fit into non-volatile memory
 */
error_code_t Storage_Init(void);


// error_code_t Storage_Delete(storage_id_t index);


/**
 * Write data to non-volatile memory.
 * @param[in] index                  Index of the desired data-block
 * @param[in] data                   Pointer to the data to be stored
 * @param[in] size                   Size of the data to be stored
 * @return                           Error code depending on write success.
 *   @retval  OK                     On success.
 */
error_code_t Storage_Write(storage_id_t index, void *data, int16_t size);


/**
 * Read data from non-volatile memory.
 * @param[in] index                  Index of the desired data-block
 * @param[in] data                   Pointer to buffer where the data should be read
 * @param[in] size                   Size of the provided data-buffer
 * @return                           Zero and positive values represent the read data size,
 *                                   negative values represent the error code on read fail.
 */
int32_t Storage_Read(storage_id_t index, void *data, int16_t size);


/**
 * @} end of STORAGE
 */

#endif // __STORAGE_H__
