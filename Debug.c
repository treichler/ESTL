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
#include "Debug_Access.h"

typedef struct {
  int32_t address;
  int32_t mask;
} Debug_data_access_t;

struct {
  Debug_data_access_t   debug[ESTL_DEBUG_NR_OF_ENTRIES];
  uint8_t               index;
} Debug_data;


int32_t Debug_GetValue(uint8_t index)
{
  if (index < ESTL_DEBUG_NR_OF_ENTRIES)
  {
    Debug_data_access_t *data = &Debug_data.debug[index];
    if( data->mask & 0xFFFF0000 )
      return (*(int32_t*)data->address) & data->mask;                         // read as 32 bit type
    else if( data->mask & 0xFF00 )
      return (int32_t)((*(int16_t*)data->address) & (int16_t)data->mask);     // read as 16 bit type
    return (int32_t)((*(int8_t*)data->address) & (int8_t)data->mask);         // read as 8 bit type
  }
  return 0;
}


error_code_t Debug_AddrParameterFunction(function_call_t function_call, int32_t * address)
{
  if (function_call == FUNCTION_READ)
    *address = Debug_data.debug[Debug_data.index].address;
  if (function_call == FUNCTION_WRITE)
  {
    if( (! Parameter_CurrentAccessLevelIsDeveloper()) && (! DebugAccess_AddressIsWhiteListed(*address)) )
    {
      Debug_data.debug[Debug_data.index].address = 0;
      Debug_data.debug[Debug_data.index].mask    = 0;
      return NOT_ACCESSIBLE;
    }
    Debug_data.debug[Debug_data.index].address = *address;
  }
  return OK;
}


error_code_t Debug_MaskParameterFunction(function_call_t function_call, int32_t * mask)
{
  if (function_call == FUNCTION_READ)
    *mask = Debug_data.debug[Debug_data.index].mask;
  if (function_call == FUNCTION_WRITE)
  {
    if( (! Parameter_CurrentAccessLevelIsDeveloper()) && (! DebugAccess_AddressIsWhiteListed(Debug_data.debug[Debug_data.index].address)) )
    {
      Debug_data.debug[Debug_data.index].address = 0;
      Debug_data.debug[Debug_data.index].mask    = 0;
      return NOT_ACCESSIBLE;
    }
    Debug_data.debug[Debug_data.index].mask = *mask;
  }
  return OK;
}


error_code_t Debug_IndexParameterFunction(function_call_t function_call, int32_t * index)
{
  if( function_call == FUNCTION_READ )
    *index = (int32_t)Debug_data.index + 1;
  if( function_call == FUNCTION_WRITE )
    Debug_data.index = (*index) - 1;
  return OK;
}


error_code_t Debug_DataParameterFunction(function_call_t function_call, int32_t * value)
{
  int32_t address, mask;
  address = Debug_data.debug[Debug_data.index].address;
  mask = Debug_data.debug[Debug_data.index].mask;

  if( function_call == FUNCTION_READ )
  {
    // if mask is zero return value from address lookup table
    if (mask == 0x00000000)
      *value = DebugAccess_LookupTableGetAddress(Debug_data.debug[Debug_data.index].address);
    else
      *value = Debug_GetValue(Debug_data.index);
  }
  if( function_call == FUNCTION_WRITE )
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
