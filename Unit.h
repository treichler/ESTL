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
 * @file Unit.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __UNIT_H__
#define __UNIT_H__


/**
 * @ingroup  ESTL
 * @defgroup UNIT  Unit
 * @brief Unit Module
 *
 * This module provides physical units and functionality to convert
 * values to strings according to their representation.
 * @{
 */

/**
 * Enumerate physical units
 */
typedef enum {
  UNIT_NONE,
  UNIT_PERCENT,
  UNIT_DECIBEL,
  UNIT_LUX,
  UNIT_REL_HUMIDITY,
  UNIT_CELSIUS,
  UNIT_KELVIN,
  UNIT_SECOND,
  UNIT_M_SECOND,
  UNIT_HERTZ,
  UNIT_K_HERTZ,
  UNIT_MEG_HERTZ,
  UNIT_BAUD,
  UNIT_RPM,
  UNIT_JOULE,
  UNIT_WATT_HOUR,
  UNIT_KILO_WATT_HOUR,
  UNIT_WATT,
  UNIT_M_WATT,
  UNIT_AMPERE,
  UNIT_M_AMPERE,
  UNIT_VOLT,
  UNIT_M_VOLT,
  UNIT_K_METRE,
  UNIT_METRE,
  UNIT_M_METRE,
  UNIT_U_METRE,
  UNIT_METRE_PER_SECOND,
  UNIT_M_METRE_PER_SECOND,
  UNIT_K_METRE_PER_HOUR,
  NR_OF_UNITS
} unit_t;


/**
 * Get the string representation of a physical unit
 *
 * @param     unit       Physical unit
 * @return               Physical unit's string representation
 */
const char * Unit_GetString(unit_t unit);


/**
 * Convert a value according to its representation to string.
 *
 * @param     str        Buffer where the string is written to
 * @param     str_len    Length of the provided buffer
 * @param     value      Value to be converted
 * @param     repr       The value's representation
 */
void Unit_ValueToString(char * str, int str_len, int32_t value, repr_t repr);


/**
 * Convert a value according to its representation and physical unit to string.
 *
 * @param     str        Buffer where the string is written to
 * @param     str_len    Length of the provided buffer
 * @param     value      Value to be converted
 * @param     repr       The value's representation
 * @param     unit       The value's physical unit
 */
void Unit_PhysicalValueToString(char * str, int str_len, int32_t value, repr_t repr, unit_t unit);


/**
 * @} end of UNIT
 */

#endif // __UNIT_H__
