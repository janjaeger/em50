/* Program Control and Jump Instructions
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


static inline uint32_t E50X(stack_alloc)(cpu_t *cpu, uint32_t s0, uint32_t sz)
{

  uint32_t sn = (E50X(vfetch_d)(cpu, s0) & ~ea_r) | (s0 & ea_r);

  if(((sn + sz) & ea_s) == (sn & ea_s))
    return sn;

  uint32_t s1 = sn & (ea_r | ea_s);

  uint32_t se = (E50X(vfetch_d)(cpu, s1 + 2) & ~ea_r) | (s0 & ea_r);

  if((se & ea_s) == 0)
    E50X(stack_fault)(cpu, s1);

  if(((se + sz) & ea_s) == (se & ea_s))
    return se;

  E50X(stack_fault)(cpu, s1);
}


E50I(cea)
{
  logop1(op, "cea");

#if defined E16S || defined E32S || defined E32R

  uint16_t a = G_A(cpu);

#if defined E16S
  if((a & 0x4000))
    a += G_X(cpu);
#endif

  if((a & 0x8000))
    a = E50X(vfetch_is)(cpu, a);

  S_A(cpu, a);
#endif
}


E50I(eaa)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "eaa", ea);

  S_A(cpu, ea);
}


E50I(eal)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "eal", ea);

  S_L(cpu, ea);
}


#ifdef I_MODE
E50I(ear)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "ear", ea);

  S_R(cpu, op_dr(op), ea);
}
#endif


E50I(ealb)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "ealb", ea);

  S_LB(cpu, ea);
}


E50I(eaxb)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "eaxb", ea);

  S_XB(cpu, ea);
}


E50I(jmp)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "jmp", ea);

#if defined V_MODE || defined I_MODE
  S_PB(cpu, ea);
#else
  cpu->p = ea;
#endif
}


E50I(jdx)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "jdx", ea);

  uint16_t x = G_X(cpu);

  S_X(cpu, --x);

  if(x != 0)
#if defined V_MODE || defined I_MODE
    S_PB(cpu, ea);
#else
    cpu->p = ea;
#endif
}


E50I(jix)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "jix", ea);

  uint16_t x = G_X(cpu);

  S_X(cpu, ++x);

  if(x != 0)
#if defined V_MODE || defined I_MODE
    S_PB(cpu, ea);
#else
    cpu->p = ea;
#endif
}


E50I(jst)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "jst", ea);

  E50X(vstore_w)(cpu, ea, cpu->p);
#if defined V_MODE || defined I_MODE
  ++ea;
  S_PB(cpu, ea);
#else
  cpu->p = ++ea;
#endif

  if(ea_ring(cpu->pb) == 0)
    longjmp(cpu->endop, endop_nointr5); // FIXME nointr1 in ring zero only
}


E50I(jsx)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "jsx", ea);

  S_X(cpu, cpu->p);
#if defined V_MODE || defined I_MODE
  S_PB(cpu, ea);
#else
  cpu->p = ea;
#endif
}


E50I(jsy)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "jsy", ea);

  S_Y(cpu, cpu->p);
#if defined V_MODE || defined I_MODE
  S_PB(cpu, ea);
#else
  cpu->p = ea;
#endif
}


E50I(jsxb)
{
uint32_t ea = E50X(ea)(cpu, op);

  logopxo(op, "jsxb", ea);

  S_XB(cpu, cpu->pb);
#if defined V_MODE || defined I_MODE
  S_PB(cpu, ea);
#else
  cpu->p = ea;
#endif
}


#ifdef I_MODE
E50I(jsr)
{
int dr = op_dr(op);
uint16_t ea = E50X(ea)(cpu, op);

  logop2oo(op, "jsr", dr, ea);

  S_RH(cpu, dr, cpu->p);
  cpu->p = ea;
}
#endif


E50I(jeq)
{
uint16_t ea = E50X(ea)(cpu, op);
int16_t a = (int16_t)G_A(cpu);

  logop2o(op, "jeq", ea);

  if(a == 0)
    cpu->p = ea;
}


E50I(jge)
{
uint16_t ea = E50X(ea)(cpu, op);
int16_t a = (int16_t)G_A(cpu);

  logop2o(op, "jge", ea);

  if(a >= 0)
    cpu->p = ea;
}


E50I(jgt)
{
uint16_t ea = E50X(ea)(cpu, op);
int16_t a = (int16_t)G_A(cpu);

  logop2o(op, "jgt", ea);

  if(a > 0)
    cpu->p = ea;
}


E50I(jle)
{
uint16_t ea = E50X(ea)(cpu, op);
int16_t a = (int16_t)G_A(cpu);

  logop2o(op, "jle", ea);

  if(a <= 0)
    cpu->p = ea;
}


E50I(jlt)
{
uint16_t ea = E50X(ea)(cpu, op);
int16_t a = (int16_t)G_A(cpu);

  logop2o(op, "jlt", ea);

  if(a < 0)
    cpu->p = ea;
}


E50I(jne)
{
uint16_t ea = E50X(ea)(cpu, op);
int16_t a = (int16_t)G_A(cpu);

  logop2o(op, "jne", ea);

  if(a != 0)
    cpu->p = ea;
}


E50I(pcl)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop2o(op, "*pcl", ea);

  uint32_t pb = E50X(vfetch_dx)(cpu, ea, acc_gt);
  if((pb & ea_f))
    E50X(pointer_fault)(cpu, pb >> 16, ea);
  uint16_t sfsize = E50X(vfetch_wx)(cpu, ea + 2, acc_gt);
  uint16_t rootsn = E50X(vfetch_wx)(cpu, ea + 3, acc_gt);
  uint16_t argsdisp = E50X(vfetch_wx)(cpu, ea + 4, acc_gt);
  int nargs = E50X(vfetch_wx)(cpu, ea + 5, acc_gt);
  uint32_t lb = E50X(vfetch_dx)(cpu, ea + 6, acc_gt);
  uint16_t keys = E50X(vfetch_wx)(cpu, ea + 8, acc_gt);

  logmsg("-> pb %8.8x sfsize %4.4x rootsn %4.4x argsdisp %4.4x nargs %4.4x lb %8.8x, keys %4.4x\n",
    pb, sfsize, rootsn, argsdisp, nargs, lb, keys);

  uint32_t sb = G_SB(cpu);
  uint32_t xb = G_XB(cpu);
  uint16_t x = G_X(cpu);
  uint32_t ao = cpu->pb;

#if 1 // TEMP FIX ARGT SWICHTING RING EARLY
{
  uint16_t sdw = E50X(fetch_sdw)(cpu, ea);
  int ring = ea_ring(cpu->pb);
  if(sdw_aaa(sdw, ring) == aaa_gate)
  {
    if((ea & 0xf))
      E50X(access_fault)(cpu, ea);
    cpu->pb &= ~ea_r;
    cpu->pb |= (pb & ea_r);;
    S_RB(cpu, cpu->pb);
  }
}
#endif

  uint32_t s0;
  if(rootsn != 0)
    s0 = (cpu->pb & ea_r) | (uint32_t)rootsn << 16;
  else
    s0 = ((cpu->pb | sb) & ea_r) | (uint32_t)E50X(vfetch_w)(cpu, sb + 1) << 16;

  uint32_t sn = E50X(stack_alloc)(cpu, s0, sfsize);

  E50X(vstore_d)(cpu, sn + 0, s0 >> 16); // clear flags and store rootsn
  E50X(vstore_d)(cpu, sn + 4, G_SB(cpu));
  E50X(vstore_d)(cpu, sn + 6, G_LB(cpu));
  E50X(vstore_w)(cpu, sn + 8, cpu->crs->km.keys);
  E50X(vstore_w)(cpu, sn + 9, cpu->p);

  if(nargs)
  {
    uint32_t at = sn + argsdisp;

    int bit, ind, br, l, s;
    uint32_t a;

    bit = 0;

    do
    {
      do 
      {
        uint32_t ca = E50X(vfetch_d)(cpu, ao); 
        bit += ca >> 28;
        ind = (ca >> 27) & 1;
        br = (ca >> 24) & 3;
        l = (ca >> 23) & 1;
        s = (ca >> 22) & 1;
        a = intraseg_i(br == 3 ? xb : G_ZB(cpu, br), ca & 0xffff) | (ao & ea_r);
        logmsg("-> pcl tmp %8.8x ap bit %d i %d br %d l %d s %d o %4.4x\n",
          ao, bit, ind, br, l, s, a);

        if(ind)
        {
          uint32_t d = E50X(vfetch_d)(cpu, a);
          logmsg("\n\n*** PCL INDIRECT %8.8x ***\n", a);

          if((d & ea_e))
          {
            bit = E50X(vfetch_w)(cpu, intraseg_i(a, 2)) >> 12;
          }

          if((d & ea_f) && (!s || (d & 0x7fff0000)))
            E50X(pointer_fault)(cpu, 0x8000, a | ea_e);

          if(!(d & ea_f))
            d |= (a & ea_r);

          a = d;
        }
        else if(br == 3) // XB
        {
          if((xb & ea_e))
          {
            bit += (x >> 12) & 0b1111;
          }
        }

        if(bit > 15)
        {
          a = intraseg_i(a, bit >> 4);
          bit &= 0b1111;
          a |= ea_e;
        }

        if(!s)
        {
          xb = a;
          if((xb & ea_e))
          {
            x = bit << 12;
            S_X(cpu, x);
          }
          S_XB(cpu, xb | (bit ? ea_e : 0));
        }

        logmsg("-> pcl %8.8x ap bit %d i %d br %d l %d s %d o %4.4x\n",
          ao, bit, ind, br, l, s, a);

        ao = intraseg_i(ao, 2);

      } while (!(l || s));

      if(s)
      {
        if(bit || (a & ea_e))
        {
          logmsg("-> arg %8.8x %8.8x%4.4x\n", at, a | ea_e, bit << 12);
          E50X(vstore_d)(cpu, at, a | ea_e);
          E50X(vstore_w)(cpu, intraseg_i(at, 2), bit << 12);
          bit = 0;
        }
        else
        {
          logmsg("-> pcl arg %8.8x %8.8x\n", at, a);
          E50X(vstore_d)(cpu, at, a);
        }
        at = intraseg_i(at, 3);
        --nargs;
      }

    } while (!l && nargs > 0);

    while(!l)
    {
      uint32_t ca = E50X(vfetch_d)(cpu, ao); 
      l = (ca >> 23) & 1;
      ao = intraseg_i(ao, 2);
    }

    while(nargs-- > 0)
    {
      logmsg("-> arg %8.8x fault set\n", at);
      E50X(vstore_d)(cpu, at, ea_f);
      at = intraseg_i(at, 3);
    }

  }

  E50X(vstore_d)(cpu, sn + 2, ao); // return addr

  E50X(vstore_d)(cpu, s0, (sn + sfsize));

  S_SB(cpu, sn);
  S_LB(cpu, lb);

  uint16_t sdw = E50X(fetch_sdw)(cpu, ea);
  int ring = ea_ring(ao);
logmsg("pcl ring %d access %d sdw %4.4x\n", ring, sdw_aaa(sdw, ring), sdw);
  if(sdw_aaa(sdw, ring) == aaa_gate)
    S_RB(cpu, pb);
  else
    S_PB(cpu, pb);

  S_KEYS(cpu, keys);
  set_cpu_mode(cpu, cpu->crs->km.mode);
}


/* ARGT
 * Argument Transfer
 * 0000000110000101 (V mode form)
 */
#if defined V_MODE || defined I_MODE
E50I(argt)
{
  logop1(op, "*argt");
}
#endif


E50I(prtn)
{
uint32_t sb = G_SB(cpu);

  logop1o(op, "*prtn", sb);

  uint32_t pc = E50X(vfetch_d)(cpu, sb + 2);
  uint32_t so = E50X(vfetch_d)(cpu, sb + 4);
  uint32_t lb = E50X(vfetch_d)(cpu, sb + 6);
  uint16_t keys = E50X(vfetch_w)(cpu, sb + 8);

  uint32_t s0 = E50X(vfetch_w)(cpu, sb + 1) << 16;
  E50X(vstore_d)(cpu, s0, sb);

  S_SB(cpu, so);
  S_LB(cpu, lb);

  S_PB(cpu, pc);

  S_KEYS(cpu, keys);
  cpu->crs->km.in = cpu->crs->km.sd = 0;
  set_cpu_mode(cpu, cpu->crs->km.mode);
}


E50I(stex)
{
uint32_t l = G_L(cpu);
uint32_t sb = G_SB(cpu);

  logop1(op, "*stex");

  if((l & 1))
    ++l;

  uint32_t s0 = (uint32_t)E50X(vfetch_w)(cpu, sb + 1) << 16;

  uint32_t sn = E50X(stack_alloc)(cpu, s0, l);

  E50X(vstore_d)(cpu, s0, sn + l);

  S_L(cpu, sn);
}


E50I(calf)
{
  uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop1o(op, "*calf", ap);
  // TODO

  uint32_t pb = E50X(vfetch_dx)(cpu, ap, acc_gt);
  if((pb & ea_f))
    E50X(pointer_fault)(cpu, pb >> 16, ap);
  uint16_t sfsize = E50X(vfetch_wx)(cpu, ap + 2, acc_gt);
  uint16_t rootsn = E50X(vfetch_wx)(cpu, ap + 3, acc_gt);
  uint16_t argsdisp = E50X(vfetch_wx)(cpu, ap + 4, acc_gt);
  int nargs = E50X(vfetch_wx)(cpu, ap + 5, acc_gt);
  uint32_t lb = E50X(vfetch_dx)(cpu, ap + 6, acc_gt);
  uint16_t keys = E50X(vfetch_wx)(cpu, ap + 8, acc_gt);

  logmsg("-> calf %8.8x sfsize %4.4x rootsn %4.4x lb %8.8x, keys %4.4x\n",
    pb, sfsize, rootsn, lb, keys);

  uint32_t sb = G_SB(cpu);

#if 1 // TEMP FIX ARGT SWICHTING RING EARLY
  uint32_t ao = cpu->pb;
{
  uint16_t sdw = E50X(fetch_sdw)(cpu, ap);
  int ring = ea_ring(cpu->pb);
  if(sdw_aaa(sdw, ring) == aaa_gate)
  {
    if((ap & 0xf))
      E50X(access_fault)(cpu, ap);
    cpu->pb &= ~ea_r;
    cpu->pb |= (pb & ea_r);;
    S_RB(cpu, cpu->pb);
  }
}
#endif

  uint32_t s0;
  if(rootsn != 0)
    s0 = (cpu->pb & ea_r) | (uint32_t)rootsn << 16;
  else
    s0 = (sb & ea_r) | (uint32_t)E50X(vfetch_w)(cpu, sb + 1) << 16;

  uint32_t sn = E50X(stack_alloc)(cpu, s0, sfsize);

  E50X(vstore_w)(cpu, sn + 0, 1); // CALF frame
  E50X(vstore_w)(cpu, sn + 1, s0 >> 16); // store rootsn
  E50X(vstore_d)(cpu, sn + 4, G_SB(cpu));
  E50X(vstore_d)(cpu, sn + 6, G_LB(cpu));
  E50X(vstore_w)(cpu, sn + 9, cpu->p);

  uint32_t at = sn + argsdisp;

  while(nargs-- > 0)
  {
      logmsg("-> calf arg %8.8x fault set\n", at);
      E50X(vstore_d)(cpu, at, ea_f);
      at += 3;
  }
    
  uint32_t csf = E50X(csf_pop)(cpu);

  E50X(vstore_d)(cpu, sn + 2, E50X(vfetch_dp)(cpu, csf + offsetin(csf_t, pc)));
  E50X(vstore_w)(cpu, sn + 8, E50X(vfetch_wp)(cpu, csf + offsetin(csf_t, keys)));
  E50X(vstore_w)(cpu, sn + 10, E50X(vfetch_wp)(cpu, csf + offsetin(csf_t, fcode)));
  E50X(vstore_d)(cpu, sn + 11, E50X(vfetch_dp)(cpu, csf + offsetin(csf_t, faddr)));

  E50X(vstore_d)(cpu, s0, sn + sfsize);

  S_SB(cpu, sn);
  S_LB(cpu, lb);

  uint16_t sdw = E50X(fetch_sdw)(cpu, ap);
  int ring = ea_ring(ao);
logmsg("calf ring %d access %d sdw %4.4x\n", ring, sdw_aaa(sdw, ring), sdw);
  if(sdw_aaa(sdw, ring) == aaa_gate)
    S_RB(cpu, pb);
  else
    S_PB(cpu, pb);

  S_KEYS(cpu, keys);
  set_cpu_mode(cpu, cpu->crs->km.mode);
}

 
E50I(svc)
{
  logop1(op, "*svc");

  E50X(svc_fault)(cpu);
}

 
E50I(xec)
{
uint32_t ea = E50X(ea)(cpu, op);
  logop1(op, "xec");

  cpu->exec = ea;  // FIXME TODO CANNOT EXECUTE FROM LOC 0
  cpu->inst = E50X(vfetch_i)(cpu);
  E50X(decode)(cpu, cpu->op);
  cpu->exec = 0;

  logopr(cpu);
}


#ifdef R_MODE
E50I(crep)
{
uint32_t ea = E50X(ea)(cpu, op);
  logop1o(op, "*crep", ea);

  E50X(vstore_w)(cpu, G_SP(cpu) + 1, cpu->p);
  cpu->p = ea;
}


E50I(entr)
{
uint32_t ea = E50X(ea)(cpu, op);
  logop1o(op, "*entr", ea);

  uint16_t sp = G_SP(cpu);
  uint16_t np = sp - ea;
  E50X(vstore_w)(cpu, np, sp);
  S_SP(cpu, np);
}


E50I(rtn)
{
  logop1(op, "*rtn");

  uint16_t sp = E50X(vfetch_w)(cpu, G_SP(cpu));
  uint16_t p = E50X(vfetch_w)(cpu, sp + 1);

  if(!p)
    E50X(stack_fault)(cpu, 0);

  S_SP(cpu, sp);
  cpu->p = p;
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
