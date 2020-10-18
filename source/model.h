/* Model
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


#ifndef _model_h
#define _model_h

#if !defined(MODEL)

typedef struct cpumodel_t {
  const char *name;
  uint16_t number;
  enum { hmap=0, pmt=1, pmtx=2 } have_pmt;
  int  nrf;
  enum { cslow=0, cshigh=1 } cs_high;
  enum { earlier=0, p750=1, current=2 } ea;
  enum { single=0, multi=1 } mp;
  uint16_t ucodeman;
  uint16_t ucodeeng;
  uint16_t ucodepln;
  uint16_t ucodeext;
} cpumodel_t;

cpumodel_t *get_cpumodel(const char *);
cpumodel_t *default_cpumodel(void);
cpumodel_t *list_cpumodel(const unsigned int);

#else

#define P400A   0
#define P400B   1
#define Pxxx    2
#define P350    3
#define P250II  4
#define P450    P250II
#define P550    P450
#define P750    5
#define P650    6
#define P150    7
#define P250    P150
#define P850    8
#define P450II  9
#define P550M   P450II
#define P550II  10
#define P650M   P550II
#define P2250   11
#define P750Y   12
#define P550Y   13
#define P850Y   14
#define P9950   15
#define P9650   16
#define P2550   17
#define P9955   18
#define P9750   19
#define P2150   20
#define P2350   21
#define P2655   22
#define P9655   23
#define P9955T  24
#define P2450   25
#define P4050   26
#define P4150   27
#define P6350   28
#define P6550   29
#define P9955II 30
#define P2755   31
#define P2455   32
#define P5310   33
#define P9755   34
#define P2850   35
#define P2950   36
#define P5330   37
#define P4450   38
#define P5370   39
#define P6650   40
#define P6450   41
#define P6150   42
#define P5320   43
#define P5340   44

/*
 * 2350-2755, 9650, 9655 8 user register files (11 total)
 * 6350, 9750 - 9955 II  4 user register files (8 total)
 * all others            2 user register files (4 total)
 */
#if MODEL == P2350   \
 || MODEL == P2450   \
 || MODEL == P2455   \
 || MODEL == P2550   \
 || MODEL == P2655   \
 || MODEL == P2755   \
 || MODEL == P9650   \
 || MODEL == P9655
 #define em50_nrf           8
#elif MODEL == P9750   \
   || MODEL == P9755   \
   || MODEL == P9950   \
   || MODEL == P9955   \
   || MODEL == P9955II \
   || MODEL >= P6350
 #define em50_nrf           4
#else
 #define em50_nrf           2
#endif

/* 
 * 6350               PMT bits 17-32 VA 23-32 RA 26 bits
 * 9955II             PMT bits 19-32 VA 23-32 RA 24 bits
 * 2755 and 9750-9955 PMT bits 20-32 VA 23-32 RA 23 bits
 * Others             HMAP bits 5-16 VA 23-32 RA 22 bits
 */
#if MODEL == P2755   \
 || MODEL == P9750   \
 || MODEL == P9755   \
 || MODEL == P9950   \
 || MODEL == P9955   \
 || MODEL == P9955II \
 || MODEL >= P6350
 #define em50_have_pmt
#endif

#if MODEL == P5310   \
 || MODEL == P5330   \
 || MODEL == P5370   \
 || MODEL == P5320   \
 || MODEL == P5340
 #define em50_have_pmtx
#endif

#if MODEL < P750
 #define em50_ea_earlier
#elif MODEL < P450II
 #define em50_ea_p750
#else
 #define em50_ea_current
#endif

#if MODEL == P6350
 #define em50_cs_high
#endif

#endif

#endif
