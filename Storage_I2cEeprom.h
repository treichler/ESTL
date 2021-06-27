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
 * @ingroup  STORAGE
 * @defgroup I2CEEPROM_STORAGE  I2C EEprom Storage
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
error_code_t StorageI2cEeprom_NvMemWrite(uint16_t addr, uint8_t *data, uint16_t size);


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
 * @} end of I2CEEPROM_STORAGE
 */

#endif // __STORAGE_I2CEEPROM_H__
