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
 * @file Terminal.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */


#ifndef __TERMINAL_H__
#define __TERMINAL_H__


/**
 * @ingroup ESTL
 * @defgroup TERMINAL  Terminal interface
 * @brief Terminal Module
 *
 * This module features a terminal-like user interface.
 * By providing input/output functions the terminal could be connected
 * to any stream device like UART or USB.
 * Depending on ESTL configuration the terminal allows access to parameter,
 * scope and via CANopen connected device's remote parameter.
 * @{
 */


/**
 * Define a terminal structure that holds function pointers to
 * transmit and receive functions.
 */
typedef struct {
  putcf   transmitFunction;                   //!< Terminal's character transmit function
  int8_t  (* const ReceivedNewLine)(char**);  //!< Terminal's line receive function
} terminal_t;


/**
 * Initialize the terminal with an array containing transmit and receive functions
 * for every input/output stream that should be connected to terminal.
 *
 * @param     terminals            Array containing the transmit/receive functions.
 * @param     number_of_terminals  number of provided transmit/receive functions.
 */
void Terminal_Init( const terminal_t * terminals, uint8_t number_of_terminals );


/**
 * The terminal task handles user's input processes the data and provides
 * information for the user.
 * This function might be time consuming ant therefore it should be called
 * in some kind of idle loop to avoid blocking of higher prioritized tasks.
 */
void Terminal_Task(void);


/**
 * @} end of TERMINAL
 */

#endif // __TERMINAL_H__
