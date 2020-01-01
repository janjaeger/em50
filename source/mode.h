/* Modes
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


#if !defined E16S && !defined E32S && !defined E32R && !defined E64R && !defined E64V && !defined E32I
 #define _E50M_ST1(_m) #_m
 #define _E50M_STR(_m) _E50M_ST1(_m)
 #define      E64V e64v
 #define E50M E64V
 #define E50X(_n)  e64v_ ## _n
 #define E50W 0xffffffff
 #define V_MODE
 #define km_cmde km_e64v
 #define HMDE
#elif defined E64V
 #undef  HMDE
 #undef       E64V
 #undef       V_MODE
 #undef  E50M
 #undef  E50X
 #undef  E50W
 #define      E32I e32i
 #define E50M E32I
 #define E50X(_n)  e32i_ ## _n
 #define E50W 0xffffffff
 #define I_MODE
 #undef  km_cmde
 #define km_cmde km_e32i
#elif defined E32I
 #undef       E32I
 #undef       I_MODE
 #undef  E50M
 #undef  E50X
 #undef  E50W
 #define      E64R e64r
 #define E50M E64R
 #define E50W 0xffff
 #define E50X(_n)  e64r_ ## _n
 #define R_MODE
 #undef  km_cmde
 #define km_cmde km_e64r
#elif defined E64R
 #undef       E64R
 #undef       R_MODE
 #undef  E50M
 #undef  E50X
 #undef  E50W
 #define      E32R e32r
 #define E50M E32R
 #define E50X(_n)  e32r_ ## _n
 #define E50W 0x7fff
 #define R_MODE
 #undef  km_cmde
 #define km_cmde km_e32r
#elif defined E32R
 #undef       E32R
 #undef       R_MODE
 #undef  E50M
 #undef  E50X
 #undef  E50W
 #define      E32S e32s
 #define E50M E32S
 #define E50X(_n)  e32s_ ## _n
 #define E50W 0x7fff
 #define S_MODE
 #undef  km_cmde
 #define km_cmde km_e32s
#elif defined E32S
 #undef       E32S
 #undef       S_MODE
 #undef  E50M
 #undef  E50X
 #undef  E50W
 #define      E16S e16s
 #define E50M E16S
 #define E50X(_n)  e16s_ ## _n
 #define E50W 0x3fff
 #define S_MODE
 #undef  km_cmde
 #define km_cmde km_e16s
 #define      EMDE
#elif defined E16S
 #error Include recursion (EMDE)
#endif

#undef  E50S
#define E50S _E50M_STR(E50M)

#undef  E50I
#define E50I(_n) \
  void E50X(_n)(cpu_t *cpu, op_t op)
