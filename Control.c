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
 * @file Control.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Types.h"
#include "Error.h"
#include "Control.h"


#define PROPORTIONAL_SHIFT      (8)
#define INTEGRAL_SHIFT          (16)


void Control_UpdateIntegralLimit( control_t * control )
{
  if( 0 != control->k_i )
  {
    control->i_limit_scale_max = (1 << INTEGRAL_SHIFT) / control->k_i;
    control->i_limit_scale_min = (1 << INTEGRAL_SHIFT) / control->k_i;
  }
  else
  {
    control->i_limit_scale_min = 0;
    control->i_limit_scale_max = 0;
  }
}

error_code_t Control_SetLimit( control_t * control, int16_t min, int16_t max )
{
  if( min >= max )
    return VALUE_INVALID;
  control->min = min;
  control->max = max;
//  Control_UpdateIntegralLimit(control);
  return OK;
}

void Control_SetKp( control_t * control, int16_t k_p )
{
  // TODO check k_p for allowed range, eventually return error code...
  control->k_p = k_p;
}


error_code_t Control_SetTn( control_t * control, int16_t t_n )
{
  return UNKNOWN_ERROR;
}


void Control_SetKi( control_t * control, int16_t k_i )
{
  // TODO check k_i for allowed range, eventually return error code...
  control->k_i = k_i;
  if( 0 == k_i )
    Control_ClearIntegral( control );
//  Control_UpdateIntegralLimit(control);
}


void Control_SetKc( control_t * control, int16_t k_c )
{
  // TODO check k_i for allowed range, eventually return error code...
  control->k_ic = k_c;
}


void Control_ClearIntegral( control_t * control )
{
  control->integral = 0;
}


int16_t Control_PI( control_t * control, int16_t error )
{
  int32_t old_integral, new_integral, integral_input;

  integral_input = error * control->k_i;
  old_integral = control->integral;
  new_integral = old_integral + integral_input;
  if( (new_integral > 0) && (old_integral < 0) && (integral_input < 0) )
    new_integral = INT32_MIN;
  if( (new_integral < 0) && (old_integral > 0) && (integral_input > 0) )
    new_integral = INT32_MAX;

  int32_t output = (((int32_t)error * control->k_p) >> PROPORTIONAL_SHIFT) + (new_integral >> INTEGRAL_SHIFT);

  // limit the output
  if( output < control->min )
  {
    output = control->min;
    // integral anti wind-up
    if( new_integral < old_integral )
      new_integral = old_integral;
  }
  else if( output > control->max )
  {
    output = control->max;
    // integral anti wind-up
    if( new_integral > old_integral )
      new_integral = old_integral;
  }
/*
  // clear integral...
  if( 0 == error )
  {
    if( new_integral > control->k_ic )
      new_integral -= control->k_ic;
    if( new_integral < (-control->k_ic) )
      new_integral += control->k_ic;
  }
*/
  control->integral = new_integral;

  return (int16_t)output;
/*
  // proportional term
  int32_t proportional = ((int32_t)error * control->k_p) >> PROPORTIONAL_SHIFT;
  if( proportional < control->min )
    proportional = control->min;
  if( proportional > control->max )
    proportional = control->max;

  // integral term
  int32_t integral = (control->integral * control->k_i) >> INTEGRAL_SHIFT;

  // limit output and integral (anti wind-up)
  int32_t output = proportional + integral;
  // limit output
  if( output < control->min )
  {
    output = control->min;
    control->integral = (control->min - proportional) * control->i_limit_scale_min;
  }
  if( output > control->max )
  {
    output = control->max;
    control->integral = (control->max - proportional) * control->i_limit_scale_max;
  }

  control->integral += error;

  return (int16_t)output;
*/
}
