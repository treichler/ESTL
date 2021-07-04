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
 * @file Scope.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Config.h"

#if( defined(ESTL_ENABLE_SCOPE) && !defined(ESTL_ENABLE_DEBUG) )
#warning "ESTL_ENABLE_SCOPE has no effect, since ESTL_ENABLE_DEBUG also needs to be set."
#endif

#if( defined(ESTL_ENABLE_SCOPE) && defined(ESTL_ENABLE_DEBUG) )

#if( ESTL_DEBUG_NR_OF_ENTRIES > 127 )
#error "ESTL_DEBUG_NR_OF_ENTRIES is too high."
#endif

#include "ESTL_Types.h"
#include "Error.h"
#include "Parameter.h"
#include "Debug.h"
#include "Scope.h"


/**
 * Define scope's states
 */
typedef enum {
  SCOPE_STOP,           //!<  scope stopped
  SCOPE_ARMED,          //!<  filling the pre-trigger buffer
  SCOPE_READY,          //!<  waiting for trigger
  SCOPE_TRIGGERED,      //!<  filling the post-trigger buffer
  SCOPE_COMPLETE,       //!<  scope completed
} scope_state_t;


/**
 * Scope's local static data structure containing
 * settings and scope buffer.
 */
struct {
  scope_state_t  state;
  int8_t         trigger_channel;
  uint8_t        pre_trigger;
  uint16_t       pre_trigger_samples;
  uint16_t       post_trigger_samples;
  uint16_t       buffer_index;
  uint16_t       sample_div;
  int32_t        trigger_level;
  scope_sample_t buffer[ESTL_SCOPE_NR_OF_SAMPLES];
} Scope_data;


error_code_t Scope_CmdParameterFunction(parameter_function_t parameter_function, int32_t * cmd)
{
  if (parameter_function == PARAMETER_READ)
    *cmd = (int32_t)Scope_data.state;
  if (parameter_function == PARAMETER_WRITE)
  {
    if (*cmd < 0)
      return BELOW_LIMIT;
    if (*cmd > 1)
      return ABOVE_LIMIT;

    if( (SCOPE_COMPLETE == Scope_data.state) || (SCOPE_STOP == Scope_data.state) )
    {
      if( *cmd == 1 )
      {
        Scope_data.pre_trigger_samples = (Scope_data.pre_trigger * ESTL_SCOPE_NR_OF_SAMPLES) / 100;
        Scope_data.post_trigger_samples = ESTL_SCOPE_NR_OF_SAMPLES - Scope_data.pre_trigger_samples;
        Scope_data.state = SCOPE_ARMED;
      }
//      else
//        return SCOPE_NOT_ACCESSIBLE;
    }
    else
    {
      if( *cmd == 0 )
        Scope_data.state = SCOPE_STOP;
      else
        return SCOPE_IS_RUNNING;
    }

  }
  return OK;
}


error_code_t Scope_SetupParameterFunction(parameter_function_t parameter_function, int32_t * param)
{
  if( PARAMETER_WRITE == parameter_function )
  {
    if( (SCOPE_COMPLETE == Scope_data.state) || (SCOPE_STOP == Scope_data.state) )
    {
      Scope_data.sample_div      = (uint16_t)Parameter_GetValue(PARAM_S_DIV) - 1;
      Scope_data.pre_trigger     = (uint8_t)Parameter_GetValue(PARAM_S_PRE);
      Scope_data.trigger_channel = (uint8_t)Parameter_GetValue(PARAM_S_TRIGC);
      Scope_data.trigger_level   = Parameter_GetValue(PARAM_S_TRIGL);
    }
    else
      return SCOPE_IS_RUNNING;
  }
  return OK;
}


void Scope_Task(void)
{
  static uint16_t sample_div = 0;
  uint8_t i;

  if( (SCOPE_STOP != Scope_data.state) && (SCOPE_COMPLETE != Scope_data.state) )
  {
    // check sample divider
    if( sample_div )
      sample_div --;
    else
    {
      sample_div = Scope_data.sample_div;

      // record the samples
      if( Scope_data.buffer_index >= ESTL_SCOPE_NR_OF_SAMPLES )
        Scope_data.buffer_index = 0;
      for(i = 0; i < ESTL_DEBUG_NR_OF_ENTRIES; i ++)
        Scope_data.buffer[Scope_data.buffer_index].channel[i] = Debug_GetValue(i);

      // update the trigger state
      if( SCOPE_ARMED == Scope_data.state )
      {
        // fill the buffer with pre-trigger content
        if( Scope_data.pre_trigger_samples )
          Scope_data.pre_trigger_samples --;
        else
          Scope_data.state = SCOPE_READY;
      }
      if( SCOPE_READY == Scope_data.state )
      {
        // wait for trigger
        if( Scope_data.trigger_channel == 0 )
        {
          // no edge -- trigger immediately
          Scope_data.state = SCOPE_TRIGGERED;
        }
        else if( Scope_data.trigger_channel < 0 )
        {
          // falling edge
          if( Scope_data.buffer[Scope_data.buffer_index].channel[-Scope_data.trigger_channel-1] <= Scope_data.trigger_level )
            Scope_data.state = SCOPE_TRIGGERED;
        }
        else
        {
          // rising edge
          if( Scope_data.buffer[Scope_data.buffer_index].channel[Scope_data.trigger_channel-1] >= Scope_data.trigger_level )
            Scope_data.state = SCOPE_TRIGGERED;
        }
      }
      if( SCOPE_TRIGGERED == Scope_data.state )
      {
        // fill the buffer with post-trigger content
        if( Scope_data.post_trigger_samples )
          Scope_data.post_trigger_samples --;
        else
          Scope_data.state = SCOPE_COMPLETE;
      }

      // increment the buffer index
      Scope_data.buffer_index ++;
    }
  }
  else
    sample_div = 0;
}


scope_sample_t * Scope_GetSample(uint16_t index)
{
  if( index >= ESTL_SCOPE_NR_OF_SAMPLES )
    index = 0;

  index += Scope_data.buffer_index;
  if( index >= ESTL_SCOPE_NR_OF_SAMPLES)
    index -= ESTL_SCOPE_NR_OF_SAMPLES;

  return &(Scope_data.buffer[index]);
}


#endif // ESTL_ENABLE_SCOPE && ESTL_ENABLE_DEBUG