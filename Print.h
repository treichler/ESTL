//----------------------------------------------------------------------------//
//  Copyright 2021  Clemens Treichler                                         //
//  Copyright 2004  Kustaa Nyholm                                             //
//                                                                            //
//  This file is part of ESTL - Embedded Systems Tiny Library,                //
//  see https://github/treichler/ESTL/                                        //
//  The original implementation could be found at                             //
//  http://www.sparetimelabs.com/tinyprintf/tinyprintf.php                    //
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
 * @file Print.h
 *
 * @author Clemens Treichler
 * @author Kustaa Nyholm
 *
 * @copyright Copyright 2021 Clemens Treichler
 * @copyright Copyright 2004 Kustaa Nyholm
 *
 * @license Released under the GNU Lesser General Public License
 */


#ifndef __PRINT_H__
#define __PRINT_H__


/**
 * @ingroup  ESTL
 * @defgroup PRINT  Print
 * @brief Print functionality for embedded systems
 *
 * This library provides a simple and small (+200 loc) printf functionality to
 * be used in embedded systems.
 * Three printf variants are provided: printf(), sprintf() and snprintf().
 * The formats supported by this implementation are: 'd' 'u' 'c' 's' 'x' 'X'
 * and optionally 'l' and 'q' for long data-types respectively Q15.16 fixed
 * point representation.
 * The latter two formats are enabled if the library is compiled with the
 * corresponding defines PRINT_LONG_SUPPORT respectively PRINT_Q15_SUPPORT.
 * Note that this will pull in some long math routines (pun intended!) and
 * thus make your executable noticeably longer.
 * Zero padding and field width are also supported.
 *
 * The memory foot print of course depends on the target CPU, compiler and
 * compiler options, but a rough estimation (based on a H8S target) is about
 * 1.4 kB for code and some twenty 'int's and 'char's, say 60 bytes of stack
 * space.
 * Not too bad. Your mileage may vary. By hacking the source code you can
 * get rid of some hundred bytes, I'm sure, but personally I feel the balance
 * of functionality and flexibility versus  code size is close to optimal for
 * many embedded systems.
 *
 * To use the printf() you need to supply your own character output function,
 * something like:
 *
 * @code
 *   void putc( void *p, char c, int *cnt )
 *   {
 *     while( !SERIAL_PORT_EMPTY ) ;
 *     SERIAL_PORT_TX_REGISTER = c;
 *   }
 * @endcode
 *
 * Before you can call printf() you need to initialize it to use your
 * character output function with something like:
 *
 * @code
 *   Print_Init( NULL, putc );
 * @endcode
 *
 * Notice the 'NULL' in Print_Init() and the parameter 'void* p' in putc(),
 * the NULL (or any pointer) you pass into the init_printf() will eventually be
 * passed to your putc() routine. This allows you to pass some storage space
 * (or anything really) to the character output function, if necessary.
 * This is not often needed but it was implemented like that because it made
 * implementing the sprintf() function so neat (look at the source code).
 * A similar concept could be found in 'int *cnt' in putc() which is used as
 * a character counter for snprintf() and also to limit the written characters
 * to the buffer's size.
 *
 * The code is reentrant, except for the init_printf() function, so it is safe
 * to call it from interrupts too, although this may result in mixed output.
 * If you rely on reentrancy, take care that your putc() function is reentrant!
 *
 * The printf(), sprintf() and snprintf() functions are actually macros that
 * translate to Print_Printf(), Print_Sprintf() and Print_Snprintf().
 * This makes it possible to use them along with 'stdio.h' printf's in a single
 * source file. You just need to undef the names before you include the
 * 'stdio.h'. Note that these are not function like macros, so if you have
 * variables or struct members with these names, things will explode in your
 * face. Without variadic macros this is the best we can do to wrap these
 * function. If it is a problem just give up the macros and use the functions
 * directly or rename them.
 * @{
 */

#include <stdarg.h>


/**
 * Define the put-character function pointer
 */
typedef void (*putcf) (void*,char,int*);


/**
 * Initialize the Print_Printf() function
 * @param[in]  putp  Pointer to a buffer that is forwarded to put-character function
 * @param[in]  putf  Function pointer to put-character function
 */
void Print_Init(void* putp, putcf putf);


/**
 * Print to standard output
 * @param[in]  fmt  A string containing optional format specifiers.
 * @param[in]  ...  Additional arguments, which values are used to replace format specifiers.
 */
void Print_Printf(const char *fmt, ...);


/**
 * Print to a buffer
 * @param[out] s    Buffer where the output is printed to.
 * @param[in]  fmt  A string containing optional format specifiers.
 * @param[in]  ...  Additional arguments, which values are used to replace format specifiers.
 * return           The amount of character to be written.
 */
int Print_Sprintf(char* s, const char *fmt, ...);


/**
 * Print to a sized buffer
 * @param[out] s    Buffer where the output is printed to.
 * @param[in]  n    The buffer's size.
 * @param[in]  fmt  A string containing optional format specifiers.
 * @param[in]  ...  Additional arguments, which values are used to replace format specifiers.
 * return           The amount of character to be written.
 */
int  Print_Snprintf ( char * s, int n, const char * fmt, ... );


/**
 * This is the printer's core function, all other printf()-like
 * functions are derived from it.
 * @param[out] putp  Buffer that is forwarded to putf.
 * @param[in]  putf  Function pointer to put-character function.
 * @param[in]  cnt   Pointer to a counter which is forwarded to putf.
 * @param[in]  fmt   A string containing optional format specifiers.
 * @param[in]  va    List containing additional arguments, which values
 *                   are used to replace format specifiers.
 */
void Print_Format(void* putp, putcf putf, int *cnt, const char *fmt, va_list va);


/**
 * Define aliases to print functions according to
 * standard output's naming convention
 * @{
 */
#define printf   Print_Printf
#define sprintf  Print_Sprintf
#define snprintf Print_Snprintf
/** @} */


/**
 * @} end of PRINT
 */

#endif // __PRINT_H__
