/* Endian Conversion
 *
 *
 * Copyright Notice:
 *
 *   Copyright (C) 1999-2020 Jan Jaeger, All Rights Reserved.
 *
 *
 * This file is part of the Prime 50 Series Emulator (em50).
 *
 *
 * License Statement:
 *
 *   The Prime 50 Series Emulator (em50) is free software:
 *   You can redistribute it and/or modify it under the terms
 *   of the GNU General Public License as published by the
 *   Free Software Foundation, either version 3 of the License,
 *   or (at your option) any later version.
 *
 *   em50 is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with em50.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#ifndef _en_h
#define _en_h

static inline unsigned char is_big_endian(void)
{
  const unsigned short int endianness = 1; 
  return !*(const unsigned char *)&endianness;
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
  #define to_le_16(_x)    (_x)
  #define to_le_32(_x)    (_x)
  #define to_le_64(_x)    (_x)
  #define to_le_128(_x)   (_x)
  #define to_be_16(_x)    __builtin_bswap16(_x)
  #define to_be_32(_x)    __builtin_bswap32(_x)
  #define to_be_64(_x)    __builtin_bswap64(_x)
  #define to_be_128(_x)   __builtin_bswap128(_x)
  #define from_le_16(_x)  (_x)
  #define from_le_32(_x)  (_x)
  #define from_le_64(_x)  (_x)
  #define from_le_128(_x) (_x)
  #define from_be_16(_x)  __builtin_bswap16(_x)
  #define from_be_32(_x)  __builtin_bswap32(_x)
  #define from_be_64(_x)  __builtin_bswap64(_x)
  #define from_be_128(_x) __builtin_bswap128(_x)
#elif __BYTE_ORDER == __BIG_ENDIAN
  #define to_le_16(_x)    __builtin_bswap16(_x)
  #define to_le_32(_x)    __builtin_bswap32(_x)
  #define to_le_64(_x)    __builtin_bswap64(_x)
  #define to_le_128(_x)   __builtin_bswap128(_x)
  #define to_le_128(_x)   (_x)
  #define to_be_16(_x)    (_x)
  #define to_be_32(_x)    (_x)
  #define to_be_64(_x)    (_x)
  #define to_be_128(_x)   (_x)
  #define from_le_16(_x)  __builtin_bswap16(_x)
  #define from_le_32(_x)  __builtin_bswap32(_x)
  #define from_le_64(_x)  __builtin_bswap64(_x)
  #define from_le_128(_x) __builtin_bswap128(_x)
  #define from_be_16(_x)  (_x)
  #define from_be_32(_x)  (_x)
  #define from_be_64(_x)  (_x)
  #define from_be_128(_x) (_x)
#else
  #error Cannot determine byte order
#endif

#endif
