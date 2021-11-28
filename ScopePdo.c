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
 * @file ScopePdo.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 *
 * @todo node_id to be handled, so that DAQ from several nodes is possible
 *
 * @todo call of PDO could be implemented similar to Sdo module
 *
 * @todo eventually move implementation to Scope module
 */


#include "ESTL_Types.h"
#include "Error.h"
#include "Parameter.h"
#include "Scope.h"
#include "ScopePdo.h"


#define SCOPE_PDO_NR_OF_BUFFER_ENTRIES  (2)
#define NR_OF_CHANNEL_LEARNING_SAMPLES  (16)


struct {
  scope_pdo_sample_t    sample_buffer[SCOPE_PDO_NR_OF_BUFFER_ENTRIES];
  scope_pdo_sample_t    *new_sample;
  uint8_t               sample_buffer_index;
  uint8_t               channel_index_max;
  uint8_t               channel_learning_counter;
  bool_t                is_new_sample;
} ScopePdo_data;


inline bool_t ScopePdo_HasNewSample( void )
{
  return ScopePdo_data.is_new_sample;
}


inline void ScopePdo_ClearNewSampleFlag( void )
{
  ScopePdo_data.is_new_sample = FALSE;
}


inline void ScopePdo_Clear( void )
{
  ScopePdo_data.channel_index_max = 0;
  ScopePdo_data.channel_learning_counter = 0;
}


inline scope_pdo_sample_t * ScopePdo_GetNewSample( void )
{
  return ScopePdo_data.new_sample;
}


inline uint16_t ScopePdo_GetNrOfChannels( void )
{
  return ScopePdo_data.channel_index_max + 1;
}


void ScopePdo_ReceiveDaq( uint8_t node_id, uint8_t * rx )
{
  uint16_t sample_index, channel_index;
  uint32_t value;
  sample_index   =  (uint16_t)rx[0];
  sample_index  |= ((uint16_t)rx[1] << 8);
  channel_index  =  (uint16_t)rx[2];
  channel_index |= ((uint16_t)rx[3] << 8);
  value          =  (uint32_t)rx[4];
  value         |= ((uint32_t)rx[5] << 8);
  value         |= ((uint32_t)rx[6] << 16);
  value         |= ((uint32_t)rx[7] << 24);

  // check if sample is new
  if( ScopePdo_data.sample_buffer[ScopePdo_data.sample_buffer_index].index != sample_index )
  {
//    if( NR_OF_CHANNEL_LEARNING_SAMPLES > ScopePdo_data.channel_learning_counter )
    {
      ScopePdo_data.is_new_sample = TRUE;
      ScopePdo_data.new_sample = &(ScopePdo_data.sample_buffer[ScopePdo_data.sample_buffer_index]);
      ScopePdo_data.channel_learning_counter ++;
    }
    // use next buffer
    ScopePdo_data.sample_buffer_index ++;
    if( ScopePdo_data.sample_buffer_index >= SCOPE_PDO_NR_OF_BUFFER_ENTRIES )
      ScopePdo_data.sample_buffer_index = 0;
    // set buffer's sample index to new one
    ScopePdo_data.sample_buffer[ScopePdo_data.sample_buffer_index].index = sample_index;
    // clear validity bits
    ScopePdo_data.sample_buffer[ScopePdo_data.sample_buffer_index].validity_bits = 0;
  }
  // write sample to buffer
  if( SCOPE_PDO_MAX_NR_OF_CHANNELS > channel_index )
  {
    ScopePdo_data.sample_buffer[ScopePdo_data.sample_buffer_index].sample[channel_index] = (int32_t)value;
    ScopePdo_data.sample_buffer[ScopePdo_data.sample_buffer_index].node_id = node_id;
    ScopePdo_data.sample_buffer[ScopePdo_data.sample_buffer_index].validity_bits |= (1 << channel_index);
/*
    if( NR_OF_CHANNEL_LEARNING_SAMPLES <= ScopePdo_data.channel_learning_counter )
    {
      if( channel_index == ScopePdo_data.nr_of_channels )
      ScopePdo_data.is_new_sample = TRUE;
    }
    else
    {
      // adapt number of scope channels
      if( ScopePdo_data.nr_of_channels < channel_index )
        ScopePdo_data.nr_of_channels = channel_index;
    }
*/
    // adapt number of scope channels
    if( ScopePdo_data.channel_index_max < channel_index )
      ScopePdo_data.channel_index_max = channel_index;
  }
}


void ScopePdo_PrepareDaqTx( uint8_t * tx, int32_t value, uint16_t channel_index, uint16_t sample_index )
{
  tx[0] = (uint8_t)(sample_index);
  tx[1] = (uint8_t)(sample_index >> 8);
  tx[2] = (uint8_t)(channel_index);
  tx[3] = (uint8_t)(channel_index >> 8);
  tx[4] = (uint8_t)(value);
  tx[5] = (uint8_t)(value >> 8);
  tx[6] = (uint8_t)(value >> 16);
  tx[7] = (uint8_t)(value >> 24);
}

/*
bool_t ScopePdo_PrepareTx( uint8_t * tx, uint16_t channel_index )
{
  tx[0] = (uint8_t)(ScopePdo_data.sample_index);
  tx[1] = (uint8_t)(ScopePdo_data.sample_index >> 8);
  tx[2] = (uint8_t)(channel_index);
  tx[3] = (uint8_t)(channel_index >> 8);
  tx[4] = (uint8_t)(ScopePdo_data.sample[ScopePdo_data.sample_index][channel_index]);
  tx[5] = (uint8_t)(ScopePdo_data.sample[ScopePdo_data.sample_index][channel_index] >> 8);
  tx[6] = (uint8_t)(ScopePdo_data.sample[ScopePdo_data.sample_index][channel_index] >> 16);
  tx[7] = (uint8_t)(ScopePdo_data.sample[ScopePdo_data.sample_index][channel_index] >> 24);
  return FALSE;
}
*/
