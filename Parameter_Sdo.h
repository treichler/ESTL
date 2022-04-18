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
 * @file Parameter_CANopen.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __PARAMETER_CANOPEN_H__
#define __PARAMETER_CANOPEN_H__


/**
 * @ingroup  PARAMETER
 * @defgroup PARAMETER_CAN_OPEN  Parameter_CANopen
 * @brief Parameter CANopen module
 *
 * The Parameter CANopen module extents the Parameter module to grant access
 * to parameter via CANopen. To be compatible with CANopen all parameter
 * related functionality is mapped into manufacturer specific profile area,
 * located in SDO index range 0x2000..0x5FFF.
 */

error_code_t Parameter_CANopen_ReadTableIndexRange( uint8_t node_id, range_t * parameter_index_range );
error_code_t Parameter_CANopen_ReadTableCrc( uint8_t node_id, uint32_t * crc );
error_code_t Parameter_CANopen_ReadName( uint8_t node_id, int16_t parameter_index, char * name, uint16_t len );
error_code_t Parameter_CANopen_ReadInfo( uint8_t node_id, int16_t parameter_index, char * info, uint16_t len );
error_code_t Parameter_CANopen_ReadValue( uint8_t node_id, int16_t parameter_index, int32_t * value );
error_code_t Parameter_CANopen_WriteValue( uint8_t node_id, int16_t parameter_index, int32_t value );
error_code_t Parameter_CANopen_ReadTableEntry( uint8_t node_id, int16_t parameter_index, parameter_data_t * parameter_data );

uint8_t Parameter_CANopen_CallbackSdoReq( uint8_t length_req, uint8_t *req_ptr, uint8_t *length_resp, uint8_t *resp_ptr );


/**
 * @} end of PARAMETER_CAN_OPEN
 */

#endif // __PARAMETER_CANOPEN_H__
