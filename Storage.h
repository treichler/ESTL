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

#include "ESTL_Defines.h"
#include "ESTL_Config.h"


/**
 * @ingroup  ESTL
 * @defgroup STORAGE  Storage
 * @brief    Storage module
 *
 * The storage module provides a common interface to save and access data in
 * some kind of non-volatile memory, where the data is categorized according
 * to its usage (see ::storage_id_t).
 * Depending on the memory technology dedicated algorithms are required.
 * The dedicated implementations could be found in storage's sub-modules
 * @ref STORAGE_EEPROM and @ref STORAGE_FLASH.
 * In the file ESTL_Config.h the desired algorithm is configured.
 * All algorithms have in common, that each data-block has a related header
 * containing data-block identifier, data size and a checksum to grant data
 * integrity.
 * @{
 */

/**
 * @enum storage_id_t
 *
 * Enumerate the different data-blocks
 */
typedef enum {
  STORAGE_PARAMETER_IMAGE,      //!< Storage-block identifier for parameter image
  STORAGE_APPLICATION_IMAGE,    //!< Storage-block identifier for non-volatile application data
  STORAGE_ADAPTIVE_DATA_IMAGE,  //!< Storage-block identifier for adaptive data
  NR_OF_STORAGES                //!< Reserved for indicating the amount of storage-blocks
} storage_id_t;


/** @cond */
// EEprom storage
extern error_code_t StorageEeprom_Init( void );
extern error_code_t StorageEeprom_Write( storage_id_t index, void *data, int16_t size );
extern int32_t StorageEeprom_Read( storage_id_t index, void *data, int16_t size );

// Flash storage
extern error_code_t StorageFlash_Init( void );
extern error_code_t StorageFlash_Write( storage_id_t index, void *data, int16_t size );
extern int32_t StorageFlash_Read( storage_id_t index, void *data, int16_t size );
/** @endcond */


/**
 * Initialize the storage module.
 * @return                           Error code.
 *   @retval  OK                     On success.
 *   @retval  STORAGE_ENUM_MISMATCH  Storage_table does not fit to data-block enumeration
 *   @retval  STORAGE_SIZE_MISMATCH  Requested data-block sizes do not fit into non-volatile memory
 */
static inline error_code_t Storage_Init(void)
{
#if( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_FAKE_NV_MEMORY )
  return StorageEeprom_Init();
#elif( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_I2CEEPROM )
  return StorageEeprom_Init();
#elif( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_FLASH )
  return StorageFlash_Init();
#else
  #error "Unknown ESTL_STORAGE_HARDWARE"
#endif
}


/**
 * Write data to non-volatile memory.
 * @param[in] index                  Index of the desired data-block
 * @param[in] data                   Pointer to the data to be stored
 * @param[in] size                   Size of the data to be stored
 * @return                           Error code depending on write success.
 *   @retval  OK                     On success.
 */
static inline error_code_t Storage_Write(storage_id_t index, void *data, int16_t size)
{
#if( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_FAKE_NV_MEMORY )
  return StorageEeprom_Write( index, data, size );
#elif( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_I2CEEPROM )
  return StorageEeprom_Write( index, data, size );
#elif( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_FLASH )
  return StorageFlash_Write( index, data, size );
#endif
}


/**
 * Read data from non-volatile memory.
 * @param[in] index                  Index of the desired data-block
 * @param[in] data                   Pointer to buffer where the data should be read
 * @param[in] size                   Size of the provided data-buffer
 * @return                           Zero and positive values represent the read data size,
 *                                   negative values represent the error code on read fail.
 */
static inline int32_t Storage_Read(storage_id_t index, void *data, int16_t size)
{
#if( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_FAKE_NV_MEMORY )
  return StorageEeprom_Read( index, data, size );
#elif( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_I2CEEPROM )
  return StorageEeprom_Read( index, data, size );
#elif( ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_FLASH )
  return StorageFlash_Read( index, data, size );
#endif
}


/**
 * @} end of STORAGE
 */

#endif // __STORAGE_H__
