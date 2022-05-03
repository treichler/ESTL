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
 * @file Storage_I2cEeprom.c
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
#include <string.h>
#include "Error.h"
#include "Target.h"
#include "Storage_I2cEeprom.h"

#if( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC32 )
  // 24LC32 -- 4 kbyte
  #define EEPROM_SIZE                     (4096)
  #define EEPROM_PAGE_SIZE                (32)
  #define EEPROM_NR_OF_ADDR_BYTES         (2)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC16 )
  // 24LC16 -- 2 kbyte
  #define EEPROM_SIZE                     (2048)
  #define EEPROM_PAGE_SIZE                (16)
  #define EEPROM_NR_OF_ADDR_BYTES         (1)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC08 )
  // 24LC08 -- 1 kbyte
  #error "EEprom needs to be tested. If successful this error can be deleted"
  #define EEPROM_SIZE                     (1024)
  #define EEPROM_PAGE_SIZE                (16)
  #define EEPROM_NR_OF_ADDR_BYTES         (1)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC04 )
  // 24LC04 -- 512 byte
  #define EEPROM_SIZE                     (512)
  #define EEPROM_PAGE_SIZE                (16)
  #define EEPROM_NR_OF_ADDR_BYTES         (1)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC02 )
  // 24LC02 -- 256 byte
  #define EEPROM_SIZE                     (256)
  #define EEPROM_PAGE_SIZE                (8)
  #define EEPROM_NR_OF_ADDR_BYTES         (1)
#else
  #error "EEprom is not defined."
#endif

#ifdef ESTL_STORAGE_I2CEEPROM_7BIT_ADDR
  #define EEPROM_BUS_WRITE_ADDRESS        (ESTL_STORAGE_I2CEEPROM_7BIT_ADDR << 1)
#else
  #define EEPROM_BUS_WRITE_ADDRESS        (0xA0)
  #warning "EEPROM_BUS_WRITE_ADDRESS is set to default address"
#endif


/**
 * @name Definitions for I2C EEprom access
 * @{
 */
#define EEPROM_BUS_READ_ADDRESS         (0x01 | EEPROM_BUS_WRITE_ADDRESS)
#define EEPROM_PAGE_INDEX_MASK          (EEPROM_PAGE_SIZE - 1)
#define EEPROM_NR_OF_WRITE_RETRIES      (100)
/** @} */


inline int32_t StorageI2cEeprom_GetSize(void)
{
  return EEPROM_SIZE;
}


error_code_t StorageI2cEeprom_NvMemWrite(uint16_t addr, uint8_t *data, uint16_t size)
{
  uint16_t page_addr, current_max_len, retries;
  uint8_t tx_buffer[EEPROM_NR_OF_ADDR_BYTES + EEPROM_PAGE_SIZE];
  error_code_t i2c_write_status;
  uint8_t eeprom_bus_write_address = EEPROM_BUS_WRITE_ADDRESS;

  if((addr + size) >= EEPROM_SIZE)
    return STORAGE_DATA_TOO_BIG;

  page_addr = addr;
  while(size)
  {
    // prepare data
    current_max_len = EEPROM_PAGE_SIZE - (EEPROM_PAGE_INDEX_MASK & page_addr);
    if (current_max_len > size)
      current_max_len = size;
    size -= current_max_len;
    memcpy(&(tx_buffer[EEPROM_NR_OF_ADDR_BYTES]), data, current_max_len);
    data += current_max_len;

    // prepare memory/page address
#if( EEPROM_NR_OF_ADDR_BYTES > 1 )
    tx_buffer[0] = (uint8_t)(page_addr >> 8);
    tx_buffer[1] = (uint8_t)(page_addr & 0xFF);
#else
    tx_buffer[0] = (uint8_t)(page_addr & 0xFF);
    eeprom_bus_write_address = EEPROM_BUS_WRITE_ADDRESS | ((page_addr >> 7) & 0x0E);
#endif
    page_addr &= (~EEPROM_PAGE_INDEX_MASK);
    page_addr += EEPROM_PAGE_SIZE;

    retries = EEPROM_NR_OF_WRITE_RETRIES;
    while (retries --)
    {
      i2c_write_status = Target_I2cWrite(eeprom_bus_write_address, tx_buffer, (EEPROM_NR_OF_ADDR_BYTES + current_max_len));
      if (OK == i2c_write_status)
        break;
    }
    if (OK != i2c_write_status)
      return i2c_write_status;
  }
  return OK;
}


// TODO needs to be checked
#define EEPROM_READ_BLOCK_SIZE  (256)
#define EEPROM_READ_BLOCK_MASK  (EEPROM_READ_BLOCK_SIZE - 1)


error_code_t StorageI2cEeprom_NvMemRead(uint16_t addr, uint8_t *data, uint16_t size)
{
  uint16_t block_addr, current_max_len;
  uint8_t mem_addr[EEPROM_NR_OF_ADDR_BYTES];
  uint8_t eeprom_bus_write_address, eeprom_bus_read_address;
  error_code_t i2c_access_status;

  if((addr + size) >= EEPROM_SIZE)
    return STORAGE_DATA_TOO_BIG;

  block_addr = addr;
  while(size)
  {
    current_max_len = EEPROM_READ_BLOCK_SIZE - (block_addr & EEPROM_READ_BLOCK_MASK);
    if (current_max_len > size)
      current_max_len = size;
    size -= current_max_len;

#if( EEPROM_NR_OF_ADDR_BYTES > 1 )
    mem_addr[0] = (uint8_t)(block_addr >> 8);
    mem_addr[1] = (uint8_t)(block_addr & 0xFF);
    eeprom_bus_write_address = EEPROM_BUS_WRITE_ADDRESS;
    eeprom_bus_read_address  = EEPROM_BUS_READ_ADDRESS;
#else
    mem_addr[0] = (uint8_t)(block_addr & 0xFF);
    eeprom_bus_write_address = EEPROM_BUS_WRITE_ADDRESS | ((block_addr >> 7) & 0x0E);
    eeprom_bus_read_address  = EEPROM_BUS_READ_ADDRESS | ((block_addr >> 7) & 0x0E);
#endif
    i2c_access_status = Target_I2cWrite(eeprom_bus_write_address, mem_addr, sizeof(mem_addr));
    if (OK != i2c_access_status)
      return i2c_access_status;
    i2c_access_status = Target_I2cRead(eeprom_bus_read_address, data, current_max_len);
    if (OK != i2c_access_status)
      return i2c_access_status;

    data += current_max_len;
    block_addr += current_max_len;
  }

  return OK;
}
