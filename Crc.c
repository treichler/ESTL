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
 * @file Crc.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Types.h"
#include "Crc.h"


// CRC-8/MAXIM (https://crccalc.com/)
uint8_t Crc_Crc8(const uint8_t *data, size_t length, uint8_t previous_crc8)
{
  static uint8_t const crc8_half_lookup[] = {
      0x00, 0x9d, 0x23, 0xBE, 0x46, 0xDB, 0x65, 0xF8, 0x8C, 0x11, 0xAF, 0x32, 0xCA, 0x57, 0xE9, 0x74,
  };
  while( length -- )
  {
    previous_crc8 = (previous_crc8 >> 4) ^ crc8_half_lookup[ (previous_crc8 ^  *data      ) & 0x0F ];
    previous_crc8 = (previous_crc8 >> 4) ^ crc8_half_lookup[ (previous_crc8 ^ (*data >> 4)) & 0x0F ];
    data++;
  }
  return previous_crc8;
}


// CRC-16/ARC (https://crccalc.com/)
uint16_t Crc_Crc16(const uint8_t * data, size_t length, uint16_t previous_crc16)
{
  static uint16_t const crc16_half_lookup[] = {
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
  };
  while( length -- )
  {
    previous_crc16 = (previous_crc16 >> 4) ^ crc16_half_lookup[ (previous_crc16 ^  *data      ) & 0x0F ];
    previous_crc16 = (previous_crc16 >> 4) ^ crc16_half_lookup[ (previous_crc16 ^ (*data >> 4)) & 0x0F ];
    data++;
  }
  return previous_crc16;
}


// CRC-32 (https://crccalc.com/)
uint32_t Crc_Crc32(const uint8_t * data, size_t length, uint32_t previous_crc32)
{
  static uint32_t const crc32_half_lookup[16] =
  {
    0x00000000,0x1DB71064,0x3B6E20C8,0x26D930AC,0x76DC4190,0x6B6B51F4,0x4DB26158,0x5005713C,
    0xEDB88320,0xF00F9344,0xD6D6A3E8,0xCB61B38C,0x9B64C2B0,0x86D3D2D4,0xA00AE278,0xBDBDF21C
  };
  previous_crc32 ^= 0xFFFFFFFF;
  while( length -- )
  {
    previous_crc32 = (previous_crc32 >> 4) ^ crc32_half_lookup[ (previous_crc32 ^  *data      ) & 0x0F ];
    previous_crc32 = (previous_crc32 >> 4) ^ crc32_half_lookup[ (previous_crc32 ^ (*data >> 4)) & 0x0F ];
    data++;
  }
  return ~previous_crc32; // same as crc ^ 0xFFFFFFFF
}


/*
uint32_t Crc_CortexCrc32(const uint8_t * data, size_t len, uint32_t previous_crc32 )
{
  static uint32_t const crc32_half_lookup[] = {
      0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
      0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd
  };
  while( len -- )
  {
    previous_crc32 = ( previous_crc32 << 4 ) ^ crc32_half_lookup[ (( ( previous_crc32 >> 24 ) ^ (*data << 0) ) & 0xF0) >> 4 ];
    previous_crc32 = ( previous_crc32 << 4 ) ^ crc32_half_lookup[ (( ( previous_crc32 >> 24 ) ^ (*data << 4) ) & 0xF0) >> 4 ];
    data ++;
  }
  return previous_crc32;
}
*/
