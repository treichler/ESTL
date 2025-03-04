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
 * @file Print.c
 *
 * @author Clemens Treichler
 * @author Kustaa Nyholm
 *
 * @copyright Copyright 2021 Clemens Treichler
 * @copyright Copyright 2004 Kustaa Nyholm
 *
 * @license Released under the GNU Lesser General Public License
 */

#include "ESTL_Config.h"
#include "ESTL_Types.h"
#include "Print.h"

static putcf stdout_putf;
static void* stdout_putp;

#ifdef PRINT_LONG_SUPPORT

static void uli2a(unsigned long int num, unsigned int base, int uc,char * bf)
    {
    int n=0;
    unsigned int d=1;
    while (num/d >= base)
        d*=base;
    while (d!=0) {
        int dgt = num / d;
        num%=d;
        d/=base;
        if (n || dgt>0|| d==0) {
            *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
            ++n;
            }
        }
    *bf=0;
    }

static void li2a (long num, char * bf)
    {
    if (num<0) {
        num=-num;
        *bf++ = '-';
        }
    uli2a(num,10,0,bf);
    }

#endif

static void ui2a(unsigned int num, unsigned int base, int uc,char * bf)
    {
    int n=0;
    unsigned int d=1;
    while (num/d >= base)
        d*=base;
    while (d!=0) {
        int dgt = num / d;
        num%= d;
        d/=base;
        if (n || dgt>0 || d==0) {
            *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
            ++n;
            }
        }
    *bf=0;
    }

static void i2a (int num, char * bf)
    {
    if (num<0) {
        num=-num;
        *bf++ = '-';
        }
    ui2a(num,10,0,bf);
    }

#ifdef PRINT_Q15_SUPPORT
int q2d(q15_t q, unsigned char len)
{
  int d = 1;
  char i;
  if(len > 9)
    len = 9;
  for(i = 0; i < len; i ++)
    d *= 10;
  return (int)(((long int)(q & 0xFFFF) * d) >> 16)  ;
}
#endif

static int a2d(char ch)
    {
    if (ch>='0' && ch<='9')
        return ch-'0';
    else if (ch>='a' && ch<='f')
        return ch-'a'+10;
    else if (ch>='A' && ch<='F')
        return ch-'A'+10;
    else return -1;
    }

static char a2i(char ch, char** src,int base,int* nump)
    {
    char* p= *src;
    int num=0;
    int digit;
    while ((digit=a2d(ch))>=0) {
        if (digit>base) break;
        num=num*base+digit;
        ch=*p++;
        }
    *src=p;
    *nump=num;
    return ch;
    }

static void putchw(void* putp, putcf putf, int *cnt, int n, char z, char *bf)
    {
    char fc=z? '0' : ' ';
    char ch;
    char* p=bf;
    while (*p++ && n > 0)
        n--;
    while (n-- > 0)
        putf(putp, fc, cnt);
    while ((ch= *bf++))
        putf(putp, ch, cnt);
    }

void Print_Format(void* putp, putcf putf, int *cnt, const char *fmt, va_list va)
    {
    char bf[12];

    char ch;


    while ((ch=*(fmt++))) {
        if (ch!='%')
            putf(putp,ch,cnt);
        else {
            char lz=0;
#ifdef  PRINT_LONG_SUPPORT
            char lng=0;
#endif
#ifdef  PRINT_Q15_SUPPORT
            int dl = 3;
#endif
            int w=0;
            ch=*(fmt++);
            if (ch=='0') {
                ch=*(fmt++);
                lz=1;
                }
            if (ch>='0' && ch<='9') {
                ch=a2i(ch,(char**)(&fmt),10,&w);
                }
#ifdef  PRINT_LONG_SUPPORT
            if (ch=='l') {
                ch=*(fmt++);
                lng=1;
            }
#endif
#ifdef  PRINT_Q15_SUPPORT
            if (ch == '.')
            {
              ch=*(fmt++);
              ch=a2i(ch,(char**)(&fmt),10,&dl);
            }
#endif
            switch (ch) {
                case 0:
                    goto abort;
                case 'u' : {
#ifdef  PRINT_LONG_SUPPORT
                    if (lng)
                        uli2a(va_arg(va, unsigned long int),10,0,bf);
                    else
#endif
                    ui2a(va_arg(va, unsigned int),10,0,bf);
                    putchw(putp,putf,cnt,w,lz,bf);
                    break;
                    }
                case 'd' :  {
#ifdef  PRINT_LONG_SUPPORT
                    if (lng)
                        li2a(va_arg(va, unsigned long int),bf);
                    else
#endif
                    i2a(va_arg(va, int),bf);
                    putchw(putp,putf,cnt,w,lz,bf);
                    break;
                    }
#ifdef  PRINT_Q15_SUPPORT
                case 'q' :  {
#ifdef  PRINT_LONG_SUPPORT
#endif
                    q15_t q_val = va_arg(va, q15_t);
                    char * bf_ptr = bf;
                    if( q_val < 0 )
                    {
                      q_val = -q_val;
                      *bf_ptr++ = '-';
                    }
                    w = w - (dl + 1);
                    ui2a( (int)q15_to_int16(q_val), 10, 0, bf_ptr );
                    putchw(putp,putf,cnt,(w > 0) ? w : 0,lz,bf);
                    putf(putp,'.',cnt);
                    ui2a( q2d( (int)q15_GetFraction(q_val), dl ), 10, 0, bf );
                    putchw(putp,putf,cnt,dl,1,bf);
                    break;
                    }
#endif
                case 'x': case 'X' :
#ifdef  PRINT_LONG_SUPPORT
                    if (lng)
                        uli2a(va_arg(va, unsigned long int),16,(ch=='X'),bf);
                    else
#endif
                    ui2a(va_arg(va, unsigned int),16,(ch=='X'),bf);
                    putchw(putp,putf,cnt,w,lz,bf);
                    break;
                case 'c' :
                    putf(putp,(char)(va_arg(va, int)),cnt);
                    break;
                case 's' :
                    putchw(putp,putf,cnt,w,0,va_arg(va, char*));
                    break;
                case '%' :
                    putf(putp,ch,cnt);
                    break;
                default:
                    break;
                }
            }
        }
    abort:;
    }


void Print_Init(void* putp, putcf putf)
    {
    stdout_putf = putf;
    stdout_putp = putp;
    }


void Print_Printf(const char *fmt, ...)
    {
    va_list va;
    va_start(va, fmt);
    Print_Format(stdout_putp, stdout_putf, NULL, fmt, va);
    va_end(va);
    }


static void putcp(void* p, char c, int *cnt)
{
  *(*((char**)p))++ = c;
  (*cnt) ++;
}


int Print_Sprintf(char* s,const char *fmt, ...)
{
  int cnt = 0;
  va_list va;
  va_start(va, fmt);
  Print_Format(&s, putcp, &cnt, fmt, va);
  putcp(&s, '\0', &cnt);
  va_end(va);
  return cnt - 1;
}


static void putncp(void* p, char c, int *cnt)
{
  if( *cnt > 1 )
    *(*((char**)p))++ = c;
  else if( *cnt == 1 )
    *(*((char**)p))++ = '\0';
  (*cnt) --;
}


int Print_Snprintf ( char * s, int n, const char * fmt, ... )
{
  int cnt = n;
  va_list va;
  va_start(va, fmt);
  Print_Format(&s, putncp, &cnt, fmt, va);
  putncp(&s, '\0', &cnt);
  va_end(va);
  return n - cnt - 1;
}

