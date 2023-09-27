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
 * @file Parse.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Types.h"
#include "Parse.h"


/**
 * Maximum number of fraction digits to be parsed.
 */
#define MAX_FRACTION_DIGITS (5)


uint16_t Parse_StrToFrac(char * str)
{
  uint32_t value = 0; //, factor = 1;
  uint8_t i = 0;
  const int32_t factor[MAX_FRACTION_DIGITS] = {
      429496730,
       42949673,
        4294968,
         429497,
          42950,
  };

  while( (*str >= '0') && (*str <= '9') && (i < MAX_FRACTION_DIGITS) )
  {
    value += (uint16_t)(*str - '0') * factor[i];
    i ++;
    str ++;
  }
  return (uint16_t)(value >> 16) + ((value & 0xF000) ? 1 : 0);
}


uint32_t Parse_StrToUint(char ** str, uint8_t radix)
{
  char *parse_end, *str_end;
  char digit;
  uint32_t value = 0, factor = 1;

  parse_end = *str;
  while( (*parse_end >= '0' && *parse_end <= '9') || \
         (*parse_end >= 'A' && *parse_end <= 'F') || (*parse_end >= 'a' && *parse_end <= 'f') )
    parse_end ++;

  str_end = parse_end;

  while( (parse_end--) != *str )
  {
    if( *parse_end >= '0' && *parse_end <= '9' )
      digit = *parse_end - '0';
    else if( *parse_end >= 'A' && *parse_end <= 'F' )
      digit = *parse_end - 'A' + 10;
    else if( *parse_end >= 'a' && *parse_end <= 'f' )
      digit = *parse_end - 'a' + 10;
    else
      break;

    value += digit * factor;
    factor *= (uint32_t)radix;
  }
  *str = str_end;
  return value;
}


int32_t Parse_StrToValue(char * str) //, int8_t radix)
{
  int32_t value = 0;
  int8_t radix = 10;
  bool_t value_is_negative = FALSE;

  // if argument starts with '0' it can be [hex], [oct], [bin] or zero
  if( *str == '0' )
  {
    str ++;
    if( (*str == 'x') || (*str == 'X') )
    {
      radix = 16;
      str ++;
    }
    else if( (*str >= '0') && (*str <= '7') )
    {
      radix = 8;
    }
    else if( (*str == 'b') || (*str == 'B') )
    {
      radix = 2;
      str ++;
    }
    else
    {
      radix = 10;
      str --;
    }
  }
  if( *str == '-' )
  {
    value_is_negative = TRUE;
    str ++;
  }
  value = Parse_StrToUint(&str, radix);
  if( (*str == '.') && (radix == 10) && (value <= INT16_MAX) && (value >= INT16_MIN) )
  {
    uint32_t dot_val = value;

    // evaluate value as fixed point number
    str++;
    value = (value << 16) + Parse_StrToFrac(str);

    // check if value is in representation aa.bb.cc respectively aa.bb.cc.dd
    // where aa, bb, cc and dd in range [0..255]
    if( (255 >= dot_val) && (! value_is_negative) )
    {
      uint32_t tmp = Parse_StrToUint(&str, radix);
      if( (255 >= tmp) && (*str == '.') )
      {
        str ++;
        dot_val <<= 16;
        dot_val  |= (tmp << 8);
        tmp = Parse_StrToUint(&str, radix);
        dot_val |= tmp;
        if( '.' == *str)
        {
          str ++;
          dot_val <<= 8;
          dot_val  |= Parse_StrToUint(&str, radix);
        }
        value = dot_val;
      }
    }
  }

  if( value_is_negative )
    return -value;
  return value;
}
