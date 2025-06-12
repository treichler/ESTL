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
 * @file Storage_Eeprom.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Types.h"
#include <string.h>
#include "Error.h"
#include "Crc.h"
#include "Storage.h"


#ifndef ESTL_STORAGE_PARAMETER_IMAGE_SIZE
#define ESTL_STORAGE_PARAMETER_IMAGE_SIZE        (0)
#warning "ESTL_STORAGE_PARAMETER_IMAGE_SIZE is not defined, so it is set to 0"
#endif
#ifndef ESTL_STORAGE_APPLICATION_IMAGE_SIZE
#define ESTL_STORAGE_APPLICATION_IMAGE_SIZE      (0)
#warning "ESTL_STORAGE_APPLICATION_IMAGE_SIZE is not defined, so it is set to 0"
#endif
#ifndef ESTL_STORAGE_ADAPTIVE_DATA_IMAGE_SIZE
#define ESTL_STORAGE_ADAPTIVE_DATA_IMAGE_SIZE    (0)
#warning "ESTL_STORAGE_ADAPTIVE_DATA_IMAGE_SIZE is not defined, so it is set to 0"
#endif


/**
 * @ingroup  STORAGE
 * @defgroup STORAGE_EEPROM  EEprom-based Storage
 * @brief    Uses EEproms for non-volatile data storage
 *
 * EEprom-based storage module relies on the advantage of random access and
 * in-place rewriting the storage cells.
 * Therefore the location and the size for each data-block (see ::::storage_id_t)
 * is fixed and has to be defined in advance.
 * The hardware-specific implementation for EEproms could be found in
 * @ref STORAGE_I2CEEPROM.
 * @{
 */


//-------------------------------------------------------------------------------------------------
//  Local prototypes
//-------------------------------------------------------------------------------------------------
static inline error_code_t Storage_NvMemRead(uint16_t addr, uint8_t *data, uint16_t size);
static inline error_code_t Storage_NvMemWrite(uint16_t addr, const uint8_t *data, uint16_t size);
static inline int32_t Storage_GetSize(void);


struct {
  bool_t        is_initialized;
#if (ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_FAKE_NV_MEMORY)
#define FAKE_NV_MEMORY_size  (256)
  uint8_t       fake_nv_memory[FAKE_NV_MEMORY_size];
#endif
} StorageEeprom_data;


#ifndef ESTL_STORAGE_HARDWARE
  #error "ESTL_STORAGE_HARDWARE is not defined"
#endif

#if (ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_I2CEEPROM)
  #include "Storage_I2cEeprom.h"
/**
 * @name Wrapper for external I2C EEprom storage functions
 * @{
 */
error_code_t Storage_NvMemRead(uint16_t addr, uint8_t *data, uint16_t size)
{
  return StorageI2cEeprom_NvMemRead(addr, data, size);
}

error_code_t Storage_NvMemWrite(uint16_t addr, const uint8_t *data, uint16_t size)
{
  return StorageI2cEeprom_NvMemWrite(addr, data, size);
}

int32_t Storage_GetSize(void)
{
  return StorageI2cEeprom_GetSize();
}

/** @} */
#elif (ESTL_STORAGE_HARDWARE == ESTL_STORAGE_HARDWARE_FAKE_NV_MEMORY)
/**
 * @name Wrapper for faking storage functions in RAM
 * @{
 */
error_code_t Storage_NvMemWrite(uint16_t addr, const uint8_t *data, uint16_t size)
{
  if((addr + size) >= sizeof(StorageEeprom_data.fake_nv_memory))
    return NOT_ACCESSIBLE;
  memcpy(StorageEeprom_data.fake_nv_memory+addr, data, size);
  return OK;
}

error_code_t Storage_NvMemRead(uint16_t addr, uint8_t *data, uint16_t size)
{
  if((addr + size) >= sizeof(StorageEeprom_data.fake_nv_memory))
    return NOT_ACCESSIBLE;
  memcpy(data, StorageEeprom_data.fake_nv_memory+addr, size);
  return OK;
}

int32_t Storage_GetSize(void)
{
  return FAKE_NV_MEMORY_size;
}
/** @} */
#endif


#ifndef STORAGE_START_ADDRESS
  #define STORAGE_START_ADDRESS   (0x0000)
#endif


/**
 * Storage header
 */
typedef struct {
  uint32_t     crc32;   //!<  Checksum of the payload
  uint16_t     size;    //!<  Total size of payload without header
  storage_id_t index;   //!<  Storage index where it belongs to
} storage_header_t;


/**
 * Define the storage entry description
 */
typedef struct
{
  const uint16_t        size;   //!<  Size in bytes to be reserved
  uint16_t              addr;   //!<  Start address of storage entry
} storage_entry_t;


/**
 * Initialize
 */
storage_entry_t Storage_table[] = {
// storage-index                   size                                 addr
  [STORAGE_PARAMETER_IMAGE]     = {ESTL_STORAGE_PARAMETER_IMAGE_SIZE,      0},
  [STORAGE_APPLICATION_IMAGE]   = {ESTL_STORAGE_APPLICATION_IMAGE_SIZE,    0},
  [STORAGE_ADAPTIVE_DATA_IMAGE] = {ESTL_STORAGE_ADAPTIVE_DATA_IMAGE_SIZE,  0},
};


#define NR_OF_STORAGE_ENTRIES  (sizeof(Storage_table) / sizeof(storage_entry_t))


/**
 * See Storage_Init() for further information.
 */
error_code_t StorageEeprom_Init(void)
{
  uint16_t i;
  uint32_t addr = STORAGE_START_ADDRESS;
  if(NR_OF_STORAGE_ENTRIES != NR_OF_STORAGES)
    return STORAGE_ENUM_MISMATCH;

  for (i = 0; i < NR_OF_STORAGES; i ++)
  {
    Storage_table[i].addr = addr;
    addr += Storage_table[i].size;
  }
  if( addr > Storage_GetSize() )
    return STORAGE_NVMEM_TOO_SMALL;

  StorageEeprom_data.is_initialized = TRUE;
  return OK;
}


/**
 * See Storage_Write() for further information.
 */
error_code_t StorageEeprom_Write(storage_id_t index, const void *data, int16_t size)
{
  error_code_t write_status;
  storage_header_t header;
  if( ! StorageEeprom_data.is_initialized )
    return STORAGE_NOT_INITIALIZED;
  if( index >= NR_OF_STORAGES )
    return INDEX_OUT_OF_BOUNDARY;
  if( (size + sizeof(header)) > Storage_table[index].size )
    return STORAGE_DATA_TOO_BIG;
  header.index = index;
  header.size = size;
  uint32_t crc = Crc_Crc32((uint8_t*)(&header) + 4, sizeof(storage_header_t) - 4, 0);
  header.crc32 = Crc_Crc32(data, size, crc);
  write_status = Storage_NvMemWrite(Storage_table[index].addr, (uint8_t*)(&header), sizeof(header));
  if( OK != write_status )
    return write_status;
  write_status = Storage_NvMemWrite(Storage_table[index].addr + sizeof(header), data, size);
  if( OK != write_status )
    return write_status;
  return OK;
}


/**
 * See Storage_Read() for further information.
 */
int32_t StorageEeprom_Read(storage_id_t index, void *data, int16_t size)
{
  error_code_t read_status;
  storage_header_t header;
  if( ! StorageEeprom_data.is_initialized )
    return STORAGE_NOT_INITIALIZED;
  if( index >= NR_OF_STORAGES )
    return (int32_t)INDEX_OUT_OF_BOUNDARY;
  read_status = Storage_NvMemRead(Storage_table[index].addr, (uint8_t*)(&header), sizeof(header));
  if( OK != read_status )
    return (int32_t)read_status;
  if( header.index != index )
    return (int32_t)STORAGE_INDEX_MISMATCH;
  if( header.size > size )
    return (int32_t)BUFFER_TOO_SMALL;
  read_status = Storage_NvMemRead(Storage_table[index].addr + sizeof(header), data, header.size);
  if( OK != read_status )
    return (int32_t)read_status;
  uint32_t crc = Crc_Crc32((uint8_t*)(&header) + 4, sizeof(storage_header_t) - 4, 0);
  if( header.crc32 != Crc_Crc32(data, header.size, crc) )
    return (int32_t)STORAGE_CRC_MISMATCH;
  return header.size;
}


/**
 * @} end of STORAGE_EEPROM
 */
