/* Floating Point Instructions
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

#include "flpt.h"

#include "int.h"


#if 0
#undef logall
#define logall(...) PRINTF(__VA_ARGS__)
#endif


#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


#if defined R_MODE || defined V_MODE || defined I_MODE
/* BFEQ
 * Branch on Floating Point Accumulator Equal to 0
 * 1100001110001010 ADDRESS\16 (V mode form)
 */
E50I(bfeq)
{
uint16_t ea = E50X(vfetch_w)(cpu, cpu->pb);

  logop2o(op, "*bfeq", ea);

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  double dac = to_dbl(G_DAC(cpu, f));

  SET_CC(cpu, dac);

  if(CC_EQ(cpu))
    cpu->p = ea;
  else
    ++cpu->p;
}


/* BFGE
 * 1100001110001101 ADDRESS\16 (V mode form)
 */
E50I(bfge)
{
uint16_t ea = E50X(vfetch_w)(cpu, cpu->pb);

  logop2o(op, "*bfge", ea);

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  double dac = to_dbl(G_DAC(cpu, f));

  SET_CC(cpu, dac);

  if(CC_GE(cpu))
    cpu->p = ea;
  else
    ++cpu->p;
}


/* BFGT
 * Branch on Floating Point Accumulator Greater Then 0
 * 1100001110001001 ADDRESS\16 (V mode form)
 */
E50I(bfgt)
{
uint16_t ea = E50X(vfetch_w)(cpu, cpu->pb);

  logop2o(op, "*bfgt", ea);

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  double dac = to_dbl(G_DAC(cpu, f));

  SET_CC(cpu, dac);

  if(CC_GT(cpu))
    cpu->p = ea;
  else
    ++cpu->p;
}


/* BFLE
 * Branch on Floating Point Accumulator Less Then or Equal to 0
 * 1100001110001000 ADDRESS\16 (V mode form)
 */
E50I(bfle)
{
uint16_t ea = E50X(vfetch_w)(cpu, cpu->pb);

  logop2o(op, "*bfle", ea);

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  double dac = to_dbl(G_DAC(cpu, f));

  SET_CC(cpu, dac);

  if(CC_LE(cpu))
    cpu->p = ea;
  else
    ++cpu->p;
}


/* BFLT
 * Branch on Floating Point Accumulator Less Then 0
 * 1100001110001100 ADDRESS\16 (V mode form)
 */
E50I(bflt)
{
uint16_t ea = E50X(vfetch_w)(cpu, cpu->pb);

  logop2o(op, "*bflt", ea);

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  double dac = to_dbl(G_DAC(cpu, f));

  SET_CC(cpu, dac);

  if(CC_LT(cpu))
    cpu->p = ea;
  else
    ++cpu->p;
}


/* BFNE
 * Branch on Floating Point Accumulator Not Equal to 0
 * 1100001110001011 ADDRESS\16 (V mode form)
 */
E50I(bfne)
{
uint16_t ea = E50X(vfetch_w)(cpu, cpu->pb);

  logop2o(op, "*bfne", ea);

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  double dac = to_dbl(G_DAC(cpu, f));

  SET_CC(cpu, dac);

  if(CC_NE(cpu))
    cpu->p = ea;
  else
    ++cpu->p;
}


E50I(lfeq)
{
  logop1(op, "*lfeq");

  double dac = to_dbl(G_DAC(cpu, 1));

  SET_CC(cpu, dac);

#ifdef I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, CC_EQ(cpu) ? 1 : 0);
#else
  S_A(cpu, CC_EQ(cpu) ? 1 : 0);
#endif
}


E50I(lfge)
{
  logop1(op, "*lfge");

  double dac = to_dbl(G_DAC(cpu, 1));

  SET_CC(cpu, dac);

#ifdef I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, CC_GE(cpu) ? 1 : 0);
#else
  S_A(cpu, CC_GE(cpu) ? 1 : 0);
#endif
}


E50I(lfgt)
{
  logop1(op, "*lfgt");

  double dac = to_dbl(G_DAC(cpu, 1));

  SET_CC(cpu, dac);

#ifdef I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, CC_GT(cpu) ? 1 : 0);
#else
  S_A(cpu, CC_GT(cpu) ? 1 : 0);
#endif
}


E50I(lfle)
{
  logop1(op, "*lfle");

  double dac = to_dbl(G_DAC(cpu, 1));

  SET_CC(cpu, dac);

#ifdef I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, CC_LE(cpu) ? 1 : 0);
#else
  S_A(cpu, CC_LE(cpu) ? 1 : 0);
#endif
}


E50I(lflt)
{
  logop1(op, "*lflt");

  double dac = to_dbl(G_DAC(cpu, 1));

  SET_CC(cpu, dac);

#ifdef I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, CC_LT(cpu) ? 1 : 0);
#else
  S_A(cpu, CC_LT(cpu) ? 1 : 0);
#endif
}


E50I(lfne)
{
  logop1(op, "*lfne");

  double dac = to_dbl(G_DAC(cpu, 1));

  SET_CC(cpu, dac);

#ifdef I_MODE
  int dr = op_dr(op);
  S_RH(cpu, dr, CC_NE(cpu) ? 1 : 0);
#else
  S_A(cpu, CC_NE(cpu) ? 1 : 0);
#endif
}
#endif


#if defined V_MODE || defined R_MODE
E50I(fsgt)
{
  logop1(op, "*fsgt");

  double dac = to_dbl(G_DAC(cpu, 1));

  if(dac > 0)
    ++cpu->p;

  logdac(cpu, "fsgt", 1);
}


E50I(fsle)
{
  logop1(op, "*fsle");

  double dac = to_dbl(G_DAC(cpu, 1));

  if(dac <= 0)
    ++cpu->p;

  logdac(cpu, "fsle", 1);
}


E50I(fsmi)
{
  logop1(op, "*fsmi");

  double dac = to_dbl(G_DAC(cpu, 1));

  if(dac < 0)
    ++cpu->p;

  logdac(cpu, "fsmi", 1);
}


E50I(fsnz)
{
  logop1(op, "*fsnz");

  double dac = to_dbl(G_DAC(cpu, 1));

  if(dac != 0)
    ++cpu->p;

  logdac(cpu, "fsnz", 1);
}


E50I(fspl)
{
  logop1(op, "*fspl");

  double dac = to_dbl(G_DAC(cpu, 1));

  if(dac >= 0)
    ++cpu->p;

  logdac(cpu, "fspl", 1);
}


E50I(fsze)
{
  logop1(op, "*fsze");

  double dac = to_dbl(G_DAC(cpu, 1));

  if(dac == 0)
    ++cpu->p;

  logdac(cpu, "fsze", 1);
}
#endif


E50I(fld)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*fld", ea);

  uint32_t fac = E50X(vfetch_d)(cpu, ea);

  S_FAC(cpu, 1, fac);

  logdac(cpu, "fld", 1);
}


E50I(dfld)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "dfld", ea);

  uint64_t dac = E50X(vfetch_q)(cpu, ea);

  S_DAC(cpu, 1, dac);

  logdac(cpu, "dfld", 1);
}


E50I(fst)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*fst", ea);

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  uint32_t fac = G_FAC(cpu, f);

  E50X(vstore_d)(cpu, ea, fac);

  logdac(cpu, "fst", f);
}


E50I(dfst)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "dfst", ea);

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  uint64_t dac = G_DAC(cpu, f);

  E50X(vstore_q)(cpu, ea, dac);

  logdac(cpu, "dfst", f);
}


E50I(flx)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t s = E50X(vfetch_w)(cpu, ea);

  logop2o(op, "flx", ea);

  S_X(cpu, s << 1);
}


E50I(dflx)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t s = E50X(vfetch_w)(cpu, ea);

  logop2o(op, "dflx", ea);

  S_X(cpu, s << 2);
}


E50I(qflx)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t s = E50X(vfetch_w)(cpu, ea);

  logop2o(op, "qflx", ea);

  S_X(cpu, s << 3);
}


E50I(flta)
{
  logop1(op, "*flta");

  int16_t a = G_A(cpu);

  double d = a;

  S_DAC(cpu, 1, from_dbl_rnd(d));

  logdac(cpu, "flta", 1);
}


E50I(flot)
{
  logop1(op, "*flot");

  int32_t l = G_L(cpu);
  l = to31(l);

  double d = l;

  S_DAC(cpu, 1, from_dbl_rnd(d));

  logdac(cpu, "flot", 1);
}


E50I(fltl)
{
  logop1(op, "*fltl");

  int32_t l = G_L(cpu);

  double d = l;

  S_DAC(cpu, 1, from_dbl_rnd(d));

  logdac(cpu, "fltl", 1);
}


E50I(fdbl)
{
  logop1(op, "*fdbl");

  cpu->crs->fr[1].lh = 0;

  logdac(cpu, "fdbl", 1);
}


// fcqd / drnm
E50I(fcdq)
{
  logop1(op, "*fcdq");

  double dac = to_dbl(G_DAC(cpu, 1));
  
  if(dac == 0)
    S_DAC(cpu, 1, 0);

  S_DAC(cpu, 0, 0);

  logdac(cpu, "fcdq", 1);
}


#if !defined I_MODE
E50I(int)
{
  logop1(op, "*int");

  double dac = to_dbl(G_DAC(cpu, 1));

  int32_t l = dac;

  l = from31(l);
  S_L(cpu, l);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "int", 1);
}
#endif


E50I(inta)
{
  logop1(op, "*inta");

  double dac = to_dbl(G_DAC(cpu, 1));

  int16_t a = dac;

  S_A(cpu, a);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "inta", 1);
}


E50I(intl)
{
  logop1(op, "*intl");

  double dac = to_dbl(G_DAC(cpu, 1));

  int32_t l = dac;

  S_L(cpu, l);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "intl", 1);
}


E50I(fcm)
{
  logop1(op, "*fcm");

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  double dac = to_dbl(G_DAC(cpu, f));

  dac = -dac;

  S_DAC(cpu, f, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fcm", f);
}


E50I(dfcm)
{
  logop1(op, "*dfcm");

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

  double dac = to_dbl(G_DAC(cpu, f));

  dac = -dac;

  S_DAC(cpu, f, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfcm", f);
}


E50I(dfcs)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*dfcs", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = to_dbl(E50X(vfetch_q)(cpu, ea));

  _SET_CC(cpu, dac, d);

  if(dac == d)
    cpu->p++;
  else
    if(dac < d)
      cpu->p += 2;

  logdac(cpu, "dfcs", 1);
}


E50I(fcs)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*fcs", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = fto_dbl(E50X(vfetch_d)(cpu, ea));

  _SET_CC(cpu, dac, d);

  if(dac == d)
    cpu->p++;
  else
    if(dac < d)
      cpu->p += 2;
}


E50I(fdv)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*fdv", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = fto_dbl(E50X(vfetch_d)(cpu, ea));

logmsg("-> fdv %e * %e", dac, d);
  dac /= d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fdv", 1);
}


E50I(fmp)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*fmp", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = fto_dbl(E50X(vfetch_d)(cpu, ea));

logmsg("-> fmp %e * %e", dac, d);
  dac *= d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fmp", 1);
}


E50I(dfmp)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*dfmp", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = to_dbl(E50X(vfetch_q)(cpu, ea));

logmsg("-> dfmp %e * %e", dac, d);
  dac *= d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfmp", 1);
}


E50I(dfdv)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*dfdv", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = to_dbl(E50X(vfetch_q)(cpu, ea));

logmsg("-> dfdv %e / %e", dac, d);
  dac /= d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfdv", 1);
}


E50I(fad)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*fad", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = fto_dbl(E50X(vfetch_d)(cpu, ea));

logmsg("-> fad %e + %e", dac, d);
  dac += d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fad", 1);
}


E50I(dfad)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*dfad", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = to_dbl(E50X(vfetch_q)(cpu, ea));

logmsg("-> dfad %e + %e", dac, d);
  dac += d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfad", 1);
}


E50I(fsb)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*fsb", ea);

  double fac = fto_dbl(G_FAC(cpu, 1));

  double d = fto_dbl(E50X(vfetch_d)(cpu, ea));

logmsg("-> fsb %e - %e", fac, d);
  fac -= d;
logmsg(" = %e\n", fac);

  S_FAC(cpu, 1, ffrom_dbl_rnd(fac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fsb", 1);
}


E50I(dfsb)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*dfsb", ea);

  double dac = to_dbl(G_DAC(cpu, 1));

  double d = to_dbl(E50X(vfetch_q)(cpu, ea));

logmsg("-> dfsb %e - %e", dac, d);
  dac -= d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfsb", 1);
}


E50I(frn)
{
  logop1(op, "*frn");

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

em50_dbl dac = {.q = G_DAC(cpu, f)};

  dac.mantissa += 0x800000;
  dac.mantissa &= ~0xffffff;

  S_DAC(cpu, f, dac.q);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "frn", 1);
}


E50I(frnm)
{
  logop1(op, "*frnm");

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

em50_dbl dac = {.q = G_DAC(cpu, f)};

  dac.mantissa &= ~0xffffff;

  S_DAC(cpu, f, dac.q);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "frnm", 1);
}


E50I(frnp)
{
  logop1(op, "*frnp");

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

em50_dbl dac = {.q = G_DAC(cpu, f)};

  dac.mantissa += 0xffffff;
  dac.mantissa &= ~0xffffff;

  S_DAC(cpu, f, dac.q);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "frnp", 1);
}


E50I(frnz)
{
  logop1(op, "*frn");

#if defined I_MODE
  int f = FAC(op);
#else
  int f = 1;
#endif

em50_dbl dac = {.q = G_DAC(cpu, f)};

  if(dac.sign)
  {
    dac.mantissa += 0x1000000;
    dac.mantissa &= ~0xffffff;
  }

  S_DAC(cpu, f, dac.q);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "frn", 1);
}


#ifdef I_MODE
E50I(fl)
{
int f = FAC(op);

  logop2o(op, "*fl", f);

  uint32_t fac = E50X(efetch_d)(cpu, op);

  S_FAC(cpu, f, fac);

  logdac(cpu, "fl", 1);
}


E50I(dfl)
{
int f = FAC(op);

  logop2o(op, "*dfl", f);

  uint64_t dac = E50X(efetch_q)(cpu, op);

  S_DAC(cpu, f, dac);

  logdac(cpu, "dfl", f);
}


E50I(fa)
{
int f = FAC(op);

  logop2o(op, "*fa", f);

  double fac = to_dbl(G_DAC(cpu, f));

  fac += fto_dbl(E50X(efetch_d)(cpu, op));

  S_DAC(cpu, f, from_dbl_rnd(fac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fa", f);
}


E50I(dfa)
{
int f = FAC(op);

  logop2o(op, "*dfa", f);

  double dac = to_dbl(G_DAC(cpu, f));

  dac += to_dbl(E50X(efetch_q)(cpu, op));

  S_DAC(cpu, f, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfa", f);
}


E50I(fs)
{
int f = FAC(op);

  logop2o(op, "dfs", f);

  double fac = to_dbl(G_DAC(cpu, f));

  fac -= fto_dbl(E50X(efetch_d)(cpu, op));

  S_DAC(cpu, f, from_dbl_rnd(fac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fs", f);
}


E50I(dfs)
{
int f = FAC(op);

  logop2o(op, "*dfs", f);

  double dac = to_dbl(G_DAC(cpu, f));

  dac -= to_dbl(E50X(efetch_q)(cpu, op));

  S_DAC(cpu, f, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfs", f);
}


E50I(fm)
{
int f = FAC(op);

  logop2o(op, "*fm", f);

  double fac = to_dbl(G_DAC(cpu, f));

  fac *= fto_dbl(E50X(efetch_d)(cpu, op));

  S_DAC(cpu, f, from_dbl_rnd(fac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fm", f);
}


E50I(dfm)
{
int f = FAC(op);

  logop2o(op, "*dfm", f);

  double dac = to_dbl(G_DAC(cpu, f));

  dac *= to_dbl(E50X(efetch_q)(cpu, op));

  S_DAC(cpu, f, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfm", f);
}


E50I(fd)
{
int f = FAC(op);

  logop2o(op, "*fd", f);

  double fac = to_dbl(G_DAC(cpu, f));

  fac /= fto_dbl(E50X(efetch_d)(cpu, op));

  S_DAC(cpu, f, from_dbl_rnd(fac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "fd", f);
}


E50I(dfd)
{
int f = FAC(op);

  logop2o(op, "*dfd", f);

  double dac = to_dbl(G_DAC(cpu, f));

  dac /= to_dbl(E50X(efetch_q)(cpu, op));

  S_DAC(cpu, f, from_dbl_rnd(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "dfd", f);
}


E50I(fc)
{
int f = FAC(op);

  logop2o(op, "*fc", f);

  double fac = fto_dbl(G_FAC(cpu, f));

  double d = fto_dbl(E50X(efetch_d)(cpu, op));

  _SET_CC(cpu, fac, d);

  logdac(cpu, "fc", f);
}


E50I(dfc)
{
int f = FAC(op);

  logop2o(op, "*dfc", f);

  double dac = to_dbl(G_DAC(cpu, f));

  double d = to_dbl(E50X(efetch_q)(cpu, op));

  _SET_CC(cpu, dac, d);

  logdac(cpu, "dfc", f);
}


E50I(dble)
{
int f = FAC(op);

  logop1o(op, "*dble", f);

  cpu->crs->fr[f].lh = 0;

  logdac(cpu, "dble", f);
}


E50I(flt)
{
int dr = op_dr(op);
int f = FAR(op);

  logop1oo(op, "*flt", dr, f);

  int32_t a = G_R(cpu, dr);

  double d = a;

  S_DAC(cpu, f, from_dbl_rnd(d));

  logdac(cpu, "flt", f);
}


E50I(flth)
{
int dr = op_dr(op);
int f = FAR(op);

  logop1oo(op, "*flth", dr, f);

  int16_t r = G_RH(cpu, dr);

  double d = r;

  S_DAC(cpu, f, from_dbl_rnd(d));

  logdac(cpu, "flth", f);
}


E50I(int)
{
int dr = op_dr(op);
int f = FAR(op);

  logop1oo(op, "*int", dr, f);

  double d = to_dbl(G_DAC(cpu, f));

  int32_t r = d;

  S_R(cpu, dr, r);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "int", f);
}


E50I(inth)
{
int dr = op_dr(op);
int f = FAR(op);

  logop1oo(op, "*inth", dr, f);

  double d = to_dbl(G_DAC(cpu, f));

  int16_t r = d;

  S_RH(cpu, dr, r);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "inth", f);
}
#endif


static inline void E50X(_qfld)(cpu_t *cpu, op_t op, uint32_t ea)
{
  logop2o(op, "*qfld", ea);

  uint64_t dac1 = E50X(vfetch_q)(cpu, ea);
  uint64_t dac0 = E50X(vfetch_q)(cpu, ea + 4);

  S_DAC(cpu, 1, dac1);
  S_DAC(cpu, 0, dac0);

  logdac(cpu, "qfld", 1);
}

static inline void E50X(_qfst)(cpu_t *cpu, op_t op, uint32_t ea)
{
  logop2o(op, "*qfst", ea);

  uint64_t dac1 = G_DAC(cpu, 1);
  uint64_t dac0 = G_DAC(cpu, 0);

  E50X(vstore_q)(cpu, ea, dac1);
  E50X(vstore_q)(cpu, ea + 4, dac0);

  logdac(cpu, "qfst", 1);
}

static inline void E50X(_qfad)(cpu_t *cpu, op_t op, uint32_t ea)
{
  logop2o(op, "*qfad", ea);

  double dac = to_dbl(G_DAC(cpu, 1));
  ins_qex(&dac, G_DAC(cpu, 0));

  uint64_t d1 = E50X(vfetch_q)(cpu, ea);
  uint64_t d0 = E50X(vfetch_q)(cpu, ea + 4);
  double d = to_dbl(d1);
  ins_qex(&d, d0);

logmsg("-> qfad %e + %e", dac, d);
  dac += d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl(dac));
  S_DAC(cpu, 0, qex_dbl(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "qfad", 1);
}

static inline void E50X(_qfsb)(cpu_t *cpu, op_t op, uint32_t ea)
{
  logop2o(op, "*qfsb", ea);

  double dac = to_dbl(G_DAC(cpu, 1));
  ins_qex(&dac, G_DAC(cpu, 0));

  uint64_t d1 = E50X(vfetch_q)(cpu, ea);
  uint64_t d0 = E50X(vfetch_q)(cpu, ea + 4);
  double d = to_dbl(d1);
  ins_qex(&d, d0);

logmsg("-> qfsb %e - %e", dac, d);
  dac -= d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl(dac));
  S_DAC(cpu, 0, qex_dbl(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "qfsb", 1);
}

static inline void E50X(_qfmp)(cpu_t *cpu, op_t op, uint32_t ea)
{
  logop2o(op, "*qfmp", ea);

  double dac = to_dbl(G_DAC(cpu, 1));
  ins_qex(&dac, G_DAC(cpu, 0));

  uint64_t d1 = E50X(vfetch_q)(cpu, ea);
  uint64_t d0 = E50X(vfetch_q)(cpu, ea + 4);
  double d = to_dbl(d1);
  ins_qex(&d, d0);

logmsg("-> qfmp %e * %e", dac, d);
  dac *= d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl(dac));
  S_DAC(cpu, 0, qex_dbl(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "qfmp", 1);
}

static inline void E50X(_qfdv)(cpu_t *cpu, op_t op, uint32_t ea)
{
  logop2o(op, "*qfdv", ea);

  double dac = to_dbl(G_DAC(cpu, 1));
  ins_qex(&dac, G_DAC(cpu, 0));

  uint64_t d1 = E50X(vfetch_q)(cpu, ea);
  uint64_t d0 = E50X(vfetch_q)(cpu, ea + 4);
  double d = to_dbl(d1);
  ins_qex(&d, d0);

logmsg("-> qfdv %e / %e", dac, d);
  dac /= d;
logmsg(" = %e\n", dac);

  S_DAC(cpu, 1, from_dbl(dac));
  S_DAC(cpu, 0, qex_dbl(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "qfdv", 1);
}

static inline void E50X(_qfcs)(cpu_t *cpu, op_t op, uint32_t ea)
{
  logop2o(op, "*qfcs", ea);

  double dac = to_dbl(G_DAC(cpu, 1));
  ins_qex(&dac, G_DAC(cpu, 0));

  uint64_t d1 = E50X(vfetch_q)(cpu, ea);
  uint64_t d0 = E50X(vfetch_q)(cpu, ea + 4);
  double d = to_dbl(d1);
  ins_qex(&d, d0);

  _SET_CC(cpu, dac, d);

#if defined V_MODE
  if(dac == d)
    cpu->p++;
  else
    if(dac < d)
      cpu->p += 2;
#endif
}

#if defined V_MODE
E50I(qfxx)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t ext = E50X(vfetch_iw)(cpu); // BIG ENDIAN

  switch(ext) {
    case 0x0000:
      return E50X(_qfld)(cpu, op, ea);
    case 0x0100:
      return E50X(_qfst)(cpu, op, ea);
    case 0x0200:
      return E50X(_qfad)(cpu, op, ea);
    case 0x0300:
      return E50X(_qfsb)(cpu, op, ea);
    case 0x0400:
      return E50X(_qfmp)(cpu, op, ea);
    case 0x0500:
      return E50X(_qfdv)(cpu, op, ea);
    case 0x0600:
      return E50X(_qfcs)(cpu, op, ea);
    default:
      return E50X(uii_fault)(cpu, ea);
  }
}
#endif


#if defined I_MODE
E50I(qfld)
{
uint32_t ea = E50X(ea)(cpu, op);

  E50X(_qfld)(cpu, op, ea);
}


E50I(qfst)
{
uint32_t ea = E50X(ea)(cpu, op);

  E50X(_qfst)(cpu, op, ea);
}


E50I(qfad)
{
uint32_t ea = E50X(ea)(cpu, op);

  E50X(_qfad)(cpu, op, ea);
}


E50I(qfsb)
{
uint32_t ea = E50X(ea)(cpu, op);

  E50X(_qfsb)(cpu, op, ea);
}


E50I(qfmp)
{
uint32_t ea = E50X(ea)(cpu, op);

  E50X(_qfmp)(cpu, op, ea);
}


E50I(qfdv)
{
uint32_t ea = E50X(ea)(cpu, op);

  E50X(_qfdv)(cpu, op, ea);
}


E50I(qfc)
{
uint32_t ea = E50X(ea)(cpu, op);

  E50X(_qfcs)(cpu, op, ea);
}
#endif


E50I(qfcm)
{
  logop1(op, "*qfcm");

  double dac = to_dbl(G_DAC(cpu, 1));
  ins_qex(&dac, G_DAC(cpu, 0));

  dac = -dac;

  S_DAC(cpu, 1, from_dbl(dac));
  S_DAC(cpu, 0, qex_dbl(dac));

  cpu->crs->km.cbit = 0;

  logdac(cpu, "qfcm", 1);
}


E50I(qinq)
{
  logop1(op, "*qinq");

  em50_qad qad = {.dbl.q = G_DAC(cpu, 1), .qex.q = G_DAC(cpu, 0)};
  double dac = to_dbl(qad.dbl.q);
  ins_qex(&dac, qad.qex.q);

  if(0377 > (int16_t)qad.dbl.exponent)
  {
    if(0200 < (int16_t)qad.dbl.exponent)
    {
      modf(dac, &dac);
    }
    else if(0200 == (int16_t)qad.dbl.exponent)
    {
      if(qad.dbl.sign)
      {
        if(qad.dbl.mantissa || qad.qex.mantissa)
          dac = -1;
        else
          dac = 0;
      }
      else
        dac = 0;
    }
    else if(0200 > (int16_t)qad.dbl.exponent)
    {
      dac = 0;
    }

    S_DAC(cpu, 1, dac == 0 ? 0 : from_dbl(dac));
    S_DAC(cpu, 0, dac == 0 ? 0 : qex_dbl(dac));
  }

  cpu->crs->km.cbit = 0;

  logdac(cpu, "qinq", 1);
}


E50I(qiqr)
{
  logop1(op, "*qiqr");

  em50_qad qad = {.dbl.q = G_DAC(cpu, 1), .qex.q = G_DAC(cpu, 0)};
  double dac = to_dbl(qad.dbl.q);
  ins_qex(&dac, qad.qex.q);

  if(0337 > (int16_t)qad.dbl.exponent)
  {
    if(0177 < (int16_t)qad.dbl.exponent)
    {
      modf(dac + 0.5, &dac);
    }
    else if(0177 == (int16_t)qad.dbl.exponent)
    {
      if(qad.dbl.sign)
      {
        if(qad.dbl.mantissa || qad.qex.mantissa)
          dac = -1;
        else
          dac = 0;
      }
      else
        dac = 0;
    }
    else if(0177 > (int16_t)qad.dbl.exponent)
    {
      dac = 0;
    }

    S_DAC(cpu, 1, dac == 0 ? 0 : from_dbl(dac));
    S_DAC(cpu, 0, dac == 0 ? 0 : qex_dbl(dac));
  }

  cpu->crs->km.cbit = 0;

  logdac(cpu, "qiqr", 1);
}


E50I(drn)
{
  logop1(op, "*drn");

  em50_qad qad = {.dbl.q = G_DAC(cpu, 1), .qex.q = G_DAC(cpu, 0)};

  // TODO
  if((qad.qex.mantissa & 0x800000000000ULL))
    ++qad.dbl.mantissa;

  S_DAC(cpu, 1, qad.dbl.q);
  S_DAC(cpu, 0, 0);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "drn", 1);
}


E50I(drnz)
{
  logop1(op, "*drnz");

  em50_qad qad = {.dbl.q = G_DAC(cpu, 1), .qex.q = G_DAC(cpu, 0)};

  // TODO
  if(qad.dbl.mantissa)
    ++qad.dbl.mantissa;

  S_DAC(cpu, 1, qad.dbl.q);
  S_DAC(cpu, 0, 0);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "drnz", 1);
}


E50I(drnp)
{
  logop1(op, "*drnp");

  em50_qad qad = {.dbl.q = G_DAC(cpu, 1), .qex.q = G_DAC(cpu, 0)};

  // TODO
  if(qad.qex.mantissa && !(qad.qex.mantissa & 0x800000000000ULL))
    ++qad.dbl.mantissa;

  S_DAC(cpu, 1, qad.dbl.q);
  S_DAC(cpu, 0, 0);

  cpu->crs->km.cbit = 0;

  logdac(cpu, "drnp", 1);
}


#ifndef EMDE
 #include __FILE__
#endif
