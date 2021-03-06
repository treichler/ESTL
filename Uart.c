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
 * @file Uart.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Defines.h"
#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include "Error.h"
#include "Uart.h"
#include "Target.h"
#include "Parameter.h"

/**
 * Terminal UART receive buffer size
 */
#define RECEIVE_BUFFER_SIZE (32)


/**
 * This data structure contains the receive buffer and some local static variables.
 */
struct {
  char          receive_buffer[RECEIVE_BUFFER_SIZE];
  uint16_t      receive_buffer_index;
  bool_t        line_received;
} Uart_data;


void Uart_ReceiveChar(char c)
{
  if( Uart_data.line_received == FALSE )
  {
    Uart_data.receive_buffer[Uart_data.receive_buffer_index] = c;
#if( ESTL_TERMINAL_LINE_BREAK == ESTL_TERMINAL_LINE_BREAK_LF )
    if( (Uart_data.receive_buffer_index > 0) && (Uart_data.receive_buffer[Uart_data.receive_buffer_index] == '\n'))
    {
      Uart_data.receive_buffer[Uart_data.receive_buffer_index] = '\0';
      Uart_data.receive_buffer_index = 0;
      Uart_data.line_received = TRUE;
    }
#elif( ESTL_TERMINAL_LINE_BREAK == ESTL_TERMINAL_LINE_BREAK_CR )
    if( (Uart_data.receive_buffer_index > 0) && (Uart_data.receive_buffer[Uart_data.receive_buffer_index] == '\r'))
    {
      Uart_data.receive_buffer[Uart_data.receive_buffer_index] = '\0';
      Uart_data.receive_buffer_index = 0;
      Uart_data.line_received = TRUE;
    }
#elif( ESTL_TERMINAL_LINE_BREAK == ESTL_TERMINAL_LINE_BREAK_CRLF )
    if( (Uart_data.receive_buffer_index > 0) &&
          (Uart_data.receive_buffer[Uart_data.receive_buffer_index - 1] == '\r') &&
          (Uart_data.receive_buffer[Uart_data.receive_buffer_index] == '\n'))
    {
      Uart_data.receive_buffer[Uart_data.receive_buffer_index - 1] = '\0';
      Uart_data.receive_buffer_index = 0;
      Uart_data.line_received = TRUE;
    }
#else
#error "ESTL_TERMINAL_LINE_BREAK not defined or invalid"
#endif
    else
    {
      Uart_data.receive_buffer_index ++;
      if (Uart_data.receive_buffer_index >= RECEIVE_BUFFER_SIZE)
        Uart_data.receive_buffer_index = 0;
    }
  }
}


bool_t Uart_NewLineReceived(char ** rx_buffer)
{
  *rx_buffer = Uart_data.receive_buffer;
  if( Uart_data.line_received )
  {
    Uart_data.line_received = FALSE;
    return TRUE;
  }
  return FALSE;
}
