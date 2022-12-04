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
 * @file Sdo.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __SDO_H__
#define __SDO_H__


/**
 * @ingroup  ESTL
 * @defgroup SDO  Sdo
 * @brief    Service Data Object module
 *
 * SDO features a light weight implementation for preparation and handling of
 * service data objects similar to those which are used within the CANopen
 * protocol standard.
 * It is built up in a client-server like topology granting access to 65536
 * indices and 256 sub-indices.
 * Small data in the range of 1..4 bytes are accessed via an expedited call and
 * larger data like strings is accessed via segmented calls.
 * SDO is intended to be used coexisting with packet oriented bus protocols
 * like CAN or LIN.
 * Therefore it is expected that the underlying bus protocol provides an
 * identifier and a data-block consisting of at least 8 bytes.
 *
 * To get it up and running the SDO module requires access to a static 8-byte
 * data array which could be part of a message object, a getter function which
 * indicates that bus' resource for SDO request is available and a send
 * function.
 * A code example of how this might look like is shown below:
 *
 * @code
 *   // example of a CAN message object containing a 8-byte data array
 *   typedef struct CCAN_MSG_OBJ {
 *     uint32_t  mode_id;
 *     uint32_t  mask;
 *     uint8_t   data[8];
 *     uint8_t   dlc;
 *     uint8_t   msgobj;
 *   } CAN_MSG_OBJ_t;
 *
 *   // Static CAN message object for SDO request
 *   static CAN_MSG_OBJ_t     sdo_req_msg_obj;
 *
 *   // CAN SDO check if available function
 *   // returns TRUE if SDO send resource is available
 *   bool_t CanSdoIsAvailable( void )
 *   {
 *     // At this point it should be checked if the previous SDO request
 *     // is already sent or if it is still in a queue.
 *     return TRUE;
 *   }
 *
 *   // CAN SDO send function
 *   // returns TRUE on success
 *   bool_t CanSdoRequest( uint8_t node_id )
 *   {
 *     sdo_req_msg_obj.mode_id = 0x600 + node_id;
 *     sdo_req_msg_obj.mask = 0x00;
 *     sdo_req_msg_obj.dlc = 8;
 *     sdo_req_msg_obj.msgobj = 8;
 *     return CanSendData( &sdo_req_msg_obj );
 *   }
 * @endcode
 *
 * Finally the prepared functions and the data array have to be connected to
 * the SDO module by calling Sdo_Init():
 *
 * @code
 *   Sdo_Init( sdo_req_msg_obj.data, CanSdoRequest, CanSdoIsAvailable, 128 );
 * @endcode
 *
 * Keep in mind when remote access to the Parameter module is activated it maps
 * its functionality into the manufacturer specific profile area, located in
 * SDO index range 0x2000..0x5FFF.
 * So do not use this indices to avoid collisions and odd behavior.
 * @{
 */


/**
 * Initialize SDO request data.
 * SDO expects that Sdo_Init() caller provides a static request data buffer
 * which consists of an 8-byte array. It is also expected that this particular
 * array is exclusively used for SDO requests only.
 *
 * @param req_data                      Pointer to 8-byte array
 * @param SdoRequestFunction            Target dependent SDO-request function
 * @param SdoIsAvailableFunction        Target dependent SDO-is-available function
 * @param nr_of_nodes                   TODO to be checked if really necessary
 * @return                              Success of initialization
 *   @retval    FALSE                   SDO could not be initialized
 *   @retval    TRUE                    SDO was successfully initialized
 */
bool_t Sdo_Init( uint8_t * req_data, bool_t (* SdoRequestFunction)(uint8_t), bool_t (* SdoIsAvailableFunction)(void), uint8_t nr_of_nodes );


/**
 * SDO's real-time task needs to be called periodically every millisecond.
 * to handle request timeouts.
 */
void Sdo_1msTask(void);


/**
 * Check if any SDO request is currently ongoing.
 *
 * @return              Busy status of SDO request
 *   @retval    FALSE   SDO is free for new request
 *   @retval    TRUE    Any SDO request is ongoing
 */
bool_t Sdo_ReqIsBusy( void );


/**
 * Get finish status of SDO request
 *
 * @return
 *   @retval    OK                      SDO request finished successfully
 *   @retval    TIMEOUT                 SDO request timed out
 *   @retval    SDO_CONNECTION_FAILED   SDO request failed
 *   @retval    RESOURCE_BUSY           SDO request is still ongoing
 */
error_code_t Sdo_ReqFinishStatus( void );


/**
 * Get the CANopen SDO abort code in case of a failed SDO request.
 * @return      CANopen SDO abort code
 */
uint32_t Sdo_GetAbortCode( void );


uint8_t Sdo_GetNrOfNodes( void );
void Sdo_SetNrOfNodes( uint8_t nr_of_nodes );


/**
 * This Function requests an expedited SDO read.
 * It is required that SDO module is initialized according to Sdo_Init() and
 * that there is currently no request ongoing.
 *
 * @param node_id               CANopen node ID
 * @param index                 SDO index
 * @param subindex              SDO sub-index
 * @param data                  Pointer to 32bit variable where data has to be read
 * @param valid_data_bytes      Pointer to 8bit variable that will contain data length after successful read [1..4]
 * @return                      Success of read request
 *   @retval    FALSE           Read request was not initiated
 *   @retval    TRUE            Read request was successful
 */
bool_t Sdo_ExpRead( uint8_t node_id, uint16_t index, uint8_t subindex, int32_t* data, uint8_t* valid_data_bytes );


/**
 * This Function issues an expedited SDO write.
 * It is required that SDO module is initialized according to Sdo_Init() and
 * that there is currently no request ongoing.
 *
 * @param node_id               CANopen node ID
 * @param index                 SDO index
 * @param subindex              SDO sub-index
 * @param data                  Data to be written
 * @param length                Length of provided data [1..4]
 * @return                      Success of write request
 *   @retval    FALSE           Read request was not initiated
 *   @retval    TRUE            Read request was successful
 */
bool_t Sdo_ExpWrite( uint8_t node_id, uint16_t index, uint8_t subindex, int32_t data, uint8_t length );


/**
 * This Function requests a segmented SDO read.
 * It is required that SDO module is initialized according to Sdo_Init() and
 * that there is currently no request ongoing.
 *
 * @param node_id               CANopen node ID
 * @param index                 SDO index
 * @param subindex              SDO sub-index
 * @param buff                  Pointer to a buffer where segmented data has to be read
 * @param buff_size             Size of provided buffer
 * @return                      Success of read request
 *   @retval    FALSE           Read request was not initiated
 *   @retval    TRUE            Read request was successful
 */
bool_t Sdo_SegRead( uint8_t node_id, uint16_t index, uint8_t subindex, void* buff, uint32_t buff_size );


/**
 * Evaluate received SDO responses.
 * This Function needs to be called as soon as any SDO response message was received.
 *
 * @param rx            Pointer to 8-byte SDO response buffer
 * @param rx_id         CANopen node ID where the response comes from
 * @param resp          Pointer to 8-byte buffer for the next SDO request
 * @param resp_id       CANopen node ID where the next request has to be transmitted to
 * @return              Necessity for next request
 *   @retval    FALSE   Evaluated response finished ongoing request
 *   @retval    TRUE    Evaluated response requires next request
 */
bool_t Sdo_RxHandler( uint8_t * rx, uint8_t rx_id, uint8_t * resp, uint8_t * resp_id );


/**
 * @} end of SDO
 */

#endif // __SDO_H__

