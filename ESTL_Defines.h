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
 * @file ESTL_Defines.h
 * @brief Embedded Systems Tiny Library Defines
 *
 * This file contains definitions which ESTL's configuration relies on.
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2021 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */


#ifndef __ESTL_DEFINES_H__
#define __ESTL_DEFINES_H__

/**
 * @defgroup ESTL  Embedded Systems Tiny Library
 */

/**
 * @ingroup  ESTL
 * @defgroup ESTL_DEFINES  Embedded Systems Tiny Library definitions
 * @brief    Holds common definitions for Embedded Systems Tiny Library
 * @{
 */


/**
 * @name  Storage hardware variants
 * @brief Enumerate different storage hardware variants
 * @{
 */
#define ESTL_STORAGE_HARDWARE_FAKE_NV_MEMORY     (1)     //!< Emulate storage functionality in RAM
#define ESTL_STORAGE_HARDWARE_I2CEEPROM          (2)     //!< EEprom connected to I2C
/** @} */


/**
 * @name  Storage hardware EEprom variants
 * @brief Enumerate different EEprom variants
 * @{
 */
#define ESTL_STORAGE_I2CEEPROM_24LC02   		(1)     //!< 24LC02  2kbit/256byte EEprom
#define ESTL_STORAGE_I2CEEPROM_24LC04   		(2)     //!< 24LC04  4kbit/512byte EEprom
#define ESTL_STORAGE_I2CEEPROM_24LC08   		(3)     //!< 24LC08  8kbit/1kbyte EEprom
#define ESTL_STORAGE_I2CEEPROM_24LC16   		(4)     //!< 24LC16 16kbit/2kbyte EEprom
#define ESTL_STORAGE_I2CEEPROM_24LC32   		(5)     //!< 24LC32 32kbit/4kbyte EEprom
/** @} */


/**
 * @name  GLCD hardware variants
 * @brief Enumerate different GLCD hardware variants
 * @{
 */
#define ESTL_GLCD_HARDWARE_KS0108                (1)     //!< Define KS0108 graphic LCD hardware
#define ESTL_GLCD_HARDWARE_S1D15605              (2)     //!< Define S1D15605 graphic LCD hardware
#define ESTL_GLCD_HARDWARE_DOGX128               (3)     //!< Define DOGx128 graphic LCD hardware
/** @} */


/**
 * @name  Help text representation
 * @brief Several variants of how a help text should be treated
 * @{
 */
#define HELP_TEXT_DEFAULT(a)    (a)     //!< Insert the provided help text
#define HELP_TEXT_HIDDEN(a)     ("")    //!< Replace the help text with empty string
/** @} */


/**
 * @} end of ESTL
 */

#endif // __ESTL_DEFINES_H__
