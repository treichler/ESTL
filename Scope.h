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
 * @file Scope.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __SCOPE_H__
#define __SCOPE_H__

#include "ESTL_Defines.h"
#include "ESTL_Config.h"


/**
 * @ingroup  DEBUG
 * @defgroup SCOPE  Scope
 * @brief    Scope module
 *
 * This module extends the debug module with scope functionality.
 * The Scope module needs to be initialized with a data print callback function.
 * This function is called during data acquisition every time a new sample is
 * ready, respectively during readout of scope's buffer.
 * The data print function has to return a boolean value that indicates if data
 * was successfully handled or not.
 * A code example of how this function could look like is shown below:
 *
 * @code
 *   static bool_t         scope_has_new_sample;
 *   static scope_sample_t scope_sample;
 *   static uint16_t       scope_sample_index
 *
 *   bool_t PrintScope( uint16_t index, scope_sample_t * scope_sample )
 *   {
 *     if( scope_has_new_sample )
 *       return FALSE;  // print could not be handled since a previous request is pending
 *     scope_sample         = scope_sample;
 *     scope_sample_index   = index;
 *     scope_has_new_sample = TRUE;
 *     return TRUE;     // print function was called successfully
 *   }
 * @endcode
 *
 * Finally initialize scope with your print function:
 *
 * @code
 *   Scope_Init( PrintScope );
 * @endcode
 * @{
 */

#ifndef ESTL_SCOPE_NR_OF_SAMPLES
  #error "ESTL_SCOPE_NR_OF_SAMPLES needs to be set in ESTL_Config.h"
#endif


/**
 * Define the scope's sample-point data-type,
 * containing memory for each debug channel.
 */
typedef struct _scope_sample_t_ {
  int32_t channel[ESTL_DEBUG_NR_OF_ENTRIES];    //!<  Provide memory for each scope channel channel
} scope_sample_t;


/**
 * Initialize the scope respectively DAQ with a data print function.
 * The print function is expected to take to arguments, where the first
 * variable holds the scope sample's index and the second one is a the
 * pointer to the scope sample to be printed.
 *
 * @param  PrintFunction
 */
void Scope_Init( bool_t (* PrintFunction)(uint16_t, scope_sample_t*) );


/**
 * Scope's command parameter function.
 *
 * @param[in]  parameter_function  Context in which this function is called
 * @param      cmd                 On write this is the command to be issued on scope,
 *                                 on read it contains the scope's current state.
 * @return                         Error code depending if command was successful
 */
error_code_t Scope_CmdParameterFunction(function_call_t function_call, int32_t * cmd);


/**
 * This is a common parameter function for several scope setting.
 * It needs to be called on accessing related parameters:
 *   - sample-divider:  Every nth sample is saved in scope's buffer.
 *   - pre-trigger:     The amount samples 0..100% of scope buffer
 *                      to be saved before triggering.
 *   - trigger channel: Select the channel -m..0..m which should be used
 *                      as trigger source, where the sign represents the
 *                      trigger-edge and zero selects no dedicated channel,
 *                      but starts the scope immediately.
 *                      m is equivalent to ESTL_DEBUG_NR_OF_ENTRIES.
 *   - trigger level:   Trigger level.
 *
 * @param[in]  parameter_function  Context in which this function is called.
 * @param      param               Has no function.
 * @return                         Error code depending if command was successful.
 *   @retval   SCOPE_IS_BUSY       Setting is prohibited as long as scope is busy.
 *   @retval   OK                  In every other case.
 */
error_code_t Scope_SetupParameterFunction(function_call_t function_call, int32_t * param);


/**
 * The scope task needs to be called periodically with a sample rate that fits
 * to application's data processing.
 */
void Scope_Task(void);


/**
 * Get one sampling point from scope's buffer.
 *
 * @param[in]  index  The index of the scope buffer's sample, it needs to be within the range of
 *                    ESTL_SCOPE_NR_OF_SAMPLES.
 * @return            A pointer to the desired sampling point containing a probe of all debug
 *                    channels. If index does not exist zero is returned instead.
 */
scope_sample_t * Scope_GetSample(uint16_t index);


/**
 * @} end of SCOPE
 */

#endif // __SCOPE_H__
