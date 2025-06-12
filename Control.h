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
 * @file Control.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __CONTROL_H__
#define __CONTROL_H__

/**
 * @ingroup ESTL
 * @defgroup CONTROL  Control functions
 * @brief Basic control functions
 *
 * This module provides basic control algorithms.
 * @{
 */

typedef struct {
  int32_t integral;             //!<  controller's integral
  int32_t i_limit_scale_min;    //!<  experimental
  int32_t i_limit_scale_max;    //!<  experimental
  int16_t k_i;                  //!<  integral scaling factor
  int16_t k_ic;                 //!<  integral clearing factor (experimental)
  int16_t k_p;                  //!<  proportional scaling factor
  int16_t t_n;                  //!<  ...
  int16_t min;                  //!<  limiter's minimum value
  int16_t max;                  //!<  limiter's maximum value
} control_t;


error_code_t Control_SetLimit( control_t * control, int16_t min, int16_t max );
void Control_SetKp( control_t * control, int16_t k_p );
error_code_t Control_SetTn( control_t * control, int16_t t_n );
void Control_SetKi( control_t * control, int16_t k_i );
void Control_SetKc( control_t * control, int16_t k_c );
void Control_ClearIntegral( control_t * control );
int16_t Control_PI( control_t * control, int16_t error );

/**
 * @} end of CONTROL
 */

#endif // __CONTROL_H__
