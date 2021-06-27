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
 * @file Debug.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "ESTL_Defines.h"
#include "ESTL_Config.h"


/**
 * @ingroup  ESTL
 * @defgroup DEBUG  Debug
 * @brief    Debug module
 *
 * This module features simple debug functionality by providing access to
 * theoretically any resource located within a 32 bit address range.
 * Access is granted by providing the address and a bit-mask.
 * The amount of debug channels provided by this module is externally defined
 * in ESTL_Config.h with ESTL_DEBUG_NR_OF_ENTRIES.
 * @{
 */

#ifndef ESTL_DEBUG_NR_OF_ENTRIES
  #error "ESTL_DEBUG_NR_OF_ENTRIES needs to be set in ESTL_Config.h"
#endif

#if( (ESTL_DEBUG_NR_OF_ENTRIES > 16) || (ESTL_DEBUG_NR_OF_ENTRIES < 1) )
  #error "ESTL_DEBUG_NR_OF_ENTRIES is out of range"
#endif


/**
 * Get variable's content of desired debug index
 * @param[in]  index  Index of debug channel
 * @return            Variable's value respectively zero if index does not exist
 */
int32_t Debug_GetValue(uint8_t index);

/**
 * Parameter function to access and set debug channel index
 */
error_code_t Debug_IndexParameterFunction(parameter_function_t parameter_function, int32_t * index);

/**
 * Parameter function to access and set variable's address.
 * The function accesses the currently selected debug channel index.
 */
error_code_t Debug_AddrParameterFunction(parameter_function_t parameter_function, int32_t * address);

/**
 * Parameter function to access and set variable's access mask.
 * The function accesses the currently selected debug channel index.
 */
error_code_t Debug_MaskParameterFunction(parameter_function_t parameter_function, int32_t * mask);

/**
 * Parameter function to access and set variable's representation.
 * The function accesses the currently selected debug channel index.
 */
error_code_t Debug_ReprParameterFunction(parameter_function_t parameter_function, int32_t * value);

/**
 * Parameter function to read and write the variable.
 * The function accesses the currently selected debug channel index.
 */
error_code_t Debug_DataParameterFunction(parameter_function_t parameter_function, int32_t * value);


/**
 * @} end of DEBUG
 */

#endif // __DEBUG_H__
