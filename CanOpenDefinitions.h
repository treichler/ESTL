//----------------------------------------------------------------------------//
//  Copyright 2023 Clemens Treichler                                          //
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
 * @file CanOpenDefinitions.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2023 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __CAN_OPEN_DEFINITIONS_H__
#define __CAN_OPEN_DEFINITIONS_H__


/**
 * @ingroup ESTL
 * @defgroup CANOPEN  CANopen definitions
 * @brief Definitions for CANopen
 * @{
 */

#define CANOPEN_ADDR_NMT_NODE_CTL       (0x000)

#define CANOPEN_ADDR_FAILSAFE_CMD       (0x001)

#define CANOPEN_ADDR_SYNC               (0x080)

#define CANOPEN_BASE_ADDR_EMERGENCY     (0x080)

#define CANOPEN_ADDR_TIME_STAMP         (0x100)

#define CANOPEN_BASE_ADDR_PDO_1_TX      (0x180)
#define CANOPEN_BASE_ADDR_PDO_1_RX      (0x200)
#define CANOPEN_BASE_ADDR_PDO_2_TX      (0x280)
#define CANOPEN_BASE_ADDR_PDO_2_RX      (0x300)
#define CANOPEN_BASE_ADDR_PDO_3_TX      (0x380)
#define CANOPEN_BASE_ADDR_PDO_3_RX      (0x400)
#define CANOPEN_BASE_ADDR_PDO_4_TX      (0x480)
#define CANOPEN_BASE_ADDR_PDO_4_RX      (0x500)

#define CANOPEN_BASE_ADDR_SDO_TX        (0x580)
#define CANOPEN_BASE_ADDR_SDO_RX        (0x600)

#define CANOPEN_BASE_ADDR_NMT           (0x700)

/**
 * @} end of CANOPEN
 */

#endif // __CAN_OPEN_DEFINITIONS_H__
