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
 * @file ScopePdo.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __SCOPE_PDO_H__
#define __SCOPE_PDO_H__


#define SCOPE_PDO_MAX_NR_OF_CHANNELS    (8) // valid range: 2..8

typedef struct {
  int32_t  sample[SCOPE_PDO_MAX_NR_OF_CHANNELS];
  uint16_t index;
  uint8_t  node_id;
  uint8_t  validity_bits;
} scope_pdo_sample_t;


bool_t ScopePdo_HasNewSample( void );
void ScopePdo_ClearNewSampleFlag( void );
void ScopePdo_Clear( void );
scope_pdo_sample_t * ScopePdo_GetNewSample( void );
uint16_t ScopePdo_GetNrOfChannels( void );
void ScopePdo_ReceiveDaq( uint8_t node_id, uint8_t * rx );
void ScopePdo_PrepareDaqTx( uint8_t * tx, int32_t value, uint16_t channel_index, uint16_t sample_index );

#endif // __SCOPE_PDO_H__
