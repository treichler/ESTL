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
 * @file Debug.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Config.h"
#ifdef ESTL_ENABLE_DEBUG

#include "ESTL_Types.h"
#include "Error.h"
#include "Parameter.h"
#include "Debug.h"
#include "Debug_LookupTable.h"

typedef struct {
  int32_t address;
  int32_t mask;
} Debug_data_access_t;

struct {
  Debug_data_access_t   debug[ESTL_DEBUG_NR_OF_ENTRIES];
  uint8_t               index;
} Debug_data;


bool_t Debug_AddressIsWhiteListed( int32_t address )
{
  // TODO check if address is allowed
  return TRUE;
}


int32_t Debug_GetValue(uint8_t index)
{
  int32_t address, mask;

  if (index < ESTL_DEBUG_NR_OF_ENTRIES)
  {
    address = Debug_data.debug[index].address;
    mask = Debug_data.debug[index].mask;
//    if ((OK == Debug_CheckAddress(address)))
//    {
      if (mask & 0xFFFF0000)
        return (*(int32_t*)address) & mask;                         // read as 32 bit type
      else if (mask & 0xFF00)
        return (int32_t)((*(int16_t*)address) & (int16_t)mask);     // read as 16 bit type
      return (int32_t)((*(int8_t*)address) & (int8_t)mask);         // read as 8 bit type
//    }
  }
  return 0;
}


error_code_t Debug_AddrParameterFunction(parameter_function_t parameter_function, int32_t * address)
{
  if (parameter_function == PARAMETER_READ)
    *address = Debug_data.debug[Debug_data.index].address;
  if (parameter_function == PARAMETER_WRITE)
  {
    if( (! Parameter_CurrentAccessLevelIsDeveloper()) && (! Debug_AddressIsWhiteListed(*address)) )
      return NOT_ACCESSIBLE;
    Debug_data.debug[Debug_data.index].address = *address;
  }
  return OK;
}


error_code_t Debug_MaskParameterFunction(parameter_function_t parameter_function, int32_t * mask)
{
  if (parameter_function == PARAMETER_READ)
    *mask = Debug_data.debug[Debug_data.index].mask;
  if (parameter_function == PARAMETER_WRITE)
  {
    if( (! Parameter_CurrentAccessLevelIsDeveloper()) && (! Debug_AddressIsWhiteListed(Debug_data.debug[Debug_data.index].address)) )
      return NOT_ACCESSIBLE;
    Debug_data.debug[Debug_data.index].mask = *mask;
  }
  return OK;
}


error_code_t Debug_IndexParameterFunction(parameter_function_t parameter_function, int32_t * index)
{
  if( parameter_function == PARAMETER_READ )
    *index = (int32_t)Debug_data.index + 1;
  if( parameter_function == PARAMETER_WRITE )
    Debug_data.index = (*index) - 1;
  return OK;
}


error_code_t Debug_DataParameterFunction(parameter_function_t parameter_function, int32_t * value)
{
  int32_t address, mask;
  address = Debug_data.debug[Debug_data.index].address;
  mask = Debug_data.debug[Debug_data.index].mask;

  if( parameter_function == PARAMETER_READ )
  {
    // if mask is zero return value from address lookup table
    if (mask == 0x00000000)
      *value = Debug_LookupTableGetAddress(Debug_data.debug[Debug_data.index].address);
    else
      *value = Debug_GetValue(Debug_data.index);
  }
  if( parameter_function == PARAMETER_WRITE )
  {
    if( ! Parameter_CurrentAccessLevelIsDeveloper() )
      return NOT_ACCESSIBLE;

    // write value only if mask is not zero
    if (mask != 0)
    {
      // TODO write only masked bits
      if (mask & 0xFFFF0000)
        *(int32_t*)address = *value;                                      // write as 32 bit type
      else if (mask & 0xFF00)
        *(int16_t*)address = (int16_t)(*value);                           // write as 16 bit type
      else
        *(int8_t*)address = (int8_t)(*value);                             // write as 8 bit type
    }
  }
  return OK;
}


#endif // ESTL_ENABLE_DEBUG
