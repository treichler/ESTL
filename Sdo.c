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
 * @file Sdo.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include "Sdo.h"


#ifndef ESTL_TERMINAL_REMOTE_PARAMETER_CON_TIMEOUT
#define ESTL_TERMINAL_REMOTE_PARAMETER_CON_TIMEOUT      (1000)  //!< default connection timeout [ms]
#warning "SDO_TIMEOUT is set to default value"
#endif




typedef enum {
  SDO_REQ_FAIL,
  SDO_REQ_SUCCESS,
  SDO_REQ_EXP_READ_BUSY,
  SDO_REQ_EXP_WRITE_BUSY,
  SDO_REQ_SEG_READ_BUSY,
  SDO_REQ_SEG_WRITE_BUSY,
} sdo_req_state_t;


struct {
  uint32_t         resp_in_buffer;
  uint8_t          * resp_buffer;
  uint8_t          * exp_valid_bytes;
  uint32_t         seg_buffer_size;
  uint32_t         abort_code;
  uint8_t          seg_id;
  sdo_req_state_t  req_state;
  uint16_t         timeout;
  uint8_t          * req_data;                          //!<  Pointer to SDO-request's message object 8-byte data buffer
  bool_t           (* SdoRequestFunction)(uint8_t);     //!<  SDO request function
  bool_t           (* SdoIsAvailableFunction)(void);    //!<  SDO request is busy function
  bool_t           is_initialized;
  uint8_t          nr_of_nodes;
} Sdo_data;


bool_t Sdo_Init( uint8_t * req_data, bool_t (* SdoRequestFunction)(uint8_t), bool_t (* SdoIsAvailableFunction)(void), uint8_t nr_of_nodes )
{
  if( req_data && SdoRequestFunction && SdoIsAvailableFunction )
  {
    Sdo_data.req_data = req_data;
    Sdo_data.SdoRequestFunction = SdoRequestFunction;
    Sdo_data.SdoIsAvailableFunction = SdoIsAvailableFunction;
    Sdo_data.nr_of_nodes = nr_of_nodes;
    Sdo_data.is_initialized = TRUE;
  }
  else
    Sdo_data.is_initialized = FALSE;
  return Sdo_data.is_initialized;
}


void Sdo_1msTask(void)
{
  // timeout for SDO client functions
  if( Sdo_ReqIsBusy() && Sdo_data.timeout )
  {
    Sdo_data.timeout --;
    if( 0 == Sdo_data.timeout )
      Sdo_data.req_state = SDO_REQ_FAIL;           // change status to fail on timeout
  }
}


bool_t Sdo_ReqIsBusy( void )
{
  return ! ((SDO_REQ_SUCCESS == Sdo_data.req_state) || (SDO_REQ_FAIL == Sdo_data.req_state));
}


bool_t Sdo_ReqIsFinished( void )
{
  return SDO_REQ_SUCCESS == Sdo_data.req_state;
}


uint32_t Sdo_GetAbortCode( void )
{
  return Sdo_data.abort_code;
}


uint8_t Sdo_GetNrOfNodes( void )
{
  return Sdo_data.nr_of_nodes;
}


void Sdo_SetNrOfNodes( uint8_t nr_of_nodes )
{
  Sdo_data.nr_of_nodes = nr_of_nodes;
}


bool_t Sdo_ExpRead( uint8_t node_id, uint16_t index, uint8_t subindex, int32_t* data, uint8_t* valid_data_bytes )
{
  if( Sdo_data.is_initialized && Sdo_data.SdoIsAvailableFunction() && \
      ((SDO_REQ_SUCCESS == Sdo_data.req_state) || (SDO_REQ_FAIL == Sdo_data.req_state)) )
  {
    // only initiate when no SDO client transfer is ongoing
    Sdo_data.req_state = SDO_REQ_EXP_READ_BUSY;
    Sdo_data.resp_in_buffer = 0;
    Sdo_data.resp_buffer = (uint8_t*)data;
    Sdo_data.exp_valid_bytes = valid_data_bytes;
    Sdo_data.req_data[0] = 0x02<<5;
    Sdo_data.req_data[1] = index & 0xFF;
    Sdo_data.req_data[2] = (index >> 8) & 0xFF;
    Sdo_data.req_data[3] = subindex;
    Sdo_data.req_data[4] = 0x00;
    Sdo_data.req_data[5] = 0x00;
    Sdo_data.req_data[6] = 0x00;
    Sdo_data.req_data[7] = 0x00;

    Sdo_data.timeout = ESTL_TERMINAL_REMOTE_PARAMETER_CON_TIMEOUT;
    return Sdo_data.SdoRequestFunction( node_id );
  }
  return FALSE;
}


bool_t Sdo_ExpWrite( uint8_t node_id, uint16_t index, uint8_t subindex, int32_t data, uint8_t length )
{
  if( Sdo_data.is_initialized && Sdo_data.SdoIsAvailableFunction() && \
      ((SDO_REQ_SUCCESS == Sdo_data.req_state) || (SDO_REQ_FAIL == Sdo_data.req_state)) )
  {
    // only initiate when no SDO client transfer is ongoing
    Sdo_data.req_state = SDO_REQ_EXP_WRITE_BUSY;
    if( (0 == length) || (4 < length) )
    {
      Sdo_data.req_state = SDO_REQ_FAIL;                // min for exp write is 0, max is 4
      return FALSE;
    }

    Sdo_data.req_data[0] = 1<<5 | (4-length)<<2 | 0x02 | 0x01;
    Sdo_data.req_data[1] = index & 0xFF;
    Sdo_data.req_data[2] = (index >> 8) & 0xFF;
    Sdo_data.req_data[3] = subindex;
    Sdo_data.req_data[4] = (uint8_t)(data);
    Sdo_data.req_data[5] = (uint8_t)(data >> 8);
    Sdo_data.req_data[6] = (uint8_t)(data >> 16);
    Sdo_data.req_data[7] = (uint8_t)(data >> 24);

    Sdo_data.timeout = ESTL_TERMINAL_REMOTE_PARAMETER_CON_TIMEOUT;
    return Sdo_data.SdoRequestFunction( node_id );
  }
  return FALSE;
}


bool_t Sdo_SegRead( uint8_t node_id, uint16_t index, uint8_t subindex, void* buff, uint32_t buff_size )
{
  if( Sdo_data.is_initialized && Sdo_data.SdoIsAvailableFunction() && \
      ((SDO_REQ_SUCCESS == Sdo_data.req_state) || (SDO_REQ_FAIL == Sdo_data.req_state)) )
  {
    Sdo_data.req_state       = SDO_REQ_SEG_READ_BUSY;
    Sdo_data.resp_buffer     = buff;
    Sdo_data.seg_buffer_size = buff_size;
    Sdo_data.seg_id          = node_id;

    Sdo_data.req_data[0] = 0x40;
    Sdo_data.req_data[1] = index & 0xFF;
    Sdo_data.req_data[2] = (index >> 8) & 0xFF;
    Sdo_data.req_data[3] = subindex;
    Sdo_data.req_data[4] = 0x00;
    Sdo_data.req_data[5] = 0x00;
    Sdo_data.req_data[6] = 0x00;
    Sdo_data.req_data[7] = 0x00;

    Sdo_data.timeout = ESTL_TERMINAL_REMOTE_PARAMETER_CON_TIMEOUT;
    return Sdo_data.SdoRequestFunction( node_id );
  }
  return FALSE;
}


bool_t Sdo_RxHandler( uint8_t * rx, uint8_t rx_id, uint8_t * resp, uint8_t * resp_id )
{
  bool_t is_resp = FALSE;

  // TODO eventually operate only on dedicated remote IDs
  //      to avoid collision between other SDO-talking nodes?
  //      Or optionally restrict master to one node?
  uint8_t i;
  if( rx[0] == 0x80 )
  {
//    CANopen_Data.sdoc_state = CANopen_SDOC_Fail;
    Sdo_data.req_state = SDO_REQ_FAIL;
    // TODO
    Sdo_data.abort_code = ((uint32_t)rx[7] << 24) | ((uint32_t)rx[6] << 16) | ((uint32_t)rx[5] << 8)  | (uint32_t)rx[4];
  }
  // message object used for SDO client
//  if(CANopen_Data.sdoc_state == CANopen_SDOC_Exp_Read_Busy)
  if( Sdo_data.req_state == SDO_REQ_EXP_READ_BUSY )
  {
    // Expedited read was initiated
    if( (rx[0] & (7<<5)) == 0x40 )
    {
      // received data from server
      i = 4 - ( (rx[0]>>2) & 0x03 );    // i now contains number of valid data bytes
      Sdo_data.resp_in_buffer = i;

      while( i-- )
        Sdo_data.resp_buffer[i] = rx[i + 4];      // save valid databytes to memory

      Sdo_data.req_state = SDO_REQ_SUCCESS;                                       // expedited read completed successfully
      if( Sdo_data.exp_valid_bytes )
        *Sdo_data.exp_valid_bytes = Sdo_data.resp_in_buffer;                         // save number of valid bytes
    }
  }
  else if( Sdo_data.req_state == SDO_REQ_EXP_WRITE_BUSY )
  {
    // expedited write was initiated
    if( rx[0] == 0x60 )
      Sdo_data.req_state = SDO_REQ_SUCCESS;                                       // received confirmation
  }
  else if(Sdo_data.req_state == SDO_REQ_SEG_READ_BUSY)
  {
    // segmented read was initiated
    if( ((rx[0] & (7<<5)) == 0x40) && ((rx[0] & (1<<1)) == 0x00) )
    {
      // Received reply on initiate command, send first segment request
      resp[0] = 0x60;
      resp[1] = 0x00;
      resp[2] = 0x00;
      resp[3] = 0x00;
      // TODO eventually save the length of the requested segmented read
      resp[4] = 0x00;
      resp[5] = 0x00;
      resp[6] = 0x00;
      resp[7] = 0x00;
      is_resp = TRUE;
      *resp_id = Sdo_data.seg_id;
      Sdo_data.resp_in_buffer = 0;
    }
    else if( (rx[0] & (7 << 5)) == 0x00 )
    {
      // Received response on request
      for( i = 0; i < (7 - ((rx[0] >> 1) & 0x07)); i ++ )
      {
        // get all data from frame and save it to memory
        if( Sdo_data.resp_in_buffer < Sdo_data.seg_buffer_size )
          Sdo_data.resp_buffer[Sdo_data.resp_in_buffer++] = rx[i+1];
        else
        {
          // SDO segment too big for buffer, abort
          Sdo_data.req_state = SDO_REQ_FAIL;
        }
      }

      if(rx[0] & 0x01)
      {
        // Last frame, change status to success
        Sdo_data.req_state = SDO_REQ_SUCCESS;
      }
      else
      {
        // not last frame, send acknowledge
        resp[0] = 0x60 | ((rx[0] & (1<<4)) ^ (1<<4));         // toggle
        resp[1] = 0x00;
        resp[2] = 0x00;
        resp[3] = 0x00;
        resp[4] = 0x00;
        resp[5] = 0x00;
        resp[6] = 0x00;
        resp[7] = 0x00;
        *resp_id = Sdo_data.seg_id;
        is_resp = TRUE;
      }
      // reinitialize timeout
      Sdo_data.timeout = ESTL_TERMINAL_REMOTE_PARAMETER_CON_TIMEOUT;
    }
  }
  else if( Sdo_data.req_state == SDO_REQ_SEG_WRITE_BUSY )
  {
    // segmented write was initiated
    if( (((rx[0] & (7<<5)) == 0x60) && ((rx[0] & (1<<1)) == 0x00)) || ((rx[0] & (7<<5)) == 0x20) )
    {
      // received acknowledge
      if((rx[0] & (7<<5)) == 0x60)
      {
        // first frame
        Sdo_data.resp_in_buffer = 0;             // Clear buffer
        resp[0] = 1<<4;       // initialize for toggle
      }
      resp[0] = ((rx[0] & (1<<4)) ^ (1<<4));                // toggle

      // fill frame data
      for( i = 0; i < 7; i ++ )
      {
        if( Sdo_data.resp_in_buffer < Sdo_data.seg_buffer_size )
          resp[i+1] = Sdo_data.resp_buffer[Sdo_data.resp_in_buffer++];
        else
          resp[i+1] = 0x00;
      }

      // if end of buffer has been reached, then this is the last frame
      if( Sdo_data.resp_in_buffer == Sdo_data.seg_buffer_size )
      {
        resp[0] |= ( (7-(Sdo_data.seg_buffer_size % 7)) << 1 ) | 0x01;      // save length
        Sdo_data.req_state = SDO_REQ_SUCCESS;                                             // set state to success
      }

      *resp_id = Sdo_data.seg_id;
      is_resp = TRUE;
    }
  }
  return is_resp;
}

/*
void CANopen_RxSdoHandler( CAN_MSG_OBJ_t * CANopen_Msg_Obj )
{
  uint8_t id;
  if( Sdo_RxHandler( CANopen_Msg_Obj->data, (CANopen_Msg_Obj->mode_id & 0x7F), CANopen_Msg_Obj->data, &id ) )
  {
    CANopen_Msg_Obj->msgobj = 8;
    CANopen_Msg_Obj->mode_id = 0x600 + id;
    CANopen_Msg_Obj->dlc = 8;
    Target_CanSendData( CANopen_Msg_Obj );
  }
}

*/
