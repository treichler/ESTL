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
 * @file Uart.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __UART_H__
#define __UART_H__


/**
 * @ingroup  ESTL
 * @defgroup UART  Uart
 * @brief UART Module
 *
 * This module implements a simple UART receive buffer.
 * Every time the UART periphery receives a valid character the function
 * Uart_ReceiveChar() has to be called. Below an example shows how this
 * could be achieved with an interrupt service routine:
 *
 * @code
 *   void USART1_IRQHandler(void)
 *   {
 *     NVIC_ClearPendingIRQ(USART1_IRQn);
 *     // check if interrupt was caused by RXNE
 *     if ((USART1->CR1 & USART_CR1_RXNEIE) && (USART1->SR & USART_SR_RXNE))
 *       Uart_ReceiveChar((char)USART1->DR);
 *   }
 * @endcode
 *
 * @{
 */

/**
 * This function features a receive buffer for UART.
 * It needs to be called every time the UART periphery received a character.
 *
 * @param     c                           UART's received character
 */
void Uart_ReceiveChar(char c);


/**
 * Get the receive buffer's address and return if buffer contains a new received line
 * since last call.
 *
 * @param     rx_buffer                   Pointer to receive buffer's address
 * @return                                New line received?
 *   @retval  FALSE                       No new line.
 *   @retval  TRUE                        New line available in buffer.
 */
bool_t Uart_NewLineReceived(char ** rx_buffer);


/**
 * @} end of UART
 */

#endif // __UART_H__
