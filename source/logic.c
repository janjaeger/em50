/* Logic Instructions
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


/* ANA
 * And to A
 * IX001111000Y00 BR\2 DISPLACEMENT\16 (V mode)
 * IX001111000000 CB\2 DISPLACEMENT\16 (R mode)
 * IX0011DISPLACEMENT\10 (S R V)
 */
#ifndef I_MODE
E50I(ana)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t val = E50X(vfetch_w)(cpu, ea);

  logopxoo(op, "ana", ea, val);

  S_A(cpu, (G_A(cpu) & val));
}
#endif


/* ANL
 * And to A Long
 * IX001111000Y11 BR\2 DISPLACEMENT\16 (V mode)
 */
#ifdef V_MODE
E50I(anl)
{
uint32_t ea = E50X(ea)(cpu, op);
uint32_t val = E50X(vfetch_d)(cpu, ea);

  logopxoo(op, "anl", ea, val);

  S_L(cpu, (G_L(cpu) & val));
}
#endif


E50I(ora)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t val = E50X(vfetch_w)(cpu, ea);

  logopxoo(op, "ora", ea, val);

  S_A(cpu, (G_A(cpu) | val));
}


E50I(era)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t val = E50X(vfetch_w)(cpu, ea);

  logopxoo(op, "era", ea, val);

  S_A(cpu, (G_A(cpu) ^ val));
}


E50I(cma)
{
  logop1(op, "cma");

  S_A(cpu, (G_A(cpu) ^ 0xffff));
}


E50I(erl)
{
uint32_t ea = E50X(ea)(cpu, op);
uint32_t val = E50X(vfetch_d)(cpu, ea);

  logopxoo(op, "erl", ea, val);

  S_L(cpu, (G_L(cpu) ^ val));
}


#ifdef I_MODE
E50I(cmh)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);

  logop2oo(op, "cmh", dr, r);

  S_RH(cpu, dr, ~r);
}
#endif


#ifdef I_MODE
E50I(cmr)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);

  logop2oo(op, "cmr", dr, r);

  S_R(cpu, dr, ~r);
}
#endif


#ifdef I_MODE
E50I(n)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint32_t v = E50X(efetch_d)(cpu, op);

  logop2o3(op, "n", dr, r, v);

  S_R(cpu, dr, r & v);
}
#endif


#ifdef I_MODE
E50I(nh)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t v = E50X(efetch_w)(cpu, op);

  logop2o3(op, "nh", dr, r, v);

  S_RH(cpu, dr, r & v);
}
#endif


#ifdef I_MODE
E50I(o)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint32_t v = E50X(efetch_d)(cpu, op);

  logop2o3(op, "o", dr, r, v);

  S_R(cpu, dr, r | v);
}
#endif


#ifdef I_MODE
E50I(oh)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t v = E50X(efetch_w)(cpu, op);

  logop2o3(op, "oh", dr, r, v);

  S_RH(cpu, dr, r | v);
}
#endif


#ifdef I_MODE
E50I(x)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint32_t v = E50X(efetch_d)(cpu, op);

  logop2o3(op, "x", dr, r, v);

  S_R(cpu, dr, r ^ v);
}
#endif


#ifdef I_MODE
E50I(xh)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t v = E50X(efetch_w)(cpu, op);

  logop2o3(op, "xh", dr, r, v);

  S_RH(cpu, dr, r ^ v);
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
