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
 * @file Unit.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include <string.h>
#include "Print.h"
#include "Unit.h"


const char * Unit_GetString(unit_t unit)
{
#ifdef ESTL_ENABLE_UNIT_NAMES
  static char const * const unit_str[] =
  {
    [UNIT_NONE]                 = "",
    [UNIT_PERCENT]              = "%",
    [UNIT_DECIBEL]              = "dB",
    [UNIT_LUX]                  = "lx",
    [UNIT_REL_HUMIDITY]         = "%RH",
    [UNIT_CELSIUS]              = "°C",
    [UNIT_KELVIN]               = "K",
    [UNIT_U_SECOND]             = "µs",
    [UNIT_M_SECOND]             = "ms",
    [UNIT_SECOND]               = "s",
    [UNIT_MINUTE]               = "min",
    [UNIT_HOUR]                 = "h",
    [UNIT_HERTZ]                = "Hz",
    [UNIT_K_HERTZ]              = "kHz",
    [UNIT_MEG_HERTZ]            = "MHz",
    [UNIT_BAUD]                 = "Bd",
    [UNIT_RPM]                  = "rpm",
    [UNIT_JOULE]                = "J",
    [UNIT_WATT_SECOND]          = "Ws",
    [UNIT_WATT_HOUR]            = "Wh",
    [UNIT_K_WATT_HOUR]          = "kWh",
    [UNIT_M_WATT]               = "mW",
    [UNIT_WATT]                 = "W",
    [UNIT_M_AMPERE]             = "mA",
    [UNIT_AMPERE]               = "A",
    [UNIT_AMPERE_PER_SECOND]    = "A/s",
    [UNIT_M_VOLT]               = "mV",
    [UNIT_VOLT]                 = "V",
    [UNIT_OHM]                  = "Ohm",
    [UNIT_VOLT_PER_AMPERE]      = "V/A",
    [UNIT_M_VOLT_PER_AMPERE]    = "mV/A",
    [UNIT_U_METRE]              = "µm",
    [UNIT_M_METRE]              = "mm",
    [UNIT_METRE]                = "m",
    [UNIT_K_METRE]              = "km",
    [UNIT_M_METRE_PER_SECOND]   = "mm/s",
    [UNIT_METRE_PER_SECOND]     = "m/s",
    [UNIT_K_METRE_PER_HOUR]     = "km/h",
  };
  if (unit >= NR_OF_UNITS)
    unit = UNIT_NONE;
  return unit_str[unit];
#else
  return "";
#endif
}


void Unit_ValueToString(char * str, int str_len, int32_t value, repr_t repr)
{
  const char * fmt;
  switch(repr)
  {
    case REPR_HEX:
      fmt = "0x%X";
      break;
    case REPR_HEX_02:
      fmt = "0x%02X";
      break;
    case REPR_HEX_04:
      fmt = "0x%04X";
      break;
    case REPR_HEX_08:
      fmt = "0x%08X";
      break;
    case REPR_DEC:
      fmt = "%d";
      break;
    case REPR_DEC_U:
      fmt = "%u";
      break;
    case REPR_Q15_0:
      fmt = "%2.0q";
      break;
    case REPR_Q15_1:
      fmt = "%3.1q";
      break;
    case REPR_Q15_2:
      fmt = "%4.2q";
      break;
    case REPR_Q15_3:
      fmt = "%5.3q";
      break;
    case REPR_Q15_4:
      fmt = "%6.4q";
      break;
    case REPR_Q15_5:
      fmt = "%7.5q";
      break;
    default:
      fmt = "%d";
      break;
  }
  snprintf(str, str_len, (char*)fmt, value);
}


void Unit_PhysicalValueToString(char * str, int str_len, int32_t value, repr_t repr, unit_t unit)
{
  Unit_ValueToString(str, str_len, value, repr);
  int len = strlen(str);
  strncpy( str + len, Unit_GetString(unit), str_len-len );
}
