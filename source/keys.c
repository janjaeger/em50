/* Keys Instructions
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


#if defined S_MODE || defined R_MODE || defined V_MODE
E50I(ink)
{
  logop1(op, "ink");

  km_t km = cpu->crs->km;
  km.vsc = G_VSC(cpu);
  S_A(cpu, km.keys);
}
#endif


#if defined S_MODE || defined R_MODE
E50I(otk)
{
  logop1(op, "otk");

  uint16_t a = G_A(cpu);
  uint16_t k = G_KEYS(cpu);

  k = (k & 0x00ff) | (a & 0xff00);
  S_KEYS(cpu, k);
  S_VSC(cpu, a & 0xff);

  set_cpu_mode(cpu, cpu->crs->km.mode);
}
#endif


E50I(rcb)
{
  logop1(op, "rcb");

  cpu->crs->km.cbit = 0;
}


E50I(scb)
{
  logop1(op, "scb");

  cpu->crs->km.cbit = 1;
}


#if defined V_MODE || defined R_MODE
E50I(tak)
{
  logop1(op, "tak");

  S_KEYS(cpu, G_A(cpu));
  cpu->crs->km.in = cpu->crs->km.sd = 0;

  set_cpu_mode(cpu, cpu->crs->km.mode);
}
#endif


//#if defined V_MODE || defined R_MODE || defined S_MODE
E50I(tka)
{
  logop1(op, "tka");

  S_A(cpu, G_KEYS(cpu));
}
//#endif


#if defined I_MODE
E50I(ink)
{
int dr = op_dr(op);

  logop1(op, "ink");

  S_RH(cpu, dr, G_KEYS(cpu));
}
#endif


#if defined I_MODE
E50I(otk)
{
int dr = op_dr(op);

  logop1(op, "otk");

  S_KEYS(cpu, G_RH(cpu, dr));
  cpu->crs->km.in = cpu->crs->km.sd = 0;

  set_cpu_mode(cpu, cpu->crs->km.mode);
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
