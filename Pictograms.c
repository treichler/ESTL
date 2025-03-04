//----------------------------------------------------------------------------//
//  Copyright 2025 Clemens Treichler                                          //
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
 * @file Pictograms.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2025 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */


#include "ESTL_Types.h"
#include "Display.h"
#include "Pictograms.h"

// 8 pixels height
const uint8_t pic_edit_data[]     = {0x9F, 0x8F, 0xC7, 0xE3, 0xF1, 0xFB};
const pictogram_t pic_edit        = {.bitmap = pic_edit_data, .height = 8, .width = sizeof(pic_edit_data)};

const uint8_t pic_apply_data[]    = {0xEF, 0xCF, 0x9F, 0xCF, 0xE7, 0xF3, 0xF9};
const pictogram_t pic_apply       = {.bitmap = pic_apply_data, .height = 8, .width = sizeof(pic_apply_data)};

const uint8_t pic_cancel_data[]   = {0xBD, 0x99, 0xC3, 0xE7, 0xC3, 0x99, 0xBD};
const pictogram_t pic_cancel      = {.bitmap = pic_cancel_data, .height = 8, .width = sizeof(pic_cancel_data)};

const uint8_t pic_up_data[]       = {0xBF, 0x9F, 0x8F, 0x87, 0x83, 0x81, 0x83, 0x87, 0x8F, 0x9F, 0xBF};
const pictogram_t pic_up          = {.bitmap = pic_up_data, .height = 8, .width = sizeof(pic_up_data)};

const uint8_t pic_down_data[]     = {0xFD, 0xF9, 0xF1, 0xE1, 0xC1, 0x81, 0xC1, 0xE1, 0xF1, 0xF9, 0xFD};
const pictogram_t pic_down        = {.bitmap = pic_down_data, .height = 8, .width = sizeof(pic_down_data)};

const uint8_t pic_plus_data[]     = {0xE7, 0xE7, 0x81, 0x81, 0xE7, 0xE7};
const pictogram_t pic_plus        = {.bitmap = pic_plus_data, .height = 8, .width = sizeof(pic_plus_data)};

const uint8_t pic_minus_data[]    = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
const pictogram_t pic_minus       = {.bitmap = pic_minus_data, .height = 8, .width = sizeof(pic_minus_data)};

const uint8_t pic_info_data[]     = {0xB7, 0x85, 0x85, 0xBF};
const pictogram_t pic_info        = {.bitmap = pic_info_data, .height = 8, .width = sizeof(pic_info_data)};

const uint8_t pic_home_data[]     = {0xEF, 0x87, 0xF3, 0xF1, 0x81, 0xB3, 0x87, 0xEF};
const pictogram_t pic_home        = {.bitmap = pic_home_data, .height = 8, .width = sizeof(pic_home_data)};

const uint8_t pic_setting_data[]  = {0xE7, 0xF3, 0xBB, 0x93, 0xC7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7, 0xE3, 0xC9, 0xDD, 0xCF, 0xE7};
const pictogram_t pic_setting     = {.bitmap = pic_setting_data, .height = 8, .width = sizeof(pic_setting_data)};

const uint8_t pic_lamp_on_data[]  = {0xFB, 0xFB, 0xFB, 0xDF, 0xEF, 0xFD, 0xF9, 0x89, 0xF9, 0xFD, 0xEF, 0xDF, 0xFB, 0xFB, 0xFB};
const pictogram_t pic_lamp_on     = {.bitmap = pic_lamp_on_data, .height = 8, .width = sizeof(pic_lamp_on_data)};

const uint8_t pic_lamp_off_data[] = {0xFD, 0xF9, 0xF9, 0xF9, 0xFD};
const pictogram_t pic_lamp_off    = {.bitmap = pic_lamp_off_data, .height = 8, .width = sizeof(pic_lamp_off_data)};

const uint8_t pic_pir_data[]      = { 0xC3, 0xBD, 0xFF, 0xE7, 0xDB, 0xFF, 0xE7, 0xE7, 0xFF, 0xDB, 0xE7, 0xFF, 0xBD, 0xC3};
const pictogram_t pic_pir         = {.bitmap = pic_pir_data, .height = 8, .width = sizeof(pic_pir_data)};

const uint8_t pic_vol_up_data[]   = {0xE7, 0xE7, 0xC3, 0x81, 0xFF, 0xE7, 0xBD, 0xC3};
const pictogram_t pic_vol_up      = {.bitmap = pic_vol_up_data, .height = 8, .width = sizeof(pic_vol_up_data)};

const uint8_t pic_vol_down_data[] = {0xE7, 0xE7, 0xC3, 0x81, 0xFF, 0xE7};
const pictogram_t pic_vol_down    = {.bitmap = pic_vol_down_data, .height = 8, .width = sizeof(pic_vol_down_data)};

const uint8_t pic_play_data[]     = {0x81, 0x81, 0xC3, 0xC3, 0xE7, 0xE7};
const pictogram_t pic_play        = {.bitmap = pic_play_data, .height = 8, .width = sizeof(pic_play_data)};

const uint8_t pic_stop_data[]     = {0x81, 0x81, 0x81, 0x81, 0x81, 0x81};
const pictogram_t pic_stop        = {.bitmap = pic_stop_data, .height = 8, .width = sizeof(pic_stop_data)};

const uint8_t pic_track_next_data[] = {0x81, 0x81, 0xC3, 0xC3, 0xE7, 0xE7, 0xFF, 0x81};
const pictogram_t pic_track_next    = {.bitmap = pic_track_next_data, .height = 8, .width = sizeof(pic_track_next_data)};

const uint8_t pic_track_prev_data[] = {0x81, 0xFF, 0xE7, 0xE7, 0xC3, 0xC3, 0x81, 0x81};
const pictogram_t pic_track_prev    = {.bitmap = pic_track_prev_data, .height = 8, .width = sizeof(pic_track_prev_data)};

const uint8_t pic_next_data[]     = {0x81, 0xC3, 0xE7, 0xFF, 0xFF, 0x81, 0xC3, 0xE7};
const pictogram_t pic_next        = {.bitmap = pic_next_data, .height = 8, .width = sizeof(pic_next_data)};

const uint8_t pic_prev_data[]     = {0xE7, 0xC3, 0x81, 0xFF, 0xFF, 0xE7, 0xC3, 0x81};
const pictogram_t pic_prev        = {.bitmap = pic_prev_data, .height = 8, .width = sizeof(pic_prev_data)};

const uint8_t pic_source_data[]   = {0x9F, 0x9F, 0x83, 0xF7, 0xFF, 0xCF, 0xCF, 0xC1, 0xFB};
const pictogram_t pic_source      = {.bitmap = pic_source_data, .height = 8, .width = sizeof(pic_source_data)};


// 14 pixels height

const uint8_t temp_indoor_14_data[]         = {0x80, 0x00, 0xC0, 0x00, 0xE0, 0x3F, 0x30, 0x00, 0x18, 0x00, 0x0C, 0x1C, 0xE6, 0x3F, 0x13, 0x3F, 0xE3, 0x3F, 0x06, 0x1C, 0x4C, 0x01, 0x18, 0x00, 0x30, 0x00, 0xE0, 0x3F, 0xC0, 0x00, 0x80, 0x00};
const pictogram_t temp_indoor_14            = {.height = 14, .bitmap = temp_indoor_14_data, .width = sizeof(temp_indoor_14_data)/2};

const uint8_t temp_outside_14_data[]        = {0x80, 0x00, 0xC0, 0x00, 0xE0, 0x3F, 0x30, 0x00, 0x18, 0x00, 0x0C, 0x00, 0x06, 0x00, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x18, 0x00, 0x30, 0x00, 0xE0, 0x3F, 0xC0, 0x00, 0x80, 0x00, 0x00, 0x0E, 0xF0, 0x1F, 0x88, 0x1F, 0xF0, 0x1F, 0x00, 0x0E, 0xA0, 0x00};
const pictogram_t temp_outside_14           = {.height = 14, .bitmap = temp_outside_14_data, .width = sizeof(temp_outside_14_data)/2};

const uint8_t solar_14_data[]               = {0x04, 0x00, 0x34, 0x00, 0x03, 0x00, 0x08, 0x00, 0x92, 0x00, 0xC2, 0x00, 0xE0, 0x3F, 0x30, 0x00, 0x18, 0x00, 0x0C, 0x00, 0x06, 0x00, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x18, 0x00, 0x30, 0x00, 0xE0, 0x3F, 0xC0, 0x00, 0x80, 0x00, 0x00, 0x00};
const pictogram_t solar_14                  = {.height = 14, .bitmap = solar_14_data, .width = sizeof(solar_14_data)/2};

const uint8_t battery_14_data[]             = {0xFC, 0x1F, 0x54, 0x15, 0x56, 0x1D, 0x56, 0x1F, 0xD6, 0x1F, 0xF4, 0x1F, 0xFC, 0x1F};
const pictogram_t battery_14                = {.height = 14, .bitmap = battery_14_data, .width = sizeof(battery_14_data)/2};

const uint8_t humidity_14_data[]            = {0x00, 0x0E, 0x80, 0x1F, 0xE0, 0x3C, 0xF8, 0x34, 0xFF, 0x3B, 0xFF, 0x3D, 0xF8, 0x32, 0xE0, 0x33, 0x80, 0x1F, 0x00, 0x0E};
const pictogram_t humidity_14               = {.height = 14, .bitmap = humidity_14_data, .width = sizeof(humidity_14_data)/2};

const uint8_t water_tap_14_data[]           = {0x38, 0x00, 0x38, 0x00, 0x38, 0x00, 0x7D, 0x00, 0x7D, 0x00, 0x7F, 0x00, 0x7D, 0x00, 0x7D, 0x00, 0x38, 0x00, 0x38, 0x00, 0xF8, 0x01, 0xF8, 0x15, 0xF0, 0x01};
const pictogram_t water_tap_14              = {.height = 14, .bitmap = water_tap_14_data, .width = sizeof(water_tap_14_data)/2};

const uint8_t sun_14_data[]                 = {0x40, 0x00, 0x40, 0x00, 0x44, 0x04, 0x08, 0x02, 0xE0, 0x00, 0x10, 0x01, 0x17, 0x1D, 0x10, 0x01, 0xE0, 0x00, 0x08, 0x02, 0x44, 0x04, 0x40, 0x00, 0x40, 0x00};
const pictogram_t sun_14                    = {.height = 14, .bitmap = sun_14_data, .width = sizeof(sun_14_data)/2};

const uint8_t noise_14_data[]               = {0xC0, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0xFC, 0x0F, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xF0, 0x03, 0x00, 0x00, 0xFF, 0x3F, 0x00, 0x00, 0xF0, 0x03, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0xC0, 0x00};
const pictogram_t noise_14                  = {.height = 14, .bitmap = noise_14_data, .width = sizeof(noise_14_data)/2};

const uint8_t hot_surface_14_data[]         = {0x00, 0x0A, 0x0C, 0x0A, 0x92, 0x0A, 0x60, 0x0A, 0x0C, 0x0A, 0x92, 0x0A, 0x60, 0x0A, 0x0C, 0x0A, 0x92, 0x0A, 0x60, 0x0A, 0x00, 0x0A};
const pictogram_t hot_surface_14            = {.height = 14, .bitmap = hot_surface_14_data, .width = sizeof(hot_surface_14_data)/2};

const uint8_t power_indoor_14_data[]        = {0x80, 0x00, 0xC0, 0x00, 0xE0, 0x3F, 0x30, 0x00, 0x98, 0x07, 0x4C, 0x08, 0x06, 0x10, 0xF3, 0x11, 0xF3, 0x11, 0x06, 0x10, 0x4C, 0x08, 0x98, 0x07, 0x30, 0x00, 0xE0, 0x3F, 0xC0, 0x00, 0x80, 0x00};
const pictogram_t power_indoor_14           = {.height = 14, .bitmap = power_indoor_14_data, .width = sizeof(power_indoor_14_data)/2};

const uint8_t socket_indoor_14_data[]       = {0x80, 0x00, 0xC0, 0x00, 0xE0, 0x3F, 0x30, 0x00, 0x98, 0x3F, 0x8C, 0x31, 0x86, 0x24, 0x83, 0x20, 0x86, 0x24, 0x8C, 0x31, 0x98, 0x3F, 0x30, 0x00, 0xE0, 0x3F, 0xC0, 0x00, 0x80, 0x00};
const pictogram_t socket_indoor_14          = {.height = 14, .bitmap = socket_indoor_14_data, .width = sizeof(socket_indoor_14_data)/2};

const uint8_t electricity_indoor_14_data[]  = {0x80, 0x00, 0xC0, 0x00, 0xE0, 0x3F, 0x30, 0x00, 0x18, 0x00, 0x0C, 0x02, 0x06, 0x1B, 0x83, 0x0F, 0xC6, 0x06, 0x0C, 0x02, 0x18, 0x00, 0x30, 0x00, 0xE0, 0x3F, 0xC0, 0x00, 0x80, 0x00};
const pictogram_t electricity_indoor_14     = {.height = 14, .bitmap = electricity_indoor_14_data, .width = sizeof(electricity_indoor_14_data)/2};

const uint8_t electricity_14_data[]         = {0x40, 0x00, 0x60, 0x00, 0x70, 0x0C, 0x78, 0x07, 0xFC, 0x03, 0xEE, 0x01, 0xE3, 0x00, 0x60, 0x00, 0x20, 0x00};
const pictogram_t electricity_14            = {.height = 14, .bitmap = electricity_14_data, .width = sizeof(electricity_14_data)/2};


