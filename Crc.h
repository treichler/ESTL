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
 * @file Crc.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __CRC_H__
#define __CRC_H__


/**
 * @ingroup ESTL
 * @defgroup CRC  CRC functions
 * @brief Cyclic redundancy check Module
 *
 * CRC provides functions for cyclic redundancy checks.
 * There are algorithms available for 8, 16 and 32 bit CRCs.
 * The implementations are based on half-byte CRC tables which is a good
 * trade-off between code size and speed.
 * @{
 */


/**
 * Calculate CRC8 checksum
 * @param[in]  data    Pointer to the data to calculate CRC
 * @param[in]  length  Length of the provided data
 * @param[in]  crc8    CRC initialization value respectively previously calculated CRC
 * @return             Calculated checksum
 */
uint8_t Crc_Crc8(const uint8_t * data, size_t length, uint8_t crc8);


/**
 * Calculate CRC16 checksum
 * @param[in]  data    Pointer to the data to calculate CRC
 * @param[in]  length  Length of the provided data
 * @param[in]  crc16   CRC initialization value respectively previously calculated CRC
 * @return             Calculated checksum
 */
uint16_t Crc_Crc16(const uint8_t * data, size_t length, uint16_t crc16);


/**
 * Calculate CRC32 checksum
 * @param[in]  data    Pointer to the data to calculate CRC
 * @param[in]  length  Length of the provided data
 * @param[in]  crc32   CRC initialization value respectively previously calculated CRC
 * @return             Calculated checksum
 */
uint32_t Crc_Crc32(const uint8_t * data, size_t length, uint32_t crc32);


/**
 * @} end of CRC
 */

#endif // __CRC_H__
