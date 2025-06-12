//----------------------------------------------------------------------------//
//  Copyright 2025 Clemens Treichler                                          //
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
 * @file Storage_EepromAlternate.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2025 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Types.h"
#include <string.h>
#include "Error.h"
#include "Crc.h"
#include "Storage.h"
#include "Storage_I2cEeprom.h"


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
 * @defgroup STORAGE_EEPROM_ALTERNATE  EEprom-based Storage with two alternating blocks
 * @brief    Organizes EEproms in two alternating blocks for non-volatile data storage
 *
 * EEprom-Alternate-based storage module relies on the advantage of random access
 * and in-place rewriting the storage cells.
 * Furthermore the EEprom's space is organized in two equal sized memory blocks.
 * Each image exists in both blocks, and write-access is performed alternating.
 * A counter within each image header is used to distinguish between the older and
 * the newer image.
 * Write-access is performed in-place of the older image, while the previous image
 * remains.
 * So an uncompleted write access does not corrupt the latest image.
 *
 * During initialization the images in both blocks are checked.
 * If both images are okay the newer one is selected.
 * If only one image is good, this one is used.
 * While image-read-access will return @ref OK in both cases, the image's vitality can
 * be checked by calling Storage_GetImageVitality().
 * The later function will return an error if one image is corrupted (non-mathing CRC).
 *
 * The hardware-specific implementation for EEproms could be found in
 * @ref STORAGE_I2CEEPROM.
 * @{
 */


#ifndef ESTL_STORAGE_HARDWARE
  #error "ESTL_STORAGE_HARDWARE is not defined"
#endif

#define AMOUNT_OF_ALTERNATING_BLOCKS    (2)     // must not be changed

#define PAGE_OPTIMIZED_IMAGE_SIZE(a)    ((a % I2C_EEPROM_PAGE_SIZE) ? (((a / I2C_EEPROM_PAGE_SIZE) + 1) * I2C_EEPROM_PAGE_SIZE) : a)

#if( PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_PARAMETER_IMAGE_SIZE ) + \
     PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_APPLICATION_IMAGE_SIZE ) + \
     PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_ADAPTIVE_DATA_IMAGE_SIZE ) > \
     (I2C_EEPROM_SIZE / AMOUNT_OF_ALTERNATING_BLOCKS) )
#error "Images are too big and will not fit into target EEprom."
#endif

#if( ESTL_STORAGE_PARAMETER_IMAGE_SIZE > ESTL_STORAGE_APPLICATION_IMAGE_SIZE)
  #define MAX_IMAGE_SIZE PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_PARAMETER_IMAGE_SIZE )
#else
  #define MAX_IMAGE_SIZE PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_APPLICATION_IMAGE_SIZE )
#endif
#if( ESTL_STORAGE_ADAPTIVE_DATA_IMAGE_SIZE > MAX_IMAGE_SIZE )
  #undef MAX_IMAGE_SIZE
  #define MAX_IMAGE_SIZE PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_ADAPTIVE_DATA_IMAGE_SIZE )
#endif

#if( (I2C_EEPROM_SIZE / AMOUNT_OF_ALTERNATING_BLOCKS) % I2C_EEPROM_PAGE_SIZE )
#error "EEProm size does not seem to be a 2^n multiple of page-size."
#endif


//-------------------------------------------------------------------------------------------------
//  Local type definitions
//-------------------------------------------------------------------------------------------------

/**
 * Storage header
 */
typedef struct {
  uint32_t     crc32;   //!<  Checksum of the payload
  uint16_t     size;    //!<  Total size of payload without header
  storage_id_t index;   //!<  Storage index where it belongs to
  uint8_t      counter; //!<  Counter to distinguish between older and newer image
} storage_header_t;


/**
 * Define the storage entry description
 */
typedef struct {
  uint16_t      addr[AMOUNT_OF_ALTERNATING_BLOCKS];
  error_code_t  error[AMOUNT_OF_ALTERNATING_BLOCKS];
  uint8_t       current_block;
  uint8_t       counter;
} storage_entry_t;


//-------------------------------------------------------------------------------------------------
//  Local prototypes
//-------------------------------------------------------------------------------------------------

static int32_t StorageEepromAlternate_ReadMinimalCheck( storage_id_t index, uint8_t block, storage_header_t * header, void *data, int16_t size );


//-------------------------------------------------------------------------------------------------
//  Local variables an constants
//-------------------------------------------------------------------------------------------------

struct {
  storage_entry_t storage_entry[NR_OF_STORAGES];
  bool_t          is_initialized;
} StorageEepromAlternate_data;


static const uint16_t estimated_image_sizes[] = {
    [STORAGE_PARAMETER_IMAGE]     = PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_PARAMETER_IMAGE_SIZE ),
    [STORAGE_APPLICATION_IMAGE]   = PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_APPLICATION_IMAGE_SIZE ),
    [STORAGE_ADAPTIVE_DATA_IMAGE] = PAGE_OPTIMIZED_IMAGE_SIZE( ESTL_STORAGE_ADAPTIVE_DATA_IMAGE_SIZE ),
};


//-------------------------------------------------------------------------------------------------
//  Implementation
//-------------------------------------------------------------------------------------------------

/**
 * See Storage_Init() for further information.
 */
error_code_t StorageEepromAlternate_Init( void )
{
  uint16_t b, i;
  uint16_t addr[AMOUNT_OF_ALTERNATING_BLOCKS] = {0, I2C_EEPROM_SIZE / 2};
  storage_header_t header;
  uint8_t buffer[MAX_IMAGE_SIZE - sizeof(storage_header_t)];

  for( i = 0; i < NR_OF_STORAGES; i ++ )
  {
    for( b = 0; b < AMOUNT_OF_ALTERNATING_BLOCKS; b ++ )
    {
      StorageEepromAlternate_data.storage_entry[i].addr[b] = addr[b];
      int32_t size = StorageEepromAlternate_ReadMinimalCheck( i, b, &header, buffer, sizeof(buffer) );
      error_code_t error = (size < 0) ? (error_code_t)size : OK;
      StorageEepromAlternate_data.storage_entry[i].error[b] = error;
      if( OK == error )
      {
        if( (0 == b) || (OK != StorageEepromAlternate_data.storage_entry[i].error[0]) ||
            (1 == (header.counter - StorageEepromAlternate_data.storage_entry[i].counter)) )
        {
          StorageEepromAlternate_data.storage_entry[i].counter = header.counter;
          StorageEepromAlternate_data.storage_entry[i].current_block = b;
        }
      }
      addr[b] += estimated_image_sizes[i];
    }
  }

  StorageEepromAlternate_data.is_initialized = TRUE;
  return OK;
}


int32_t StorageEepromAlternate_ReadMinimalCheck( storage_id_t index, uint8_t block, storage_header_t * header, void *data, int16_t size )
{
  error_code_t read_status;
  storage_entry_t * entry = &StorageEepromAlternate_data.storage_entry[index];

  // read header and perform plausibility checks
  read_status = StorageI2cEeprom_NvMemRead( entry->addr[block], (uint8_t*)header, sizeof(storage_header_t) );
  if( OK != read_status )
    return (int32_t)read_status;
  if( header->index != index )
    return (int32_t)STORAGE_INDEX_MISMATCH;
  if( header->size > size )
    return (int32_t)BUFFER_TOO_SMALL;

  // read data and perform CRC
  read_status = StorageI2cEeprom_NvMemRead( entry->addr[block] + sizeof(storage_header_t), data, header->size );
  if( OK != read_status )
    return (int32_t)read_status;
  uint32_t crc = Crc_Crc32((uint8_t*)header + 4, sizeof(storage_header_t) - 4, 0);
  if( header->crc32 != Crc_Crc32(data, header->size, crc) )
    return (int32_t)STORAGE_CRC_MISMATCH;
  return header->size;
}


/**
 * See Storage_Read() for further information.
 */
int32_t StorageEepromAlternate_Read(storage_id_t index, void *data, int16_t size)
{
  storage_header_t header;

  // plausibility checks
  if( ! StorageEepromAlternate_data.is_initialized )
    return STORAGE_NOT_INITIALIZED;
  if( index >= NR_OF_STORAGES )
    return (int32_t)INDEX_OUT_OF_BOUNDARY;

  // read data from EEprom
  uint8_t block = StorageEepromAlternate_data.storage_entry[index].current_block;
  return StorageEepromAlternate_ReadMinimalCheck( index, block, &header, data, size );
}


/**
 * See Storage_Write() for further information.
 */
error_code_t StorageEepromAlternate_Write(storage_id_t index, const void *data, int16_t size)
{
  error_code_t write_status;
  storage_header_t header;

  if( ! StorageEepromAlternate_data.is_initialized )
    return STORAGE_NOT_INITIALIZED;
  if( index >= NR_OF_STORAGES )
    return INDEX_OUT_OF_BOUNDARY;
  if( (size + sizeof(storage_header_t)) > estimated_image_sizes[index] )
    return STORAGE_DATA_TOO_BIG;

  storage_entry_t * entry = &StorageEepromAlternate_data.storage_entry[index];

  // prepare header
  header.index   = index;
  header.counter = entry->counter + 1;
  header.size    = size;
  uint32_t crc   = Crc_Crc32((uint8_t*)(&header) + 4, sizeof(storage_header_t) - 4, 0);
  header.crc32   = Crc_Crc32(data, size, crc);

  // write header and data to EEprom
  uint8_t write_block = (entry->current_block + 1) & 0x01;
  write_status = StorageI2cEeprom_NvMemWrite( entry->addr[write_block], (uint8_t*)(&header), sizeof(storage_header_t) );
  if( OK != write_status )
    return write_status;
  write_status = StorageI2cEeprom_NvMemWrite( entry->addr[write_block] + sizeof(storage_header_t), data, size );
  if( OK != write_status )
    return write_status;

  // update storage entry
  entry->counter       = header.counter;
  entry->current_block = write_block;

  return OK;
}


/**
 * See Storage_GetImageVitality() for further information.
 */
error_code_t StorageEepromAlternate_GetImageVitality( storage_id_t index )
{
  storage_entry_t * entry = &StorageEepromAlternate_data.storage_entry[index];

  if( index >= NR_OF_STORAGES )
    return (int32_t)INDEX_OUT_OF_BOUNDARY;

  // check if both blocks are okay
  if( (OK == entry->error[0]) && (OK == entry->error[1]) )
    return OK;

  // check if both blocks have errors
  if( (OK != entry->error[0]) && (OK != entry->error[1]) )
    return entry->error[entry->current_block];

  // check if we have a fall-back situation
  if( OK == entry->error[0] )
  {
    // block 0 is good
    if( STORAGE_CRC_MISMATCH == entry->error[1] )
      return STORAGE_IMAGE_UNCERTAIN;
  }
  else
  {
    // block 1 is good
    if( STORAGE_CRC_MISMATCH == entry->error[0] )
      return STORAGE_IMAGE_UNCERTAIN;
  }

  return OK;
}


/**
 * @} end of STORAGE_EEPROM_ALTERNATE
 */
