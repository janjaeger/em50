/* Opcode Decoding
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



#include "ea.h"


E50I(decode);
E50I(eio);
E50I(pio);
E50I(irtc);
E50I(irtn);
E50I(inh);
E50I(inhm);
E50I(inhp);
E50I(enb);
E50I(enbm);
E50I(enbp);
E50I(esim);
E50I(evim);
E50I(cai);

#define logopc_adr "%08x "
#define logop1_str "%08x %08x        %-5s"
#define logop2_str "%08x %08x %06x %-5s"

#ifdef DEBUG
 #if 1
  #define logopc(_n, ...) \
  do { \
  /*if(!isalpha(*_n))*/ \
    if(*_n == '*' /* || cpu->crs->km.mode == km_e32i */) \
    { \
  /*  logall("%llu;", cpu->c); */ \
      logall(__VA_ARGS__); \
    } \
    else \
      logmsg(__VA_ARGS__); \
  } while (0)
 #else
  #define logopc(_o, ...) logmsg(__VA_ARGS__)
 #endif
#else
 #define logopc(...)
#endif

#define logop1(_o, _n) \
  logopc(_n, logop1_str "\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], _n)

#define logop1o(_o, _n, _d) \
  logopc(_n, logop1_str " %x\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], _n, _d)

#define logop1oo(_o, _n, _d, _e) \
  logopc(_n, logop1_str " %x %x\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], _n, _d, _e)

#define logop1o3(_o, _n, _d, _e, _f) \
  logopc(_n, logop1_str " %x %x %x\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], _n, _d, _e, _f)

#define logop2(_o, _n) \
  logopc(_n, logop2_str "\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], (_o[2] << 8) | _o[3], _n)

#define logop2d(_o, _n, _d) \
  logopc(_n, logop2_str " %+d""\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], (_o[2] << 8) | _o[3], _n, _d)

#define logop2o(_o, _n, _d) \
  logopc(_n, logop2_str " %x""\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], (_o[2] << 8) | _o[3], _n, _d)

#define logop2oo(_o, _n, _d, _e) \
  logopc(_n, logop2_str " %x %x""\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], (_o[2] << 8) | _o[3], _n, _d, _e)

#define logop2o3(_o, _n, _d, _e, _f) \
  logopc(_n, logop2_str " %x %x %x""\n", cpu->exec ? cpu->exec : cpu->po, (_o[0] << 8) | _o[1], (_o[2] << 8) | _o[3], _n, _d, _e, _f)

#define logopxo(_o, _n, _d) \
 do { \
   if(op_is_long(op)) \
     logop2o(_o, _n, _d); \
   else \
     logop1o(_o, _n, _d); \
 } while (0)

#define logopxoo(_o, _n, _d, _e) \
 do { \
   if(op_is_long(op)) \
     logop2oo(_o, _n, _d, _e); \
   else \
     logop1oo(_o, _n, _d, _e); \
 } while (0)

#if 1
#ifdef I_MODE
 #undef logopr
 #define logopr(_c) \
  logmsg("-> r0 %8.8x r1 %8.8x r2 %8.8x r3 %8.8x\n" \
         "-> r4 %8.8x r5 %8.8x r6 %8.8x r7 %8.8x\n" \
         "-> cbit %d link %d eq %d lt %d dp %d ie %d\n" \
         "-> pb %8.8x sb %8.8x lb %8.8x xb %8.8x\n", \
         G_R(_c, 0), G_R(_c, 1), G_R(_c, 2), G_R(_c, 3), \
         G_R(_c, 4), G_R(_c, 5), G_R(_c, 6), G_R(_c, 7), \
         (_c)->crs->km.cbit, (_c)->crs->km.link,(_c)->crs->km.eq,(_c)->crs->km.lt, \
         (_c)->crs->km.dp, (_c)->crs->km.ie, \
         G_PB(_c), G_SB(_c), G_LB(_c), G_XB(_c))
#else
 #undef logopr
 #define logopr(_c) \
  logmsg("-> a %4.4x b %4.4x x %4.4x y %4.4x e %8.8x\n" \
         "-> cbit %d link %d eq %d lt %d dp %d ie %d\n" \
         "-> pb %8.8x sb %8.8x lb %8.8x xb %8.8x\n", \
         G_A(_c), G_B(_c), G_X(_c), G_Y(_c), G_E(_c), \
         (_c)->crs->km.cbit, (_c)->crs->km.link,(_c)->crs->km.eq,(_c)->crs->km.lt, \
         (_c)->crs->km.dp, (_c)->crs->km.ie, \
         G_PB(_c), G_SB(_c), G_LB(_c), G_XB(_c))
#endif
#else
 #define logopr(_c)
#endif
