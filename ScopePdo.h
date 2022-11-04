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


#define SCOPE_PDO_MAX_NR_OF_CHANNELS    (16) // valid range: 1..16

typedef struct {
  int32_t  sample[SCOPE_PDO_MAX_NR_OF_CHANNELS];
  uint16_t validity_bits;
  uint8_t  nr_channels;
  uint8_t  node_id;
  uint16_t index;
} scope_pdo_sample_t;


extern void ScopePdo_Init( bool_t (* PrintFunction)(scope_pdo_sample_t*) );
extern void ScopePdo_ReceiveDaq( uint8_t node_id, uint8_t * rx );
extern void ScopePdo_PrepareDaqTx( uint8_t * tx, int32_t value, uint8_t channel_index, uint16_t sample_index );

#endif // __SCOPE_PDO_H__
