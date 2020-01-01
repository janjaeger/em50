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


#include "emu.h"

#include "mode.h"

#include "opcode.h"

#include "shift.h"

#include "int.h" // to/from31


/* ARL n
 * A Right Logical
 * 0100000100 N\6 (S R V)
 */
E50I(arl)
{
int n = shift_op(op);
uint16_t a = G_A(cpu);
int o;

  logop1o(op, "arl", n);

  o = shift_right_logical16(&a, n);

  S_A(cpu, a);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


/* ARR n
 * A Right Rotate
 * 0100000110 N\6 (S R V)
 */
E50I(arr)
{
int n = shift_op(op);
uint16_t a = G_A(cpu);
int o;

  logop1o(op, "arr", n);

  o = rotate_right16(&a, n);

  S_A(cpu, a);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


/* ARS n
 * A Arithmetic Right Shift
 * 0100000101 N\6 (S R V)
 */
E50I(ars)
{
int n = shift_op(op);
int16_t a = G_A(cpu);
int o;

  logop1o(op, "ars", n);

  o = shift_right_arithmetic16(&a, n);

  S_A(cpu, a);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(lrl)
{
int n = shift_op(op);
uint32_t l = G_L(cpu);
int o;

  logop1o(op, "lrl", n);

  o = shift_right_logical32(&l, n);

  S_L(cpu, l);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(lrs)
{
int n = shift_op(op);
int32_t l = G_L(cpu);
int o;

  logop1oo(op, "lrs", l, n);

#if defined S_MODE || defined R_MODE
uint32_t b1 = l & 0x00008000;
  l = to31(l);
#endif

  o = shift_right_arithmetic32(&l, n);

#if defined S_MODE || defined R_MODE
  l = from31(l);
  l |= b1;
#endif

  S_L(cpu, l);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}

E50I(lrr)
{
int n = shift_op(op);
uint32_t l = G_L(cpu);
int o;

  logop1o(op, "lrr", n);

  o = rotate_right32(&l, n);

  S_L(cpu, l);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}

E50I(lll)
{
int n = shift_op(op);
uint32_t l = G_L(cpu);
int o;

  logop1o(op, "lll", n);

  o = shift_left_logical32(&l, n);

  S_L(cpu, l);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(lls)
{
int n = shift_op(op);
int32_t l = G_L(cpu);

  logop1o(op, "lls", n);

#if defined S_MODE || defined R_MODE
uint32_t b1 = l & 0x00008000;
  l = to31(l);

  int ovf = shift_left_arithmetic31(&l, n);

  l = from31(l);
  l |= b1;
#else
  int ovf = shift_left_arithmetic32(&l, n);
#endif

  S_L(cpu, l);
  cpu->crs->km.cbit = ovf;

  if(ovf)
    E50X(int_ovf)(cpu);
}


E50I(llr)
{
int n = shift_op(op);
uint32_t l = G_L(cpu);
int o;

  logop1o(op, "llr", n);

  o = rotate_left32(&l, n);

  S_L(cpu, l);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


/* ALL n
 * A Left Logical
 * 0100001100 N\6 (S R V)
 */
#ifndef I_MODE
E50I(all)
{
int n = shift_op(op);
uint16_t a = G_A(cpu);
int o;

  logop1o(op, "all", n);

  o = shift_left_logical16(&a, n);

  S_A(cpu, a);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}
#endif


/* ALS n
 * A Arithmetic Left Shift
 * 0100001101 N\6 (S R V)
 */
#ifndef I_MODE
E50I(als)
{
int n = shift_op(op);
int16_t a = G_A(cpu);

  logop1o(op, "als", n);

  int ovf = shift_left_arithmetic16(&a, n);

  S_A(cpu, a);
  cpu->crs->km.cbit = ovf;

  if(ovf)
    E50X(int_ovf)(cpu);
}
#endif


/* ALR n
 * A Left Rotate
 * 0100001110 N\6 (S R V)
 */
#ifndef I_MODE
E50I(alr)
{
int n = shift_op(op);
uint16_t a = G_A(cpu);

  logop1o(op, "alr", n);

  int o = rotate_left16(&a, n);

  S_A(cpu, a);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}
#endif


#ifdef I_MODE
E50I(rot)
{
int dr = op_dr(op);
uint32_t ea = E50X(ea)(cpu, op);
int n = shift_ea(ea);
int o;

  logop1o3(op, "rot", dr, n, ea);

  if((ea & 0x4000)) // Bit 18 1: halfword shift 0: fullword shift
  {
  uint16_t r = G_RH(cpu, dr);

    if((ea & 0x8000))
      o = rotate_right16(&r, n);
    else
      o = rotate_left16(&r, n);

    S_RH(cpu, dr, r);
  }
  else
  {
  uint32_t r = G_R(cpu, dr);

    if((ea & 0x8000))
      o = rotate_right32(&r, n);
    else
      o = rotate_left32(&r, n);

    S_R(cpu, dr, r);
  }

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(sha)
{
int dr = op_dr(op);
uint32_t ea = E50X(ea)(cpu, op);
int n = shift_ea(ea);
int o;

  logop1o3(op, "sha", dr, n, ea);

  if((ea & 0x4000)) // Bit 18 1: halfword shift 0: fullword shift
  {
  uint16_t r = G_RH(cpu, dr);

    if((ea & 0x8000))
      o = shift_right_arithmetic16((int16_t *)&r, n);
    else
      o = shift_left_arithmetic16((int16_t *)&r, n);

    S_RH(cpu, dr, r);
  }
  else
  {
  uint32_t r = G_R(cpu, dr);

    if((ea & 0x8000))
      o = shift_right_arithmetic32((int32_t *)&r, n);
    else
      o = shift_left_arithmetic32((int32_t *)&r, n);

    S_R(cpu, dr, r);
  }

  cpu->crs->km.cbit = o ? 1 : 0;
  if(!(ea & 0x8000))
    cpu->crs->km.link = cpu->crs->km.cbit;
  else
    if(cpu->crs->km.cbit)
      E50X(int_ovf)(cpu);
}


E50I(shl)
{
int dr = op_dr(op);
uint32_t ea = E50X(ea)(cpu, op);
int n = shift_ea(ea);
int o;

  logop1o3(op, "shl", dr, n, ea);

  if((ea & 0x4000)) // Bit 18 1: halfword shift 0: fullword shift
  {
  uint16_t r = G_RH(cpu, dr);

    if((ea & 0x8000))
      o = shift_right_logical16(&r, n);
    else
      o = shift_left_logical16(&r, n);

    S_RH(cpu, dr, r);
  }
  else
  {
  uint32_t r = G_R(cpu, dr);

    if((ea & 0x8000))
      o = shift_right_logical32(&r, n);
    else
      o = shift_left_logical32(&r, n);

    S_R(cpu, dr, r);
  }

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(shl1)
{
int dr = op_dr(op);

  logop1o(op, "shl1", dr);

  uint16_t r = G_RH(cpu, dr);
  int o = shift_left_logical16(&r, 1);
  S_RH(cpu, dr, r);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(shl2)
{
int dr = op_dr(op);

  logop1o(op, "shl2", dr);

  uint16_t r = G_RH(cpu, dr);
  int o = shift_left_logical16(&r, 2);
  S_RH(cpu, dr, r);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(shr1)
{
int dr = op_dr(op);

  logop1o(op, "shr1", dr);

  uint16_t r = G_RH(cpu, dr);
  int o = shift_right_logical16(&r, 1);
  S_RH(cpu, dr, r);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(shr2)
{
int dr = op_dr(op);

  logop1o(op, "shr2", dr);

  uint16_t r = G_RH(cpu, dr);
  int o = shift_right_logical16(&r, 2);
  S_RH(cpu, dr, r);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(sl1)
{
int dr = op_dr(op);

  logop1o(op, "sl1", dr);

  uint32_t r = G_R(cpu, dr);
  int o = shift_left_logical32(&r, 1);
  S_R(cpu, dr, r);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(sl2)
{
int dr = op_dr(op);

  logop1o(op, "sl2", dr);

  uint32_t r = G_R(cpu, dr);
  int o = shift_left_logical32(&r, 2);
  S_R(cpu, dr, r);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(sr1)
{
int dr = op_dr(op);

  logop1o(op, "sr1", dr);

  uint32_t r = G_R(cpu, dr);
  int o = shift_right_logical32(&r, 1);
  S_R(cpu, dr, r);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}


E50I(sr2)
{
int dr = op_dr(op);

  logop1o(op, "sr2", dr);

  uint32_t r = G_R(cpu, dr);
  int o = shift_right_logical32(&r, 2);
  S_R(cpu, dr, r);

  cpu->crs->km.cbit = cpu->crs->km.link = o ? 1 : 0;
}

#endif


#ifndef EMDE
 #include __FILE__
#endif
