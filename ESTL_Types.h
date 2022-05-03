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
 * @file ESTL_Types.h
 * @brief Embedded Systems Tiny Library data types
 *
 * This file includes and defines several dedicated data-types and
 * conversion functionalities
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __ESTL_TYPES_H__
#define __ESTL_TYPES_H__

#include <stdint.h>
#include <stddef.h>


/**
 * Define the boolean data-type
 */
typedef enum {
  FALSE = 0,
  TRUE  = 1,
} bool_t;


/**
 * Define a type to represent a range
 */
typedef struct {
  int16_t min;  //!< Range's minimum value
  int16_t max;  //!< Range's maximum value
} range_t;


/**
 * Check if a value is within a given range.
 * @param [in]  value  The value is to be checked.
 * @param [in]  range  The range where the value is expected to fit.
 * @return
 *   @retval TRUE   Value fits to range.
 *   @retval FALSE  Value violates range.
 */
static inline bool_t ValueInRange(int16_t value, range_t range)
{
  return ((value >= range.min) && (value <= range.max)) ? TRUE : FALSE;
}


/**
 * Define an enumeration to describe the representation of a value.
 */
typedef enum {
  REPR_HEX,             //!< Hexadecimal without leading zeros
  REPR_HEX_02,          //!< Hexadecimal 8-bit with leading zeros
  REPR_HEX_04,          //!< Hexadecimal 16-bit with leading zeros
  REPR_HEX_08,          //!< Hexadecimal 32-bit with leading zeros
  REPR_DEC,             //!< Decimal signed
  REPR_DEC_U,           //!< Decimal unsigned
  REPR_Q15_0,           //!< Fixed point q15.16 with 0 decimal precision
  REPR_Q15_1,           //!< Fixed point q15.16 with 1 decimal precision
  REPR_Q15_2,           //!< Fixed point q15.16 with 2 decimal precision
  REPR_Q15_3,           //!< Fixed point q15.16 with 3 decimal precision
  REPR_Q15_4,           //!< Fixed point q15.16 with 4 decimal precision
  REPR_Q15_5,           //!< Fixed point q15.16 with 5 decimal precision
  NR_OF_REPRS           //!< This needs to be the last enumeration entry
} repr_t;


/**
 * Define the q15.16 fixed point data-type.
 */
typedef int32_t q15_t;

/**
 * @name Definitions for Q15.16 fixed point representation
 * @{
 */
#define Q15_SHIFT       (16)                    //!<  Decimal point position of Q15.16 representation
#define Q15_FACTOR      (1 << Q15_SHIFT)        //!<  Q15.16 scaling factor
/** @} */


/**
 * Helper to get float constants converted to fixed point representation (q15.16).
 * @code q15_t value = Q15(1.23); @endcode
 */
#define Q15(a)  ((q15_t)(a*Q15_FACTOR) == 0 ? 0 : (q15_t)(a*Q15_FACTOR) < 0 ? (q15_t)(a*Q15_FACTOR)-1 : (q15_t)(a*Q15_FACTOR)+1)


/**
 * Minimum of Q15 data type
 */
#define Q15_MIN INT32_MIN


/**
 * Maximum of Q15 data type
 */
#define Q15_MAX INT32_MAX

/**
 * Get the mantissa of a q15.16 fixed point representation.
 * @param [in] q15      The given q15.16 fixed point value
 * @return              The value's mantissa
 */
static inline int16_t q15_GetMantissa(q15_t q15)
{
  return (int16_t)(q15 >> 16);
}


/**
 * Get the fraction of a q15.16 fixed point representation.
 * @param [in] q15      The given q15.16 fixed point value
 * @return              The value's fraction
 */
static inline uint16_t q15_GetFraction(q15_t q15)
{
  return (uint16_t)(q15 & 0xFFFF);
}


#endif // __ESTL_TYPES_H__
