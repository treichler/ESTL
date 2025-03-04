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
 * @file Display.h
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2025 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__


/**
 * @ingroup  ESTL
 * @defgroup DISPLAY  Display
 * @brief    Display Module
 *
 * This module implements functionality for monochrome 2-color graphic display.
 * The display's content is stored in a frame-buffer, where each bit represents
 * one pixel.
 * So each modification of display content is performed virtually in RAM.
 * This approach allows maximum compatibility and it requires the application-
 * developer only to implement a single function which writes the frame-buffer's
 * content to the physical display.
 * @{
 */


/**
 * Define Drawing modes.
 */
typedef enum {
  DRAW_BLACK_SOLID,             //!< Solid Black.
  DRAW_WHITE_SOLID,             //!< Solid White.
  DRAW_BLACK_TRANSPARENT,       //!< Black pixel-input drawn black, white pixel-input is interpreted as transparent.
  DRAW_WHITE_TRANSPARENT,       //!< Black pixel-input drawn white, white pixel-input is interpreted as transparent.
} draw_mode_t;


/**
 * Define the structure of pictograms.
 */
typedef struct {
  const uint8_t width;          //!< Pictogram's width.
  const uint8_t height;         //!< Pictogram's height.
  const uint8_t * bitmap;       //!< Pixel-bitmap of the pictogram.
} pictogram_t;


/**
 * Define the structure of fonts.
 */
typedef struct {
  const char    first_char;      //!< First character (according to ASCII) representable by this font.
  const char    last_char;       //!< Last character (according to ASCII) representable by this font.
  const uint8_t nominal_width;   //!< Nominal character width.
  const uint8_t height;          //!< Character height.
  const uint8_t * width;         //!< Array containing widths for each character stored in bitmap; if NULL font is treated as mono-space.
  const uint8_t * bitmap;        //!< Array containing characters' pixel-bitmap
  const char    * special_chars; //!< Additional characters representable by this font.
} font_t;


/**
 * Get the pointer to the frame-buffer.
 *
 * @return    Frame-buffer's start address.
 */
extern const uint8_t * Display_GetFrameBuffer( void );


/**
 * Set the position of the cursor.
 *
 * @param[in]   x               x-coordinate.
 * @param[in]   y               y-coordinate.
 */
extern void Display_SetCursor( int16_t x, int16_t y );


/**
 * Fill the whole display.
 *
 * @param[in]   draw_mode       Fill-color, either DRAW_BLACK_SOLID or DRAW_WHITE_SOLID.
 */
extern void Display_Fill( draw_mode_t draw_mode );


/**
 * Draw a filled rectangle.
 * Starting point is the current cursor position.
 *
 * @param[in]   width           Width of the rectangle.
 * @param[in]   height          Height of the rectangle.
 * @param[in]   draw_mode       Rectangle's color, either DRAW_BLACK_SOLID or DRAW_WHITE_SOLID.
 */
extern void Display_DrawFilledRectangle( uint16_t width, uint16_t height, draw_mode_t draw_mode );


/**
 * Draw a pictogram.
 * Starting point is the current cursor position.
 *
 * @param[in]   pic             Pointer to the pictogram.
 * @param[in]   draw_mode       Desired drawing color.
 */
extern void Display_DrawPic( const pictogram_t * pic, draw_mode_t draw_mode );


/**
 * Draw a single pixel at desired coordinates.
 *
 * @param[in]   x               x-coordinate.
 * @param[in]   y               y-coordinate.
 * @param[in]   draw_mode       Pixel-color, either DRAW_BLACK_SOLID or DRAW_WHITE_SOLID.
 */
extern void Display_DrawPixel( uint16_t x, uint16_t y, draw_mode_t draw_mode);


/**
 * Get the distance between letters for a desired font.
 *
 * @param[in]   font            Desired font.
 * @return    Distance in pixels.
 */
extern uint16_t Display_GetLetterDistance( const font_t * font );


/**
 * Draw a character.
 *
 * @param[in]   c               Character to be drawn.
 * @param[in]   font            Desired Font.
 * @param[in]   draw_mode       Drawing color.
 */
extern char Display_DrawChar( char c, const font_t * font, draw_mode_t draw_mode );


/**
 * Write a String
 *
 * @param[in]   str             Character-string.
 * @param[in]   font            Desired Font.
 * @param[in]   draw_mode       Drawing color.
 */
extern char Display_WriteString( const char * str, const font_t * font, draw_mode_t draw_mode );


/**
 * Get the width of a string.
 *
 * @param[in]   str             Character-string.
 * @param[in]   font            Desired Font.
 * @return    Strings's width in pixels.
 */
extern uint16_t Display_GetStrWidth( const char * str, const font_t * font );


/**
 * @} end of DISPLAY
 */

#endif // __DISPLAY_H__
