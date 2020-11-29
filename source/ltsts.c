/* Logical Test and Set Instructions
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


E50I(lf)
{
  logop1(op, "lf");

#ifdef I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, 0);
#else
  S_A(cpu, 0);
#endif
}


E50I(lt)
{
  logop1(op, "lt");

#ifdef I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, 1);
#else
  S_A(cpu, 1);
#endif
}


E50I(leq)
{
  logop1(op, "leq");
  
#ifdef I_MODE
  int dr = op_dr(op);
  int32_t r = G_R(cpu, dr);

  SET_CC(cpu, r);

  if(CC_EQ(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_EQ(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lne)
{
  logop1(op, "lne");
  
#ifdef I_MODE
  int dr = op_dr(op);
  int32_t r = G_R(cpu, dr);

  SET_CC(cpu, r);

  if(CC_NE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_NE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lle)
{
  logop1(op, "lle");
  
#ifdef I_MODE
  int dr = op_dr(op);
  int32_t r = G_R(cpu, dr);

  SET_CC(cpu, r);

  if(CC_LE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_LE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lge)
{
  logop1(op, "lge");
  
#ifdef I_MODE
  int dr = op_dr(op);
  int32_t r = G_R(cpu, dr);

  SET_CC(cpu, r);

  if(CC_GE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_GE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lgt)
{
  logop1(op, "lgt");
  
#ifdef I_MODE
  int dr = op_dr(op);
  int32_t r = G_R(cpu, dr);

  SET_CC(cpu, r);

  if(CC_GT(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_GT(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(llt)
{
  logop1(op, "llt");
  
#ifdef I_MODE
  int dr = op_dr(op);
  int32_t r = G_R(cpu, dr);

  SET_CC(cpu, r);

  if(CC_LT(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_LT(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lleq)
{
int32_t l = (int32_t)G_L(cpu);

  logop1(op, "lleq");
  
  SET_CC(cpu, l);

  if(CC_EQ(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
}


E50I(llne)
{
int32_t l = (int32_t)G_L(cpu);

  logop1(op, "llne");
  
  SET_CC(cpu, l);

  if(CC_NE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
}


E50I(llge)
{
int32_t l = (int32_t)G_L(cpu);

  logop1(op, "llge");
  
  SET_CC(cpu, l);

  if(CC_GE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
}


E50I(llgt)
{
int32_t l = (int32_t)G_L(cpu);

  logop1(op, "llgt");
  
  SET_CC(cpu, l);

  if(CC_GT(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
}


E50I(llle)
{
int32_t l = (int32_t)G_L(cpu);

  logop1(op, "llle");
  
  SET_CC(cpu, l);

  if(CC_LE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
}


E50I(lllt)
{
int32_t l = (int32_t)G_L(cpu);

  logop1(op, "lllt");
  
  SET_CC(cpu, l);

  if(CC_LT(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
}


E50I(lceq)
{
  logop1(op, "lceq");

#ifdef I_MODE
  int dr = op_dr(op);

  if(CC_EQ(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  if(CC_EQ(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lcne)
{
  logop1(op, "lcne");

#ifdef I_MODE
  int dr = op_dr(op);

  if(CC_NE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  if(CC_NE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lcge)
{
  logop1(op, "lcge");

#ifdef I_MODE
  int dr = op_dr(op);

  if(CC_GE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  if(CC_GE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lcgt)
{
  logop1(op, "lcgt");

#ifdef I_MODE
  int dr = op_dr(op);

  if(CC_GT(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  if(CC_GT(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lcle)
{
  logop1(op, "lcle");

#ifdef I_MODE
  int dr = op_dr(op);

  if(CC_LE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  if(CC_LE(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


E50I(lclt)
{
  logop1(op, "lclt");

#ifdef I_MODE
  int dr = op_dr(op);

  if(CC_LT(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
#else
  if(CC_LT(cpu))
    S_A(cpu, 1);
  else
    S_A(cpu, 0);
#endif
}


#ifdef I_MODE
E50I(lheq)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);

  logop1oo(op, "lheq", dr, r);

  SET_CC(cpu, r);

  if(CC_EQ(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
}


E50I(lhne)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);

  logop1oo(op, "lhne", dr, r);

  SET_CC(cpu, r);

  if(CC_NE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
}


E50I(lhge)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);

  logop1oo(op, "lhge", dr, r);

  SET_CC(cpu, r);

  if(CC_GE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
}


E50I(lhgt)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);

  logop1oo(op, "lhgt", dr, r);

  SET_CC(cpu, r);

  if(CC_GT(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
}


E50I(lhle)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);

  logop1oo(op, "lhle", dr, r);

  SET_CC(cpu, r);

  if(CC_LE(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
}


E50I(lhlt)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);

  logop1oo(op, "lhlt", dr, r);

  SET_CC(cpu, r);

  if(CC_LT(cpu))
    S_RH(cpu, dr, 1);
  else
    S_RH(cpu, dr, 0);
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
