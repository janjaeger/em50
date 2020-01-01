/* Clear Register/Memory Instructions
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


E50I(crl)
{
  logop1(op, "crl");
  S_L(cpu, 0);
}


E50I(cra)
{
  logop1(op, "cra");
  S_A(cpu, 0);
}


E50I(crb)
{
  logop1(op, "crb");
  S_B(cpu, 0);
}


E50I(crbx)
{
  logop1(op, "crb");
  S_B(cpu, 0);
  cpu->crs->fr[1].lh = 0;
}


E50I(cre)
{
  logop1(op, "cre");
  S_E(cpu, 0);
}


E50I(crle)
{
  logop1(op, "crle");
  S_L(cpu, 0);
  S_E(cpu, 0);
}


E50I(cal)
{
  logop1(op, "cal");
  S_A(cpu, G_A(cpu) & 0x00ff);
}


E50I(car)
{
  logop1(op, "car");
  S_A(cpu, G_A(cpu) & 0xff00);
}


#ifdef I_MODE
E50I(cr)
{
int dr = op_dr(op);

  logop1o(op, "cr", dr);

  S_R(cpu, dr, 0);
}
#endif


#ifdef I_MODE
E50I(crbl)
{
int dr = op_dr(op);

  logop1o(op, "crbl", dr);

  S_RH(cpu, dr, G_RH(cpu, dr) & 0x00ff);
}
#endif


#ifdef I_MODE
E50I(crbr)
{
int dr = op_dr(op);

  logop1o(op, "crbr", dr);

  S_RH(cpu, dr, G_RH(cpu, dr) & 0xff00);
}
#endif


#ifdef I_MODE
E50I(crhl)
{
int dr = op_dr(op);

  logop1o(op, "crhl", dr);

  S_RH(cpu, dr, 0);
}
#endif


#ifdef I_MODE
E50I(crhr)
{
int dr = op_dr(op);

  logop1o(op, "crhr", dr);

  S_RL(cpu, dr, 0);
}
#endif


#ifdef I_MODE
E50I(zm)
{
  logop1(op, "zm");

  E50X(estore_d)(cpu, op, 0);
}
#endif


#ifdef I_MODE
E50I(zmh)
{
  logop1(op, "zmh");

  E50X(estore_w)(cpu, op, 0);
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
