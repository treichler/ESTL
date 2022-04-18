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
 * @file Parameter_Sdo.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __PARAMETER_SDO_H__
#define __PARAMETER_SDO_H__


/**
 * @ingroup  PARAMETER
 * @defgroup PARAMETER_SDO  Parameter_Sdo
 * @brief CANopen compatible Parameter SDO module
 *
 * The Parameter SDO module extents the Parameter module to grant access
 * to parameter via an CANopen compatible SDO interface.
 * Therefore all parameter related functionality is mapped into manufacturer
 * specific profile area, located in SDO index range 0x2000..0x5FFF.
 */


/**
 * Read a remote node's parameter index range.
 *
 * @param node_id                       ID of node to be requested.
 * @param parameter_index_range         Pointer to range variable where data is read to.
 * @return                              Success status of SDO read request.
 *   @retval OK                         Data successfully read.
 *   @retval SDO_CONNECTION_FAILED      Data could not be read due to SDO connection failure.
 */
error_code_t ParameterSdo_ReadTableIndexRange( uint8_t node_id, range_t * parameter_index_range );


/**
 * Read a remote node's parameter table CRC.
 *
 * @param node_id                       ID of node to be requested.
 * @param crc                           Pointer to variable where CRC is read to.
 * @return                              Success status of SDO read request.
 *   @retval OK                         Data successfully read.
 *   @retval SDO_CONNECTION_FAILED      Data could not be read due to SDO connection failure.
 */
error_code_t ParameterSdo_ReadTableCrc( uint8_t node_id, uint32_t * crc );


/**
 * Read a remote node's dedicated parameter name.
 * It is expected, that the provided buffer is big enough to hold the parameter's
 * name, otherwise function call will abort and return an error code.
 *
 * @param node_id                       ID of node to be requested.
 * @param parameter_index               Index of the parameter which name has to be read.
 * @param name                          Pointer to buffer where the name is read to.
 * @param len                           Length of the provided buffer.
 * @return                              Success status of SDO read request.
 *   @retval OK                         Data successfully read.
 *   @retval SDO_CONNECTION_FAILED      Data could not be read due to SDO connection failure.
 */
error_code_t ParameterSdo_ReadName( uint8_t node_id, int16_t parameter_index, char * name, uint16_t len );


/**
 * Read a remote node's dedicated parameter info respectively help text.
 * It is expected, that the provided buffer is big enough to hold the parameter's
 * info, otherwise function call will abort and return an error code.
 *
 * @param node_id                       ID of node to be requested.
 * @param parameter_index               Index of the parameter which info has to be read.
 * @param info                          Pointer to buffer where the info text is read to.
 * @param len                           Length of the provided buffer.
 * @return                              Success status of SDO read request.
 *   @retval OK                         Data successfully read.
 *   @retval SDO_CONNECTION_FAILED      Data could not be read due to SDO connection failure.
 */
error_code_t ParameterSdo_ReadInfo( uint8_t node_id, int16_t parameter_index, char * info, uint16_t len );


/**
 * Read a remote node's dedicated parameter value.
 *
 * @param node_id                       ID of node to be requested.
 * @param parameter_index               Index of the parameter which value has to be read.
 * @param value                         Pointer to variable where value is read to.
 * @return                              Success status of SDO read request.
 *   @retval OK                         Data successfully read.
 *   @retval SDO_CONNECTION_FAILED      Data could not be read due to SDO connection failure.
 */
error_code_t ParameterSdo_ReadValue( uint8_t node_id, int16_t parameter_index, int32_t * value );


/**
 * Write a remote node's dedicated parameter value.
 *
 * @param node_id                       ID of node to be requested.
 * @param parameter_index               Index of the parameter which value has to be written.
 * @param value                         Value to be written to the parameter.
 * @return                              Success status of SDO write request.
 *   @retval OK                         Data successfully written.
 *   @retval SDO_CONNECTION_FAILED      Data could not be written due to SDO connection failure.
 */
error_code_t ParameterSdo_WriteValue( uint8_t node_id, int16_t parameter_index, int32_t value );


/**
 * Read a remote node's dedicated parameter data.
 * This data contains parameter properties (unit, representation, flags),
 * nominal, minimum and maximum value.
 *
 * @param node_id                       ID of node to be requested.
 * @param parameter_index               Index of the parameter which info has to be read.
 * @param parameter_data                Pointer to parameter data structure where data is read to.
 * @return                              Success status of SDO read request.
 *   @retval OK                         Data successfully read.
 *   @retval SDO_CONNECTION_FAILED      Data could not be read due to SDO connection failure.
 */
error_code_t ParameterSdo_ReadTableEntry( uint8_t node_id, int16_t parameter_index, parameter_data_t * parameter_data );


/**
 * This callback maps the parameter-set to the SDO index range 0x2000..0x5FFF.
 * This function needs to be called if any other SDO request for this
 * particular node cannot be handled.
 *
 * @param length_req    Length of SDO request.
 * @param req_ptr       Pointer to 8-byte request.
 * @param length_resp   Pointer to a variable that will get the response's length.
 * @param resp_ptr      Pointer to 8-byte response array.
 * @return
 *   @retval    0       process regularly, no impact
 *   @retval    1       processed in callback, auto-send returned message
 *   @retval    2       processed in callback, don't send response
 */
uint8_t ParameterSdo_CallbackSdoReq( uint8_t length_req, uint8_t *req_ptr, uint8_t *length_resp, uint8_t *resp_ptr );


/**
 * @} end of PARAMETER_CAN_OPEN
 */

#endif // __PARAMETER_SDO_H__
