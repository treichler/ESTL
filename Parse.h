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
 * @file Parse.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __PARSE_H__
#define __PARSE_H__


/**
 * @ingroup  ESTL
 * @defgroup PARSE  Parse
 * @brief String Parsing Functionality
 *
 * Parse provides basic functions to convert numbers encoded in strings
 * to variables.
 * @{
 */

/**
 * This function interprets the provided number string as decimal place
 * and converts it to a 16-bit binary fraction.
 *
 * @param     str        Number string
 * @return               Binary fraction
 */
uint16_t Parse_StrToFrac(char * str);


/**
 * Convert a number string to unsigned integer value according to the
 * given radix. After parsing the string's address is set to the end
 * of the parsed number.
 *
 * @param     str        Pointer to number string's address
 * @param     radix      Number's radix
 * @return               Integer value
 */
uint32_t Parse_StrToUint(char ** str, uint8_t radix);


/**
 * Convert a formated number string to integer respectively Q15 value.
 * Depending on the format the function can parse following values:
 * - '0x123' hexadecimal coded value
 * - '0123'  octal coded value
 * - '0b101' binary coded value
 * - '123'   decimal coded value
 * - '12.3'  real number coded value
 *
 * @param     str        Number string
 * @return               Integer respectively Q15 coded value
 */
int32_t Parse_StrToValue(char * str);


/**
 * @} end of PARSE
 */

#endif // __PARSE_H__
