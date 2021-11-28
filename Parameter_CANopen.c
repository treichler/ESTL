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
 * @file Parameter_CANopen.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#include <string.h>
#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include "Error.h"

//#include "Target.h"

//#include "CANopen.h"
#include "Sdo.h"
#include "Parameter.h"
#include "Parameter_CANopen.h"

// pseudo_code[31..16], mapped_error[15..0]
#define PSEUDO_SDO_ABORT_CODE (0x10100000)


enum {
  SUBIDX_PARAM_INDEX_MIN,
  SUBIDX_PARAM_INDEX_MAX,
  SUBIDX_PARAM_INDEX_RANGE,     //<!  index_min and index_max united to one single read.
  SUBIDX_PARAM_TABLE_CRC,       //<!  placeholder for some kind of check-sum over parameter table
  SUBIDX_PARAM_ACTUAL,
  SUBIDX_PARAM_NOMINAL,
  SUBIDX_PARAM_MINIMUM,
  SUBIDX_PARAM_MAXIMUM,
  SUBIDX_PARAM_UNIT,
  SUBIDX_PARAM_REPR,
  SUBIDX_PARAM_FLAGS,
  SUBIDX_PARAM_PROPERTY,        //<! Unites the three properties unit, representation and flags to one single read.
  SUBIDX_PARAM_NAME,
  SUBIDX_PARAM_INFO,
};


error_code_t Parameter_CANopen_ReadTableIndexRange( uint8_t node_id, range_t * parameter_index_range )
{
  int32_t temp;

  Sdo_ExpRead_Foo( node_id, 0x2000, SUBIDX_PARAM_INDEX_RANGE, &temp, 0 );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  parameter_index_range->max = (int16_t)(temp >> 16);
  parameter_index_range->min = (int16_t)temp;

  return OK;
}


error_code_t Parameter_CANopen_ReadTableCrc( uint8_t node_id, uint32_t * crc )
{
  Sdo_ExpRead_Foo( node_id, 0x2000, SUBIDX_PARAM_TABLE_CRC, (int32_t*)crc, 0 );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  return OK;
}


error_code_t Parameter_CANopen_ReadName( uint8_t node_id, int16_t parameter_index, char * name, uint16_t len )
{
  uint16_t index = 0x2000 + ((uint16_t)(parameter_index) >> 2);
  uint8_t sub_index = ((uint8_t)(parameter_index) << 6);

  Sdo_SegRead_Foo( node_id, index, sub_index + SUBIDX_PARAM_NAME, name, len );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  return OK;
}


error_code_t Parameter_CANopen_ReadInfo( uint8_t node_id, int16_t parameter_index, char * info, uint16_t len )
{
  uint16_t index = 0x2000 + ((uint16_t)(parameter_index) >> 2);
  uint8_t sub_index = ((uint8_t)(parameter_index) << 6);

  if( len <= 1 )
    return -1;
  Sdo_SegRead_Foo( node_id, index, sub_index + SUBIDX_PARAM_INFO, info, len-1 );
  while( Sdo_ReqIsBusy() );
  info[len-1] = '\0';
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  return OK;
}


error_code_t Parameter_CANopen_ReadValue( uint8_t node_id, int16_t parameter_index, int32_t * value )
{
  uint16_t index = 0x2000 + ((uint16_t)(parameter_index) >> 2);
  uint8_t sub_index = ((uint8_t)(parameter_index) << 6);

  Sdo_ExpRead_Foo( node_id, index, sub_index + SUBIDX_PARAM_ACTUAL, value, 0 );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  return OK;
}


error_code_t Parameter_CANopen_WriteValue( uint8_t node_id, int16_t parameter_index, int32_t value )
{
  uint16_t index = 0x2000 + ((uint16_t)(parameter_index) >> 2);
  uint8_t sub_index = ((uint8_t)(parameter_index) << 6);

  Sdo_ExpWrite_Foo( node_id, index, sub_index + SUBIDX_PARAM_ACTUAL, value, 4 );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
  {
    uint32_t sdo_abort_code = Sdo_GetAbortCode();
    if( PSEUDO_SDO_ABORT_CODE == (0xFFFF0000 & sdo_abort_code) )
      return (error_code_t)sdo_abort_code;
    else
      return CAN_SDO_CONNECTION_FAILED;
  }
  return OK;
}


error_code_t Parameter_CANopen_ReadTableEntry( uint8_t node_id, int16_t parameter_index, parameter_data_t * parameter_data )
{
  uint16_t index = 0x2000 + ((uint16_t)(parameter_index) >> 2);
  uint8_t sub_index = ((uint8_t)(parameter_index) << 6);
  int32_t temp;

  Sdo_ExpRead_Foo( node_id, index, sub_index + SUBIDX_PARAM_PROPERTY, &temp, 0 );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  parameter_data->flags = (uint16_t)temp;
  parameter_data->repr = (repr_t)(temp >> 16);
  parameter_data->unit = (unit_t)(temp >> 24);

  Sdo_ExpRead_Foo( node_id, index, sub_index + SUBIDX_PARAM_NOMINAL, &temp, 0 );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  parameter_data->nominal = (int32_t)temp;

  Sdo_ExpRead_Foo( node_id, index, sub_index + SUBIDX_PARAM_MINIMUM, &temp, 0 );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  parameter_data->minimum = (int32_t)temp;

  Sdo_ExpRead_Foo( node_id, index, sub_index + SUBIDX_PARAM_MAXIMUM, &temp, 0 );
  while( Sdo_ReqIsBusy() );
  if( ! Sdo_ReqIsFinished() )
    return CAN_SDO_CONNECTION_FAILED;
  parameter_data->maximum = (int32_t)temp;

  return OK;
}


// Return values for CANopen_CallbackSdoReq() callback
#define CAN_SDOREQ_NOTHANDLED     0     // process regularly, no impact
#define CAN_SDOREQ_HANDLED_SEND   1     // processed in callback, auto-send returned msg
#define CAN_SDOREQ_HANDLED_NOSEND 2     // processed in callback, don't send response

/**
 * This callback maps the parameter-set to the SDO index range 0x2000..0x5FFF.
 * This function needs to be called if any other SDO request for this
 * particular node cannot be handled.
 */
uint8_t Parameter_CANopen_CallbackSdoReq( uint8_t length_req, uint8_t *req_ptr, uint8_t *length_resp, uint8_t *resp_ptr )
{
  if( 8 == length_req )
  {
    static uint16_t read_ofs;
    static uint16_t buffer_len;
    static uint8_t * read_buffer;
    uint16_t i;
    uint16_t index = req_ptr[1] | (req_ptr[2] << 8);
    int16_t  parameter_index = (int16_t)( ((index - 0x2000) << 2) | (req_ptr[3] >> 6) );
    uint8_t  parameter_sub_index = req_ptr[3] &= 0x0F;
    if( (req_ptr[0] & 0xE0) == 0x40 ) // expedited or segmented read
    {
      if( (index >= 0x2000) && (index <= 0x5FFF) )
      {
        if( ! Parameter_IndexExists(parameter_index) )
          return CAN_SDOREQ_NOTHANDLED;

        parameter_data_t parameter_data;
        Parameter_ReadData(parameter_index, &parameter_data);
        if( (SUBIDX_PARAM_NAME == parameter_sub_index) || (SUBIDX_PARAM_INFO == parameter_sub_index) )
        {
          // segmented read
          read_ofs = 0;

          if( SUBIDX_PARAM_NAME == parameter_sub_index )
            read_buffer = (uint8_t*)parameter_data.name;
          else
            read_buffer = (uint8_t*)Parameter_GetHelp(parameter_index);

          buffer_len = strlen( (char*)read_buffer ) + 1;
          *length_resp = 8;
          resp_ptr[0] = 0x41; // set size indicated
          resp_ptr[1] = req_ptr[1];
          resp_ptr[2] = req_ptr[2];
          resp_ptr[3] = req_ptr[3];
          resp_ptr[4] = buffer_len & 0xFF;
          resp_ptr[5] = buffer_len >> 8;
          resp_ptr[6] = 0;
          resp_ptr[7] = 0;
          return CAN_SDOREQ_HANDLED_SEND;
        }
        else
        {
          // expedited read
          uint32_t payload;
          uint8_t len;
          range_t index_range;
          switch( parameter_sub_index )
          {
            case SUBIDX_PARAM_ACTUAL:
              payload = Parameter_GetValue(parameter_index);
              len = sizeof(int32_t);
              break;

            case SUBIDX_PARAM_NOMINAL:
              payload = parameter_data.nominal;
              len = sizeof(parameter_data.nominal);
              break;

            case SUBIDX_PARAM_MINIMUM:
              payload = parameter_data.minimum;
              len = sizeof(parameter_data.minimum);
              break;

            case SUBIDX_PARAM_MAXIMUM:
              payload = parameter_data.maximum;
              len = sizeof(parameter_data.maximum);
              break;

            case SUBIDX_PARAM_UNIT:
              payload = parameter_data.unit;
              len = sizeof(parameter_data.unit);
              break;

            case SUBIDX_PARAM_REPR:
              payload = parameter_data.repr;
              len = sizeof(parameter_data.repr);
              break;

            case SUBIDX_PARAM_FLAGS:
              payload = parameter_data.flags;
              len = sizeof(parameter_data.flags);
              break;

            case SUBIDX_PARAM_PROPERTY:
              payload = ((parameter_data.unit & 0xFF) << 24) | ((parameter_data.repr & 0xFF) << 16) | (parameter_data.flags & 0xFFFF);
              len = sizeof(payload);
              break;

            case SUBIDX_PARAM_INDEX_MIN:
              index_range = Parameter_GetIndexRange();
              payload = index_range.min;
              len = sizeof(index_range.min);
              break;

            case SUBIDX_PARAM_INDEX_MAX:
              index_range = Parameter_GetIndexRange();
              payload = index_range.max;
              len = sizeof(index_range.max);
              break;

            case SUBIDX_PARAM_INDEX_RANGE:
              index_range = Parameter_GetIndexRange();
              payload = (index_range.max << 16) | (index_range.min & 0xFFFF);
              len = sizeof(index_range);
              break;

            case SUBIDX_PARAM_TABLE_CRC:
              payload = Parameter_GetTableCrc();
              len = sizeof(payload);
              break;

            default:
              return CAN_SDOREQ_NOTHANDLED;
              break;
          }
          resp_ptr[0] = (((4 - len) & 0x3) << 2) | 0x43;
          resp_ptr[1] = req_ptr[1];
          resp_ptr[2] = req_ptr[2];
          resp_ptr[3] = req_ptr[3];
          resp_ptr[4] = payload;
          resp_ptr[5] = payload >> 8;
          resp_ptr[6] = payload >> 16;
          resp_ptr[7] = payload >> 24;
          return CAN_SDOREQ_HANDLED_SEND;
        }
      }
      else if( (index == 0x1008) || (index == 0x100A) )
      {
        // segmented read -- response to product-name and revision
        const char * firmware_name = FIRMWARE_NAME;
        const char * firmware_revision = FIRMWARE_REVISION;
        read_ofs = 0;

        if( index == 0x1008 )     // FIRMWARE_NAME
          read_buffer = (uint8_t*)firmware_name;
        else
          read_buffer = (uint8_t*)firmware_revision;

        buffer_len = strlen( (char*)read_buffer ) + 1;
        *length_resp = 8;
        resp_ptr[0] = 0x41; // set size indicated
        resp_ptr[1] = req_ptr[1];
        resp_ptr[2] = req_ptr[2];
        resp_ptr[3] = req_ptr[3];
        resp_ptr[4] = buffer_len & 0xFF;
        resp_ptr[5] = buffer_len >> 8;
        resp_ptr[6] = 0;
        resp_ptr[7] = 0;
        return CAN_SDOREQ_HANDLED_SEND;
      }
    }
    else if( (req_ptr[0] & 0xE0) == 0x60 ) // respond to segmented read
    {
      uint8_t * data = &resp_ptr[1];
      *length_resp = 8;
      i = 7;
      while( i && (read_ofs < buffer_len) )
      {
        *data++ = read_buffer[read_ofs++];
        i--;
      }
      resp_ptr[0] = (req_ptr[0] & 0x10) | ((7-i) < 1);
      if( read_ofs == buffer_len ) // The whole buffer is read:
      {
        // this is the last segment
        resp_ptr[0] |= 0x01;
      }
      return CAN_SDOREQ_HANDLED_SEND;
    }
    else if( ((req_ptr[0] & 0xE0) == 0x20) && (index >= 0x2000) && (index <= 0x5FFF) && (SUBIDX_PARAM_ACTUAL == parameter_sub_index) ) // expedited write
    {
      int32_t value = (uint32_t)req_ptr[4] | (uint32_t)(req_ptr[5] << 8) | (uint32_t)(req_ptr[6] << 16) | (uint32_t)(req_ptr[7] << 24);
      error_code_t error_code = Parameter_WriteValue(parameter_index, value);
      if( OK == error_code )
      {
        resp_ptr[0] = 0x60;
        resp_ptr[4] = 0;
        resp_ptr[5] = 0;
        resp_ptr[6] = 0;
        resp_ptr[7] = 0;
      }
      else
      {
        resp_ptr[0] = 0x80;
        // map parameter access error to SDO pseudo abort code
        resp_ptr[4] = (uint8_t)error_code;
        resp_ptr[5] = (uint8_t)((uint16_t)error_code >> 8);
        resp_ptr[6] = (PSEUDO_SDO_ABORT_CODE >> 16) & 0xFF;
        resp_ptr[7] = (PSEUDO_SDO_ABORT_CODE >> 24) & 0xFF;
      }
      resp_ptr[1] = req_ptr[1];
      resp_ptr[2] = req_ptr[2];
      resp_ptr[3] = req_ptr[3];
      return CAN_SDOREQ_HANDLED_SEND;
    }
  }
  return CAN_SDOREQ_NOTHANDLED;
}


