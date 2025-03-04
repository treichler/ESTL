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
 * @file Display.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2025 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */


#include <string.h>
#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include "Display.h"


#ifndef DISPLAY_WIDTH
#define DISPLAY_WIDTH           (128)
#warning "DISPLAY_WIDTH is set to default value. Check if it fits to your application."
#endif

#ifndef DISPLAY_HEIGHT
#define DISPLAY_HEIGHT          (64)
#warning "DISPLAY_HEIGHT is set to default value. Check if it fits to your application."
#endif

#define DISPLAY_PAGE_HEIGHT     (8)

#define FRAME_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / DISPLAY_PAGE_HEIGHT)


struct {
  int16_t x;
  int16_t y;
  uint8_t frame_buffer[FRAME_BUFFER_SIZE];
} Display_Data;


const uint8_t * Display_GetFrameBuffer( void )
{
  return Display_Data.frame_buffer;
}


void Display_SetCursor( int16_t x, int16_t y )
{
  Display_Data.x = x;
  if( 0 > Display_Data.x )
    Display_Data.x = 0;
  if( DISPLAY_WIDTH < Display_Data.x )
    Display_Data.x = DISPLAY_WIDTH;
  Display_Data.y = y;
  if( 0 > Display_Data.y )
    Display_Data.y = 0;
  if( DISPLAY_HEIGHT < Display_Data.y )
    Display_Data.y = DISPLAY_HEIGHT;
}


void Display_Fill( draw_mode_t draw_mode )
{
  uint16_t i;
  uint8_t fill = 0x00;
  if( DRAW_BLACK_SOLID == draw_mode )
    fill = 0xFF;
  for( i = 0; i < FRAME_BUFFER_SIZE; i ++ )
    Display_Data.frame_buffer[i] = fill;
}


extern void Display_DrawFilledRectangle( uint16_t width, uint16_t height, draw_mode_t draw_mode )
{
  uint16_t x, y = 0;
//  printf( "\r\nDisplay_DrawFilledRectangle:" );
  while( (y < height) && ((y + Display_Data.y) < DISPLAY_HEIGHT) )
  {
    uint8_t mask;
    if( (height - y) / DISPLAY_PAGE_HEIGHT )
      mask = 0xFF << ((y + Display_Data.y) % DISPLAY_PAGE_HEIGHT);
    else
    {
      mask = ((0x01 << (height - y)) - 1) << ((y + Display_Data.y) % DISPLAY_PAGE_HEIGHT);
//      mask = (0x01 << (height - y)) - 1;
//      mask = ~(0xFF >> (height - y ));
//      mask = ~(0xFF >> (height - y - (Display_Data.y % DISPLAY_PAGE_HEIGHT)));
//      mask = (0xFF >> (DISPLAY_PAGE_HEIGHT - (height - y)));
//      mask = 0xFF << DISPLAY_PAGE_HEIGHT - height + y - (Display_Data.y % DISPLAY_PAGE_HEIGHT);
    }

    for( x = 0; (x < width) && ((x + Display_Data.x) < DISPLAY_WIDTH); x ++ )
    {
      if( DRAW_BLACK_SOLID == draw_mode )
        Display_Data.frame_buffer[(y + Display_Data.y) / DISPLAY_PAGE_HEIGHT * DISPLAY_WIDTH + x + Display_Data.x] |= mask;
      else if( DRAW_WHITE_SOLID == draw_mode )
        Display_Data.frame_buffer[(y + Display_Data.y) / DISPLAY_PAGE_HEIGHT * DISPLAY_WIDTH + x + Display_Data.x] &= ~mask;
    }
//    printf( " (%d, 0x%02X)", y, mask );
//    printf( " (%d, 0x%02X, %d, 0x%02X)", y, mask, (height - y) / DISPLAY_PAGE_HEIGHT, (height - y) % DISPLAY_PAGE_HEIGHT );
    y += DISPLAY_PAGE_HEIGHT - ((y + Display_Data.y) % DISPLAY_PAGE_HEIGHT);
  }
//  printf( "\r\n" );
}


void Display_DrawPixel( uint16_t x, uint16_t y, draw_mode_t draw_mode)
{
  uint8_t mask = 0x01 << (y % DISPLAY_PAGE_HEIGHT);
  if( (DISPLAY_WIDTH > x) && (DISPLAY_HEIGHT > y) )
  {
    if( DRAW_BLACK_SOLID == draw_mode )
      Display_Data.frame_buffer[(y / DISPLAY_PAGE_HEIGHT * DISPLAY_WIDTH) + x] |= mask;
    else if( DRAW_WHITE_SOLID == draw_mode )
      Display_Data.frame_buffer[(y / DISPLAY_PAGE_HEIGHT * DISPLAY_WIDTH) + x] &= ~mask;
  }
}


void Display_DrawData( const uint8_t * data, int16_t width, int16_t height, draw_mode_t draw_mode )
{
  uint16_t x, y, data_inc;

  data_inc = ((height - 1) / DISPLAY_PAGE_HEIGHT + 1);
/*
  printf( "DrawData: " );
  for( x = 0; (x < width) && (x < DISPLAY_WIDTH - Display_Data.x); x ++ )
    printf( "%02X", data[x] );
*/
  y = 0;
  while( (y < height) && (y < DISPLAY_HEIGHT - Display_Data.y) )
  {
    for( x = 0; (x < width) && (x < DISPLAY_WIDTH - Display_Data.x); x ++ )
    {
      // TODO could be improved: partial byte modify instead of bit modify
//      if( DRAW_BLACK_SOLID == draw_mode )
      uint8_t mask = 0x01 << ((y + Display_Data.y) % DISPLAY_PAGE_HEIGHT);
      {
        if( data[y / DISPLAY_PAGE_HEIGHT + x * data_inc] & (0x01 << (y % DISPLAY_PAGE_HEIGHT)) )
          Display_Data.frame_buffer[((y + Display_Data.y) / DISPLAY_PAGE_HEIGHT * DISPLAY_WIDTH) + x + Display_Data.x] |= mask;
        else
          Display_Data.frame_buffer[((y + Display_Data.y) / DISPLAY_PAGE_HEIGHT * DISPLAY_WIDTH) + x + Display_Data.x] &= ~mask;
//        printf( "%d ", y / DISPLAY_PAGE_HEIGHT + x * data_inc );
//        printf( "%d ", y / DISPLAY_PAGE_HEIGHT * width + x );
//        printf( "%d ", ((y + Display_Data.y) / DISPLAY_PAGE_HEIGHT * DISPLAY_WIDTH) + x + Display_Data.x );
      }
    }
    // TODO to be verified if useful for partial byte modify
//    y += DISPLAY_PAGE_HEIGHT - ((y + Display_Data.y) % DISPLAY_PAGE_HEIGHT);
    y ++;
  }
//  printf( "\r\n" );

}


void Display_DrawPic( const pictogram_t * pic, draw_mode_t draw_mode )
{
  Display_DrawData( pic->bitmap, pic->width, pic->height, draw_mode );
}


uint16_t Display_GetLetterDistance( const font_t * font )
{
  return 1 + (font->nominal_width / 8);
}


char Display_CheckChar( char c, const font_t * font )
{
  // check if character is in standard char-set
  if( (font->first_char <= c) && (font->last_char >= c) )
    return c;

  // check if special char-set is available
  if( NULL != font->special_chars )
  {
    // look for character in special char-set
    uint8_t char_pos = font->last_char;
    const char * special_char = font->special_chars;
    while( *special_char && (*special_char <= c) )
    {
      char_pos ++;
      if( *special_char == c )
        return char_pos;
      special_char ++;
    }
  }
  return '\0';
}


char Display_DrawChar( char c, const font_t * font, draw_mode_t draw_mode )
{
  uint32_t bitmap_offset, i;
  uint16_t width_sum = 0;
  uint16_t font_index;
  uint8_t  font_width;
  char     special_char;

  // check if character is available
  c = Display_CheckChar( c, font );
  if( '\0' == c )
    return c;
/*
  if( ((font->first_char > c) || (font->last_char < c)) )
    return '\0';
*/

  font_index = c - font->first_char;
  if( NULL == font->width )
    font_width = font->nominal_width;
  else
    font_width = font->width[font_index];

  // check if font fits into remaining space on current position
  if( DISPLAY_WIDTH  < (Display_Data.x + font_width) || DISPLAY_HEIGHT < (Display_Data.y + font->height) )
    return '\0';

  if( NULL == font->width )
    width_sum = font->nominal_width * font_index;
  else
  {
    for( i = 0; i < font_index; i ++ )
      width_sum += font->width[i];
  }

  bitmap_offset = width_sum * ((font->height - 1) / DISPLAY_PAGE_HEIGHT + 1);
  Display_DrawData( &font->bitmap[bitmap_offset], font_width, font->height, DRAW_BLACK_SOLID );
//  bitmap_offset = (c - font->first_char) * font->width * ((font->height - 1) / DISPLAY_PAGE_HEIGHT + 1);
//  Display_DrawData( &font->bitmap[bitmap_offset], font->width, font->height, DRAW_BLACK_SOLID );

  // increment cursor according to character width and distance
  Display_Data.x += font_width + Display_GetLetterDistance( font );
  if( DISPLAY_WIDTH < Display_Data.x )
    Display_Data.x = DISPLAY_WIDTH;

  // return written char for validation
  return c;
}


char Display_WriteString( const char * str, const font_t * font, draw_mode_t draw_mode )
{
  // Write until null-byte
  while( *str )
  {
    if( Display_DrawChar(*str, font, draw_mode) != *str )
    {
      // Char could not be written
//      return *str;
    }

    // Next char
    str++;
  }

  // Everything ok
  return *str;
}


uint16_t Display_GetStrWidth( const char * str, const font_t * font )
{
  uint16_t width_sum = 0;
  uint16_t len = strlen( str );
  uint16_t distance = Display_GetLetterDistance( font );
  if( 0 < len )
  {
    if( NULL == font->width )
      width_sum = font->nominal_width * len;
    else
    {
      while( *str )
      {
        char c = Display_CheckChar( *str, font );
        if( c )
          width_sum += font->width[c - font->first_char] + distance;
        str ++;
      }
    }
    return width_sum - distance;
  }
  return 0;
}

