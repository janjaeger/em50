/* Integer Instructions
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

#include "int.h"


E50I(dbl)
{
  logop1(op, "dbl");

  cpu->crs->km.dp = 1;
}


E50I(sgl)
{
  logop1(op, "sgl");

  cpu->crs->km.dp = 0;
}


E50I(sca)
{
  logop1(op, "sca");

#if 0
  S_A(cpu, cpu->crs->km.vsc);
#else
  S_A(cpu, G_FACE(cpu) & 0xff);
#endif
}


E50I(chs)
{
  logop1(op, "chs");
#if defined I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, G_RH(cpu, dr) ^ 0x8000);
#else
  S_A(cpu, G_A(cpu) ^ 0x8000);
#endif
}


/* A1A
 * Add 1 to A
 * 1100001010000110 (S, R, V mode form)
 */
#ifndef I_MODE
E50I(a1a)
{
int16_t a = (int16_t)G_A(cpu);
int16_t r;
int ovf, car;

  logop1(op, "a1a");

  r = add_w(a, 1, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;
 
  S_A(cpu, r);

  if(ovf)
  {
    SET_CC(cpu, a);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}
#endif


/* A2A
 * Add 2 to A
 * 1100000011000100 (S, R, V mode form)
 */
#ifndef I_MODE
E50I(a2a)
{
int16_t a = (int16_t)G_A(cpu);
int16_t r;
int ovf, car;

  logop1(op, "a2a");

  r = add_w(a, 2, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_A(cpu, r);

  if(ovf)
  {
    SET_CC(cpu, a);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}
#endif


/* ACA
 * Add CBIT to A
 * 1100001010001110 (S, R, V mode form)
 */
#ifndef I_MODE
E50I(aca)
{
int16_t a = (int16_t)G_A(cpu);
int16_t r;
int ovf, car;

  logop1(op, "aca");

  r = add_w(a, cpu->crs->km.cbit ? 1 : 0, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_A(cpu, r);

  if(ovf)
  {
    SET_CC(cpu, a);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}
#endif


/* ADD
 * Add
 * IX011011000Y00BR\2  (V mode long)
 * DISPLACEMENT\16
 * IX011011000000CB\2  (R mode long)
 * DISPLACEMENT\16
 * IX0110DISPLACEMENT\16 (S mode; R, V mode short)
 */
#ifndef I_MODE
E50I(add)
{
uint32_t ea = E50X(ea)(cpu, op);
int ovf, car;
#if defined S_MODE || defined R_MODE
  if(cpu->crs->km.dp)
  {
  int32_t s = (int32_t)E50X(vfetch_d)(cpu, ea);
  s = to31(s);
  int32_t l = (int32_t)G_L(cpu);
  uint32_t b1 = l & 0x00008000;
  l = to31(l);
  int32_t r;

    logopxoo(op, "dad", ea, (uint32_t)l);

    r = add_d31(l, s, &ovf, &car);

    cpu->crs->km.cbit = ovf;
    cpu->crs->km.link = car;

    r = from31(r);
    r |= b1;
    S_L(cpu, r);

    if(ovf)
    {
      SET_CC(cpu, l);
      E50X(int_ovf)(cpu);
    }
    else
      SET_CC(cpu, r);
  }
  else
#endif
  {
  int16_t s = (int16_t)E50X(vfetch_w)(cpu, ea);
  int16_t a = (int16_t)G_A(cpu);
  int16_t r;

    logopxoo(op, "add", ea, (uint16_t)s);

    r = add_w(a, s, &ovf, &car);

    cpu->crs->km.cbit = ovf;
    cpu->crs->km.link = car;

    S_A(cpu, r);

    if(ovf)
    {
      if(a != -32768 || s != -32768)
        SET_CC(cpu, a);
      else
        cpu->crs->km.eq = cpu->crs->km.lt = 1;
      E50X(int_ovf)(cpu);
    }
    else
      SET_CC(cpu, r);
  }
}
#endif


/* ADL
 * Add
 * IX01101000Y11BR\2  (V mode long)
 * DISPLACEMENT\16
 */
#ifdef V_MODE
E50I(adl)
{
uint32_t ea = E50X(ea)(cpu, op);
int32_t s = (int32_t)E50X(vfetch_d)(cpu, ea);
int32_t l = (int32_t)G_L(cpu);
int32_t r;
int ovf, car;

  logopxoo(op, "adl", ea, (uint32_t)s);

  r = add_d(l, s, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_L(cpu, r);

  if(ovf)
  {
      if(l != -2147483648 || s != -2147483648)
        SET_CC(cpu, l);
      else
        cpu->crs->km.eq = cpu->crs->km.lt = 1;
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}
#endif


/* ADLL
 * Add LINK to L
 * 1100001000000000 (V mode form)
 */
#ifdef V_MODE
E50I(adll)
{
int32_t l = (int32_t)G_L(cpu);
int32_t r;
int ovf, car;

  logop1(op, "adll");

  r = add_d(l, cpu->crs->km.link ? 1 : 0, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_L(cpu, r);

  if(ovf)
  {
    SET_CC(cpu, l);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}
#endif


E50I(s1a)
{
int16_t a = (int16_t)G_A(cpu);
int16_t r;
int ovf, car;

  logop1(op, "s1a");

  r = sub_w(a, 1, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_A(cpu, r);

  if(ovf)
  {
    SET_CC(cpu, a);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}


E50I(s2a)
{
int16_t a = (int16_t)G_A(cpu);
int16_t r;
int ovf, car;

  logop1(op, "s2a");

  r = sub_w(a, 2, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_A(cpu, r);

  if(ovf)
  {
    SET_CC(cpu, a);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}


E50I(tca)
{
int16_t a = (int16_t)G_A(cpu);
int16_t r;
int ovf, car;

  logop1(op, "tca");

  r = sub_w(0, a, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_A(cpu, r);

  if(ovf)
  {
    SET_CC(cpu, 1);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}


E50I(tcl)
{
int32_t l = (int32_t)G_L(cpu);
int32_t r;
int ovf, car;

  logop1(op, "tcl");

  r = sub_d(0, l, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_L(cpu, r);

  if(ovf)
  {
    SET_CC(cpu, 1);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}


E50I(ssm)
{
  logop1(op, "ssm");
#if defined I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, G_RH(cpu, dr) | 0x8000);
#else
  S_A(cpu, G_A(cpu) | 0x8000);
#endif
}


E50I(ssp)
{
  logop1(op, "ssp");
#if defined I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, G_RH(cpu, dr) & 0x7fff);
#else
  S_A(cpu, G_A(cpu) & 0x7fff);
#endif
}


E50I(sub)
{
uint32_t ea = E50X(ea)(cpu, op);
#if defined S_MODE || defined R_MODE
  if(cpu->crs->km.dp)
  {
  int32_t s = (int32_t)E50X(vfetch_d)(cpu, ea);
  s = to31(s);
  int32_t l = (int32_t)G_L(cpu);
  uint32_t b1 = l & 0x00008000;
  l = to31(l);
  int32_t r;
  int ovf, car;

    logopxoo(op, "dsb", ea, (uint32_t)l);

    r = sub_d31(l, s, &ovf, &car);

    cpu->crs->km.cbit = ovf;
    cpu->crs->km.link = car;

    r = from31(r);
    r |= b1;
    S_L(cpu, r);

    if(ovf)
    {
      SET_CC(cpu, -l);
      E50X(int_ovf)(cpu);
    }
    else
      SET_CC(cpu, r);
  }
  else
#endif
  {
  int16_t s = (int16_t)E50X(vfetch_w)(cpu, ea);
  int16_t a = (int16_t)G_A(cpu);
  int16_t r;
  int ovf, car;

    logopxoo(op, "sub", ea, (uint16_t)s);

    r = sub_w(a, s, &ovf, &car);

    cpu->crs->km.cbit = ovf;
    cpu->crs->km.link = car;

    S_A(cpu, r);

    if(ovf)
    {
      if(a == 0 && s == -32768)
        SET_CC(cpu, 1);
      else
        SET_CC(cpu, a);
      E50X(int_ovf)(cpu);
    }
    else
      SET_CC(cpu, r);
  }
}


E50I(sbl)
{
uint32_t ea = E50X(ea)(cpu, op);
int32_t s = (int32_t)E50X(vfetch_d)(cpu, ea);
int32_t l = (int32_t)G_L(cpu);
int32_t r;
int ovf, car;

  logopxoo(op, "sbl", ea, (uint32_t)s);

  r = sub_d(l, s, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_L(cpu, r);

  if(ovf)
  {
    if(l == 0 && s == -2147483648)
      SET_CC(cpu, 1);
    else
      SET_CC(cpu, l);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}


#ifndef I_MODE
E50I(div)  // TODO
{
uint32_t ea = E50X(ea)(cpu, op);
int16_t v = (int16_t)E50X(vfetch_w)(cpu, ea);
int32_t l = (int32_t)G_L(cpu);

  logopxoo(op, "div", ea, v & 0xffff);

  if(v == 0)
  {
    cpu->crs->km.cbit = 1;
    E50X(int_div0)(cpu);
    return;
  }

#if defined S_MODE || defined R_MODE
  l = to31(l);
#endif

  int32_t q = l / v;
  int32_t r = l % v;

  S_A(cpu, q);
  S_B(cpu, r);

  if(q < -32768 || q > 32767)
  {
    cpu->crs->km.cbit = 1;
    E50X(int_ovf)(cpu);
  }
  else
    cpu->crs->km.cbit = 0;
}
#endif


#ifdef V_MODE
E50I(dvl)
{
uint32_t ea = E50X(ea)(cpu, op);
int32_t v = (int32_t)E50X(vfetch_d)(cpu, ea);
uint32_t l = G_L(cpu);
uint32_t e = G_E(cpu);

int64_t d = (uint64_t)l << 32 | e;

  logopxoo(op, "dvl", ea, v);

  if(v == 0)
  {
    cpu->crs->km.cbit = 1;
    E50X(int_div0)(cpu);
    return;
  }

  int64_t q = d / v;
  int32_t r = d % v;

  S_L(cpu, q);
  S_E(cpu, r);

  if(q < -2147483648 || q > 2147483647)
  {
    cpu->crs->km.cbit = 1;
    E50X(int_ovf)(cpu);
  }
  else
    cpu->crs->km.cbit = 0;
}
#endif


E50I(mpl) 
{
uint32_t ea = E50X(ea)(cpu, op);
int32_t l = (int32_t)G_L(cpu);
int32_t v = (int32_t)E50X(vfetch_d)(cpu, ea);

  logopxoo(op, "mpl", ea, v);

  int64_t r = (int64_t)l * v;

  S_L(cpu, r >> 32);
  S_E(cpu, r & 0xffffffff);
  cpu->crs->km.cbit = 0;
}


E50I(mpy)
{
uint32_t ea = E50X(ea)(cpu, op);
int16_t a = (int16_t)G_A(cpu);
int16_t v = (int16_t)E50X(vfetch_w)(cpu, ea);

  logopxoo(op, "mpy", ea, v & 0xffff);

  int32_t l = (int32_t)a * v;

#if defined S_MODE || defined R_MODE
  if((uint16_t)a == 0x8000 && (uint16_t)v == 0x8000)
  {
    S_B(cpu, 0);
    cpu->crs->km.cbit = 1;
    E50X(int_ovf)(cpu);
    return;
  }

  l = from31(l); // TODO CHECK OVERFLOW
#endif

  S_L(cpu, l);
  cpu->crs->km.cbit = 0;
}


#ifndef I_MODE
E50I(pid)
{
uint16_t a = G_A(cpu);
uint16_t b = a & 0x7fff;

  logop1(op, "pid");

  if((a & 0x8000))
    a = 0xffff;
  else
    a = 0x0000;

  S_A(cpu, a);
  S_B(cpu, b);
}
#endif


E50I(pida)
{
uint16_t a = G_A(cpu);

  logop1(op, "pida");

  uint32_t l = a;
  if((a & 0x8000))
    l |= 0xffff0000;
  else
    l &= 0x0000ffff;

  S_L(cpu, l);
}


E50I(pidl)
{
uint32_t l = G_L(cpu);

  logop1(op, "pidl");

  S_E(cpu, l);

  if((l & 0x80000000))
    l |= 0x7fffffff;
  else
    l &= 0x00000000;

  S_L(cpu, l);
}

#ifndef I_MODE
E50I(pim)
{
uint16_t a = G_A(cpu);
uint16_t b = G_B(cpu);

  logop1(op, "pim");

  a &= 0x8000;

  S_A(cpu, a | b);
}
#endif


E50I(pima)
{
uint32_t l = G_L(cpu);

  logop1(op, "pima");

  int ovf = ((l & 0xffff8000) != 0 && (l & 0xffff8000) != 0xffff8000) ? 1 : 0;

  S_L(cpu, l >> 16 | l << 16);

  cpu->crs->km.cbit = ovf;

  if(ovf)
    E50X(int_ovf)(cpu);
}


E50I(piml)
{
int32_t e = G_E(cpu);
int32_t l = G_L(cpu);

  logop1(op, "piml");

  int ovf = ((l == 0 && (e & 0x80000000) == 0) || (l == -1 && (e & 0x80000000) != 0)) ? 0 : 1;

  S_L(cpu, e);

  cpu->crs->km.cbit = ovf;

  if(ovf)
    E50X(int_ovf)(cpu);
}


E50I(nrm)
{
uint32_t l = G_L(cpu);
int32_t  r = to31(l);
uint16_t s = 0;

  if(r)
    while((r & 0x40000000) == ((r & 0x20000000) << 1))
    {
      r <<= 1;
      ++s;
    }

  l = from31(r);

  S_L(cpu, l);
  S_VSC(cpu, s);
}


#ifdef I_MODE
E50I(a)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int32_t v = E50X(efetch_d)(cpu, op);
int ovf, car;

  logop2o3(op, "a", dr, r, v);

  int32_t d = add_d(r, v, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;
 
  S_R(cpu, dr, d);

  if(ovf)
  {
      if(r != -2147483648 || v != -2147483648)
        SET_CC(cpu, r);
      else
        cpu->crs->km.eq = cpu->crs->km.lt = 1;
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, d);
}
#endif


#ifdef I_MODE
E50I(adlr)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int ovf, car;

  logop2oo(op, "adlr", dr, r);

  int32_t d = add_d(r, cpu->crs->km.link ? 1 : 0, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;
 
  S_R(cpu, dr, d);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, d);
}
#endif


#ifdef I_MODE
E50I(ah)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int16_t v = E50X(efetch_w)(cpu, op);
int ovf, car;

  logop2o3(op, "ah", dr, r, v);

  int16_t h = add_w(r, v, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;
 
  S_RH(cpu, dr, h);

  if(ovf)
  {
    if(r != -32768 || v != -32768)
      SET_CC(cpu, r);
    else
      cpu->crs->km.eq = cpu->crs->km.lt = 1;
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, h);
}
#endif


#ifdef I_MODE
E50I(c)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int32_t v = E50X(efetch_d)(cpu, op);

  logop2oo(op, "c", r, v);

  int ovf, car;
  sub_d(r, v, &ovf, &car);

  cpu->crs->km.link = car;
 
  _SET_CC(cpu, r, v);
}
#endif


#ifdef I_MODE
E50I(ch)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int16_t v = E50X(efetch_w)(cpu, op);

  logop2oo(op, "ch", r, v);

  int ovf, car;
  sub_w(r, v, &ovf, &car);

  cpu->crs->km.link = car;
 
  _SET_CC(cpu, r, v);
}
#endif


#ifdef I_MODE
E50I(d)
{
int dr = op_dr(op) & 0b110;
uint32_t r0 = G_R(cpu, dr);
uint32_t r1 = G_R(cpu, (dr + 1) & 7);
int64_t d = (uint64_t)r0 << 32 | r1;
int32_t v = E50X(efetch_d)(cpu, op);

  logop2oo(op, "d", dr, v);

  if(v == 0)
  {
    cpu->crs->km.cbit = 1;
    E50X(int_div0)(cpu);
    return;
  }

  int64_t q = d / v;
  int32_t r = d % v;

  S_R(cpu, dr, q);
  S_R(cpu, (dr + 1) & 7, r);

  if(q < -2147483648 || q > 2147483647)
  {
    cpu->crs->km.cbit = 1;
    E50X(int_ovf)(cpu);
  }
  else
    cpu->crs->km.cbit = 0;
}
#endif


#ifdef I_MODE
E50I(dh)
{
int dr = op_dr(op);
int32_t d = G_R(cpu, dr);
int16_t v = E50X(efetch_w)(cpu, op);

  logop2oo(op, "dh", dr, v);

  if(v == 0)
  {
    cpu->crs->km.cbit = 1;
    E50X(int_div0)(cpu);
    return;
  }

  int32_t q = d / v;
  int16_t r = d % v;

  S_R(cpu, dr, ((int32_t)q << 16) | (r & 0xffff));

  if(q < -32768 || q > 32767)
  {
    cpu->crs->km.cbit = 1;
    E50X(int_ovf)(cpu);
  }
  else
    cpu->crs->km.cbit = 0;
}
#endif


#ifdef I_MODE
E50I(dh1)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int ovf, car;

  logop1oo(op, "dh1", dr, r);

  int16_t h = sub_w(r, 1, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_RH(cpu, dr, h);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, h);
}
#endif


#ifdef I_MODE
E50I(dh2)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int ovf, car;

  logop1oo(op, "dh2", dr, r);

  int16_t h = sub_w(r, 2, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_RH(cpu, dr, h);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, h);
}
#endif


#ifdef I_MODE
E50I(dr1)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int ovf, car;

  logop1oo(op, "dr1", dr, r);

  int32_t h = sub_d(r, 1, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_R(cpu, dr, h);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, h);
}
#endif


#ifdef I_MODE
E50I(dr2)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int ovf, car;

  logop1oo(op, "dr2", dr, r);

  int32_t h = sub_d(r, 2, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_R(cpu, dr, h);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, h);
}
#endif


#ifdef I_MODE
E50I(dm)
{
volatile int32_t r = E50X(efetch_d)(cpu, op); // GCC compiler bug, fix with volatile

  logop1o(op, "dm", r);

  E50X(estore_dx)(cpu, op, --r, 0);

//SET_CC(cpu, r);
  SET_CC(cpu, (uint32_t)r == 0x7fffffff ? -1 : r);
}
#endif


#ifdef I_MODE
E50I(dmh)
{
int16_t r = E50X(efetch_w)(cpu, op);

  logop1o(op, "dmh", r & 0xffff);

  E50X(estore_wx)(cpu, op, --r, 0);

//SET_CC(cpu, r);
  SET_CC(cpu, (uint16_t)r == 0x7fff ? -1 : r);
}
#endif


#ifdef I_MODE
E50I(ih1)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int ovf, car;

  logop1oo(op, "ih1", dr ,r);

  int16_t h = add_w(r, 1, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_RH(cpu, dr, h);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, h);
}
#endif


#ifdef I_MODE
E50I(ih2)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int ovf, car;

  logop1oo(op, "ih2", dr, r);

  int16_t h = add_w(r, 2, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_RH(cpu, dr, h);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, h);
}
#endif


#ifdef I_MODE
E50I(ir1)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int ovf, car;

  logop1oo(op, "ir1", dr, r);

  int32_t d = add_d(r, 1, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_R(cpu, dr, d);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, d);
}
#endif


#ifdef I_MODE
E50I(ir2)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int ovf, car;

  logop1oo(op, "ir2", dr, r);

  int32_t d = add_d(r, 2, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_R(cpu, dr, d);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, d);
}
#endif


#ifdef I_MODE
E50I(im)
{
volatile int32_t r = E50X(efetch_d)(cpu, op); // GCC compiler bug, fix with volatile

  logop1o(op, "im", r);

  E50X(estore_dx)(cpu, op, ++r, 0);

//SET_CC(cpu, r);
  SET_CC(cpu, (uint32_t)r == 0x80000000 ? 1 : r);
}
#endif


#ifdef I_MODE
E50I(imh)
{
int16_t r = E50X(efetch_w)(cpu, op);

  logop1o(op, "imh", r & 0xffff);

  E50X(estore_wx)(cpu, op, ++r, 0);

//SET_CC(cpu, r);
  SET_CC(cpu, (uint16_t)r == 0x8000 ? 1 : r);
}
#endif


#ifdef I_MODE
E50I(m)
{
int dr = op_dr(op); //  & 0b110;
int32_t r = G_R(cpu, dr);
int32_t v = E50X(efetch_d)(cpu, op);

  logop2o3(op, "m", dr, r, v);

  int64_t d = (int64_t)r * v;

  S_R(cpu, dr, d >> 32);
  S_R(cpu, (dr + 1) & 7, d & 0xffffffff);

  cpu->crs->km.cbit = 0;
}
#endif


#ifdef I_MODE
E50I(mh)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int16_t v = E50X(efetch_w)(cpu, op);

  logop2o3(op, "mh", dr, r, v);

  int32_t d = r * v;

  S_R(cpu, dr, d);

  cpu->crs->km.cbit = 0;
}
#endif


#ifdef I_MODE
E50I(pid)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);

  logop1(op, "pid");

  S_R(cpu, (dr + 1) & 7, r);

  if((r & 0x80000000))
    r |= 0x7fffffff;
  else
    r &= 0x00000000;

  S_R(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(pidh)
{
int dr = op_dr(op);
int16_t h = G_RH(cpu, dr);

  logop1(op, "pidh");

  int32_t r = h;

  S_R(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(pim)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int32_t r1 = G_R(cpu, (dr + 1) & 7);

  logop1(op, "pim");

  int ovf = ((r == 0 && (r1 & 0x80000000) == 0) || (r == -1 && (r1 & 0x80000000) != 0)) ? 0 : 1;

  S_R(cpu, dr, r1);

  cpu->crs->km.cbit = ovf;

  if(ovf)
    E50X(int_ovf)(cpu);
}
#endif


#ifdef I_MODE
E50I(pimh)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);

  logop1(op, "pimh");

  int ovf = ((r & 0xffff8000) != 0 && (r & 0xffff8000) != 0xffff8000) ? 1 : 0;

  S_R(cpu, dr, r >> 16 | r << 16);

  cpu->crs->km.cbit = ovf;

  if(ovf)
    E50X(int_ovf)(cpu);
}
#endif


#ifdef I_MODE
E50I(s)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int32_t v = E50X(efetch_d)(cpu, op);
int ovf, car;

  logop2o3(op, "s", dr, r, v);

  int32_t d = sub_d(r, v, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;
 
  S_R(cpu, dr, d);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, d);
}
#endif


#ifdef I_MODE
E50I(sh)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int16_t v = E50X(efetch_w)(cpu, op);
int ovf, car;

  logop2o3(op, "sh", dr, r, v);

  int16_t h = sub_w(r, v, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;
 
  S_RH(cpu, dr, h);

  if(ovf)
  {
    SET_CC(cpu, r);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, h);
}
#endif


#ifdef I_MODE
E50I(tc)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int ovf, car;

  logop1oo(op, "tc", dr, r);

  r = sub_d(0, r, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_R(cpu, dr, r);

  if(ovf)
  {
    SET_CC(cpu, 1);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}
#endif


#ifdef I_MODE
E50I(tch)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
int ovf, car;

  logop1oo(op, "tch", dr, r);

  r = sub_w(0, r, &ovf, &car);

  cpu->crs->km.cbit = ovf;
  cpu->crs->km.link = car;

  S_RH(cpu, dr, r);

  if(ovf)
  {
    SET_CC(cpu, 1);
    E50X(int_ovf)(cpu);
  }
  else
    SET_CC(cpu, r);
}
#endif


#ifdef I_MODE
E50I(tm)
{
int32_t r = E50X(efetch_d)(cpu, op);

  logop1o(op, "tm", r);

  SET_CC(cpu, r);
}
#endif


#ifdef I_MODE
E50I(tmh)
{
int16_t r = E50X(efetch_w)(cpu, op);

  logop1o(op, "tmh", r & 0xffff);

  SET_CC(cpu, r);
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
