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
 * @file Storage_I2cEeprom.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __STORAGE_I2CEEPROM_H__
#define __STORAGE_I2CEEPROM_H__


/**
 * @ingroup  STORAGE_EEPROM
 * @defgroup STORAGE_I2CEEPROM  I2C EEprom Storage
 * @brief External I2C EEprom Storage Module
 *
 * This module provides functionality to access an externally connected
 * I2C EEprom. Since this module relies on externally connected hardware
 * and MCU's periphery, dedicated I2C-functions have to be provided in
 * Target.c respectively Target.h, namely:
 *   - Target_I2cRead()
 *   - Target_I2cWrite()
 * @{
 */

#if( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC64 )
  // 24LC64 -- 8 kbyte
  #define I2C_EEPROM_SIZE                       (8192)
  #define I2C_EEPROM_PAGE_SIZE                  (32)
  #define I2C_EEPROM_NR_OF_ADDR_BYTES           (2)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC32 )
  // 24LC32 -- 4 kbyte
  #define I2C_EEPROM_SIZE                       (4096)
  #define I2C_EEPROM_PAGE_SIZE                  (32)
  #define I2C_EEPROM_NR_OF_ADDR_BYTES           (2)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC16 )
  // 24LC16 -- 2 kbyte
  #define I2C_EEPROM_SIZE                       (2048)
  #define I2C_EEPROM_PAGE_SIZE                  (16)
  #define I2C_EEPROM_NR_OF_ADDR_BYTES           (1)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC08 )
  // 24LC08 -- 1 kbyte
  #error "EEprom needs to be tested. If successful this error can be deleted"
  #define I2C_EEPROM_SIZE                       (1024)
  #define I2C_EEPROM_PAGE_SIZE                  (16)
  #define I2C_EEPROM_NR_OF_ADDR_BYTES           (1)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC04 )
  // 24LC04 -- 512 byte
  #define I2C_EEPROM_SIZE                       (512)
  #define I2C_EEPROM_PAGE_SIZE                  (16)
  #define I2C_EEPROM_NR_OF_ADDR_BYTES           (1)
#elif( ESTL_STORAGE_I2CEEPROM == ESTL_STORAGE_I2CEEPROM_24LC02 )
  // 24LC02 -- 256 byte
  #define I2C_EEPROM_SIZE                       (256)
  #define I2C_EEPROM_PAGE_SIZE                  (8)
  #define I2C_EEPROM_NR_OF_ADDR_BYTES           (1)
#else
  #error "EEprom is not defined."
#endif

/**
 * Get the size of the EEprom which is connected to I2C bus
 *
 * @return               Size of EEprom.
 */
int32_t StorageI2cEeprom_GetSize(void);


/**
 * Write data to an EEprom which is connected to I2C bus.
 *
 * @param     addr       Start address where to write.
 * @param     data       Pointer to data to be written.
 * @param     size       Size of data.
 * @return               Error code.
 */
error_code_t StorageI2cEeprom_NvMemWrite(uint16_t addr, const uint8_t *data, uint16_t size);


/**
 * Read data from an EEprom which is connected to I2C bus.
 *
 * @param     addr       Start address where to read.
 * @param     data       Pointer to data to be read.
 * @param     size       Size of data.
 * @return               Error code.
 */
error_code_t StorageI2cEeprom_NvMemRead(uint16_t addr, uint8_t *data, uint16_t size);


/**
 * @} end of STORAGE_I2CEEPROM
 */

#endif // __STORAGE_I2CEEPROM_H__
