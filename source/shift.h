/* Register Shift Instructions
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


#ifndef _shift_h
#define _shift_h


static inline int shift_tc(int n)
{
  return 64 - n;
}

static inline int shift_op(op_t op)
{
int n = (op[1] & 0b00111111);
    
  return shift_tc(n);
}

static inline int shift_ea(uint32_t ea)
{
int n = ea & 0b00111111;

  return shift_tc(n);
}


static inline int shift_right_logical32(uint32_t *val, int n)
{
int o;
uint64_t v = *val;

  if(n > 63)
    n = 63;

  v <<= 1;
  v >>= n;
  o = v & 1;
  v >>= 1;

  *val = v;
  return o;
}


static inline int shift_left_logical32(uint32_t *val, int n)
{
int o;
uint64_t v = *val;

  if(n > 63)
    n = 63;

  v <<= n;
  o = (v & 0x100000000ULL) != 0;

  *val = v;
  return o;


  return shift_right_logical32(val, -n);
}


static inline int shift_right_logical16(uint16_t *val, int n)
{
int o;
uint32_t v = *val;

  if(n > 31)
    n = 31;

  v <<= 1;
  v >>= n;
  o = v & 1;
  v >>= 1;

  *val = v;
  return o;
}


static inline int shift_left_logical16(uint16_t *val, int n)
{
int o;
uint32_t v = *val;

  if(n > 31)
    n = 31;

  v <<= n;
  o = (v & 0x10000ULL) != 0;

  *val = v;
  return o;
}


static inline int shift_right_arithmetic16(int16_t *val, int n)
{
int o;
int32_t v = *val;

  if(n > 31)
    n = 31;

  v <<= 1;

  v >>= n;
  o = v & 1;
  v >>= 1;

  *val = v;
  return o;
}


static inline int shift_left_arithmetic16(int16_t *val, int n)
{
int o;
int32_t v = *val;

  if(n > 31)
    n = 31;

  v <<= n;

  o = ((*val == 0) || (n < 16 && ((v >> 15) == 0 || (v >> 15) == -1))) ? 0 : 1;

  *val = v;

  return o;
}


static inline int shift_right_arithmetic32(int32_t *val, int n)
{
int o;
int64_t v = *val;

  if(n > 63)
    n = 63;

  v <<= 1;
  v >>= n;
  o = v & 1;
  v >>= 1;

  *val = v;
  return o;
}


static inline int shift_left_arithmetic32(int32_t *val, int n)
{
int o;
int64_t v = *val;

  if(n > 63)
    n = 63;

  v <<= n;

  o = ((*val == 0) || (n < 32 && ((v >> 31) == 0 || (v >> 31) == -1))) ? 0 : 1;

  *val = v;

  return o;
}

static inline int rotate_right16(uint16_t *val, int n)
{
  n = n & 15;

  if (n)
    *val = (*val >> n) | (*val << (16-n));

  return *val >> 15;
}

static inline int rotate_left16(uint16_t *val, int n)
{
  n = n & 15;

  if (n)
    *val = (*val << n) | (*val >> (16-n));

  return *val & 1;
}

static inline int rotate_right32(uint32_t *val, int n)
{
  n = n & 31;

  if (n)
    *val = (*val >> n) | (*val << (32-n));

  return *val >> 31;
}

static inline int rotate_left32(uint32_t *val, int n)
{
  n = n & 31;

  if (n)
    *val = (*val << n) | (*val >> (32-n));

  return *val & 1;
}
#endif


E50I(lrl);
E50I(lrs);
E50I(lrr);
E50I(arl);
E50I(ars);
E50I(arr);
E50I(lll);
E50I(lls);
E50I(llr);
E50I(all);
E50I(als);
E50I(alr);

E50I(rot);
E50I(sha);
E50I(shl);
E50I(shl1);
E50I(shl2);
E50I(shr1);
E50I(shr2);
E50I(sl1);
E50I(sl2);
E50I(sr1);
E50I(sr2);
