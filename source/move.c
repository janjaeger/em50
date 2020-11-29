/* Move Instructions
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

#include "prcex.h"


E50I(ldlr)
{
uint16_t offset = E50X(ea)(cpu, op);
  logop2oo(op, "*ldlr", offset, E50X(fetch_rs)(cpu, offset));

  if((offset & 0x4010))
    E50X(rxm_check)(cpu);

  S_L(cpu, E50X(fetch_rs)(cpu, offset));
}


E50I(stlr)
{
uint16_t offset = E50X(ea)(cpu, op);
  logop2oo(op, "*stlr", offset, G_L(cpu));

  if((offset & 0x4010))
    E50X(rxm_check)(cpu);

  E50X(store_rs)(cpu, offset, G_L(cpu));
}


#ifdef I_MODE
E50I(ldar)
{
uint16_t offset = E50X(ea)(cpu, op);
  logop2oo(op, "*ldar", offset, E50X(fetch_rs)(cpu, offset));

  if((offset & 0x4010))
    E50X(rxm_check)(cpu);

  S_R(cpu, op_dr(op), E50X(fetch_rs)(cpu, offset));
}
#endif


#ifdef I_MODE
E50I(star)
{
uint16_t offset = E50X(ea)(cpu, op);
  logop2oo(op, "*star", offset, G_R(cpu, op_dr(op)));

  if((offset & 0x4010))
    E50X(rxm_check)(cpu);

  E50X(store_rs)(cpu, offset, G_R(cpu, op_dr(op)));
}
#endif


E50I(lda)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "lda", ea);

#if defined S_MODE || defined R_MODE
  if(cpu->crs->km.dp)
    S_L(cpu, E50X(vfetch_d)(cpu, ea));
  else
#endif
    S_A(cpu, E50X(vfetch_w)(cpu, ea));
}


E50I(dld)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "dld", ea);

  S_L(cpu, E50X(vfetch_d)(cpu, ea));
}


E50I(ldl)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "ldl", ea);

  S_L(cpu, E50X(vfetch_d)(cpu, ea));
}


E50I(ldx)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "ldx", ea);

  S_X(cpu, E50X(vfetch_w)(cpu, ea));
}


E50I(ldy)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "ldy", ea);

  S_Y(cpu, E50X(vfetch_w)(cpu, ea));
}


E50I(sta)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "sta", ea);

#if defined S_MODE || defined R_MODE
  if(cpu->crs->km.dp)
    E50X(vstore_d)(cpu, ea, G_L(cpu));
  else
#endif
    E50X(vstore_w)(cpu, ea, G_A(cpu));
}


E50I(dst)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "dst", ea);

  E50X(vstore_d)(cpu, ea, G_L(cpu));
}


E50I(stx)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "stx", ea);

  E50X(vstore_w)(cpu, ea, G_X(cpu));
}


E50I(sty)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "sty", ea);

  E50X(vstore_w)(cpu, ea, G_Y(cpu));
}


E50I(stl)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "stl", ea);

  E50X(vstore_d)(cpu, ea, G_L(cpu));
}


E50I(csa)
{
uint16_t a = G_A(cpu);

  logop1(op, "csa");

  cpu->crs->km.cbit = a >> 15;

  S_A(cpu, a & 0x7fff);
}


E50I(ima)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t m = E50X(vfetch_w)(cpu, ea);

  logopxo(op, "ima", ea);

  E50X(vstore_w)(cpu, ea, G_A(cpu));
  S_A(cpu, m);
}


E50I(tax)
{
  logop1(op, "tax");

  S_X(cpu, G_A(cpu));
}


E50I(tay)
{
  logop1(op, "tay");

  S_Y(cpu, G_A(cpu));
}


E50I(txa)
{
  logop1(op, "txa");

  S_A(cpu, G_X(cpu));
}


E50I(tya)
{
  logop1(op, "tya");

  S_A(cpu, G_Y(cpu));
}


E50I(xca)
{
  logop1(op, "xca");

  S_B(cpu, G_A(cpu));
  S_A(cpu, 0);
}


E50I(xcb)
{
  logop1(op, "xcb");

  S_A(cpu, G_B(cpu));
  S_B(cpu, 0);
}


E50I(iab)
{
uint16_t a = G_A(cpu);

  logop1(op, "iab");

  S_A(cpu, G_B(cpu));
  S_B(cpu, a);
}


E50I(ica)
{
uint16_t a = G_A(cpu);

  logop1(op, "ica");

  S_A(cpu, (a >> 8) | (a << 8));
}


E50I(icl)
{
uint16_t a = G_A(cpu);

  logop1(op, "icl");

  S_A(cpu, a >> 8);
}


E50I(icr)
{
  logop1(op, "icr");

  S_A(cpu, G_A(cpu) << 8);
}


E50I(ile)
{
uint32_t e = G_E(cpu);

  logop1(op, "ile");

  S_E(cpu, G_L(cpu));
  S_L(cpu, e);
}


E50I(tab)
{
  logop1(op, "tab");

  S_B(cpu, G_A(cpu));
}


E50I(tba)
{
  logop1(op, "tba");

  S_A(cpu, G_B(cpu));
}


E50I(stac)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);
uint16_t b = G_B(cpu);
uint16_t s = E50X(vfetch_w)(cpu, ap);

  logop1o(op, "stac", ap);

  if(b == s)
  {
    uint16_t a = G_A(cpu);
    E50X(vstore_w)(cpu, ap, a);
    cpu->crs->km.eq = 1;
  }
  else
    cpu->crs->km.eq = 0;
  cpu->crs->km.lt = 0;
}


E50I(stlc)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);
uint32_t e = G_E(cpu);
uint32_t s = E50X(vfetch_d)(cpu, ap);

  logop1o(op, "stlc", ap);

  if(e == s)
  {
    uint32_t l = G_L(cpu);
    E50X(vstore_d)(cpu, ap, l);
    cpu->crs->km.eq = 1;
  }
  else
    cpu->crs->km.eq = 0;
  cpu->crs->km.lt = 0;
}


#ifdef I_MODE
E50I(stch)
{
int dr = op_dr(op);
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);
uint16_t rl = G_R(cpu, dr) & 0xffff;
uint16_t s = E50X(vfetch_w)(cpu, ap);

  logop1o(op, "stch", ap);

  if(rl == s)
  {
    E50X(vstore_w)(cpu, ap, G_RH(cpu, dr));
    cpu->crs->km.eq = 1;
  }
  else
    cpu->crs->km.eq = 0;
  cpu->crs->km.lt = 0;
}
#endif


#ifdef I_MODE
E50I(stcd)
{
int dr = op_dr(op) & 0b110;
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);
uint32_t r1 = G_R(cpu, dr + 1);
uint32_t s = E50X(vfetch_d)(cpu, ap);

  logop1o(op, "stcd", ap);

  if(r1 == s)
  {
    E50X(vstore_d)(cpu, ap, G_R(cpu, dr));
    cpu->crs->km.eq = 1;
  }
  else
    cpu->crs->km.eq = 0;
  cpu->crs->km.lt = 0;
}
#endif


#ifdef I_MODE
E50I(csr)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);

  logop2o(op, "csr", dr);

  cpu->crs->km.cbit = r >> 31;
  
  S_R(cpu, dr, r & 0x7fffffff);
}
#endif


#ifdef I_MODE
E50I(i)
{
int dr = op_dr(op);
uint32_t r = E50X(efetch_d)(cpu, op);

  logop2oo(op, "i", dr, r);

  E50X(estore_dx)(cpu, op, G_R(cpu, dr), 0);
  S_R(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(ih)
{
int dr = op_dr(op);
uint16_t r = E50X(efetch_w)(cpu, op);

  logop2oo(op, "ih", dr, r);

  E50X(estore_wx)(cpu, op, G_RH(cpu, dr), 0);
  S_RH(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(l)
{
int dr = op_dr(op);
uint32_t r = E50X(efetch_d)(cpu, op);

  logop2oo(op, "l", dr, r);

  S_R(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(lh)
{
int dr = op_dr(op);
uint16_t r = E50X(efetch_w)(cpu, op);

  logop2oo(op, "lh", dr, r);

  S_RH(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(lhl1)
{
int dr = op_dr(op);
uint16_t r = E50X(efetch_w)(cpu, op);

  logop2oo(op, "lhl1", dr, r);

  S_RH(cpu, dr, r << 1);
}
#endif


#ifdef I_MODE
E50I(lhl2)
{
int dr = op_dr(op);
uint16_t r = E50X(efetch_w)(cpu, op);

  logop2oo(op, "lhl2", dr, r);

  S_RH(cpu, dr, r << 2);
}
#endif


#ifdef I_MODE
E50I(lhl3)
{
int dr = op_dr(op);
uint16_t r = E50X(efetch_w)(cpu, op);

  logop2oo(op, "lhl3", dr, r);

  S_RH(cpu, dr, r << 3);
}
#endif


#ifdef I_MODE
E50I(icbl)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);

  logop1oo(op, "icbl", dr, r);

  r >>= 8;

  S_RH(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(icbr)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);

  logop1oo(op, "icbr", dr, r);

  r <<= 8;

  S_RH(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(ichl)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);

  logop1oo(op, "ichl", dr, r);

  r >>= 16;

  S_R(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(ichr)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);

  logop1oo(op, "ichr", dr, r);

  r <<= 16;

  S_R(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(irb)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);

  logop1oo(op, "irb", dr, r);

  r = r << 8 | r >> 8;

  S_RH(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(irh)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);

  logop1oo(op, "irh", dr, r);

  r = r << 16 | r >> 16;

  S_R(cpu, dr, r);
}
#endif


#ifdef I_MODE
E50I(st)
{
int dr = op_dr(op);
uint32_t v = G_R(cpu, dr);
uint32_t ea = E50X(ea)(cpu, op);

  logop2o3(op, "st", dr, v, ea);

  E50X(vstore_d)(cpu, ea, v);
}
#endif


#ifdef I_MODE
E50I(sth)
{
int dr = op_dr(op);
uint16_t v = G_RH(cpu, dr);
uint32_t ea = E50X(ea)(cpu, op);

  logop2o3(op, "sth", dr, v & 0xffff, ea);

  E50X(vstore_w)(cpu, ea, v);
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
