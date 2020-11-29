/* Machine Control Instructions
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

#include "io.h"


E50I(lpsw)
{
  uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop1o(op, "lpsw", ap);

  E50X(rxm_check)(cpu);

  uint32_t pb = E50X(vfetch_d)(cpu, ap);
  km_t km = { .d = E50X(vfetch_d)(cpu, intraseg_i(ap, 2)) };

  S_RB(cpu, pb);

  if((cpu->crs->km.sm ^ km.sm))
    mm_ptlb(cpu);

  set_crs(cpu, km.crs);

  cpu->crs->km = km;

  if(km.in)
    E50X(pxm_disp)(cpu);

  km = cpu->crs->km;
  logmsg("-> pc %06o pb %o cbit %d link %d mode %d fex %d iex %d lt %d eq %d dex %d\n",
    cpu->p, G_PB(cpu) >> 16, km.cbit, km.link, km.mode, km.fex, km.iex, km.lt, km.eq, km.dex);
  logmsg("->           in %d sd %d ie %d vim %d crs %d mio %d pxm %d sm %d mcm %d\n",
    km.in, km.sd, km.ie, km.vim, km.crs, km.mio, km.pxm, km.sm, km.mcm);

  set_cpu_mode(cpu, km.mode);
}


E50I(rts)
{
int16_t a = G_A(cpu);
  logop1o(op, "*rts", a);

  E50X(rxm_check)(cpu);

  uint32_t owner = cpu->crs->owner;

  if(cpu->crs->km.pxm && owner)
  {
    int32_t timer = E50X(timer_get)(cpu);

    uint32_t etimer = E50X(vfetch_dp)(cpu, owner + offsetin(pcb_t, etimer));

    int16_t itimerh = timer >> 16;

    etimer += (uint32_t)itimerh - (uint32_t)a;
    itimerh = a;

    E50X(vstore_dp)(cpu, owner + offsetin(pcb_t, etimer), etimer);

    E50X(timer_set)(cpu, (itimerh << 16) | (timer & 0x0000ffff));
  }
}


E50I(sttm)
{
uint32_t ea = G_XB(cpu);

  logop1o(op, "*sttm", ea);

  uint32_t owner = cpu->crs->owner;

  int32_t timer = E50X(timer_get)(cpu);

  E50X(vstore_d)(cpu, intraseg_i(ea, 1), timer);
  
  if(cpu->crs->km.pxm && owner)
  {
    timer >>= 16;
    timer += (int32_t)E50X(vfetch_dp)(cpu, owner + offsetin(pcb_t, etimer));
    E50X(vstore_d)(cpu, ea, timer);
  }
  else
    E50X(vstore_w)(cpu, ea, 0);
}


E50I(liot)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);
uint32_t l = G_L(cpu);

  logop1oo(op, "*liot", ap, l);

  E50X(rxm_check)(cpu);

  mm_itlb(cpu, ap);

  if(!io_seg(l))
    mm_itlb(cpu, l);

  E50X(v2r)(cpu, ap, acc_io);
}


E50I(itlb)
{
uint32_t l = G_L(cpu);

  logop1o(op, "*itlb", l);

  E50X(rxm_check)(cpu);

  mm_itlb(cpu, l);
}


E50I(ptlb)
{
#ifdef DEBUG
uint32_t l = G_L(cpu);

  logop1o(op, "ptlb", l);
#endif

  E50X(rxm_check)(cpu);

  mm_ptlb(cpu); // FIXME TODO PARTIAL PURGE
}


#if !defined(MODEL)
 #define stpm_number   (cpu->model.number)
 #define stpm_ucodeman (cpu->model.ucodeman)
 #define stpm_ucodeeng (cpu->model.ucodeeng)
 #define stpm_ucodepln (cpu->model.ucodepln)
 #define stpm_ucodeext (cpu->model.ucodeext)
#else
 #define stpm_number   MODEL
 #if defined(MODEL_M)
  #define stpm_ucodeman MODEL_M
 #else 
  #define stpm_ucodeman (0)
 #endif
 #if defined(MODEL_E)
  #define stpm_ucodeeng MODEL_E
 #else 
  #define stpm_ucodeeng (0)
 #endif
 #if defined(MODEL_M)
  #define stpm_ucodepln MODEL_P
 #else 
   #define stpm_ucodepln (0)
 #endif
 #if defined(MODEL_M)
  #define stpm_ucodeext MODEL_X
 #else 
  #define stpm_ucodeext (0)
 #endif
#endif

E50I(stpm)
{
uint32_t ea = G_XB(cpu);

  logop1o(op, "stpm", ea);

  E50X(rxm_check)(cpu);

  E50X(vstore_d)(cpu, intraseg_i(ea, 000), stpm_number);
  E50X(vstore_w)(cpu, intraseg_i(ea, 002), stpm_ucodeman);
  E50X(vstore_w)(cpu, intraseg_i(ea, 003), stpm_ucodeeng);
  E50X(vstore_w)(cpu, intraseg_i(ea, 004), stpm_ucodepln);
  E50X(vstore_w)(cpu, intraseg_i(ea, 005), stpm_ucodeext);
  E50X(vstore_d)(cpu, intraseg_i(ea, 006), 0x00000000);
}


E50I(sssn)
{
uint32_t ea = G_XB(cpu);

  logop1o(op, "sssn", ea);

  E50X(vstore_d)(cpu, intraseg_i(ea, 000), from_be_32(cpu->sys->serd[0]));
  E50X(vstore_d)(cpu, intraseg_i(ea, 002), from_be_32(cpu->sys->serd[1]));
  E50X(vstore_d)(cpu, intraseg_i(ea, 004), from_be_32(cpu->sys->serd[2]));
  E50X(vstore_d)(cpu, intraseg_i(ea, 006), from_be_32(cpu->sys->serd[3]));
  E50X(vstore_q)(cpu, intraseg_i(ea, 010), 0);
  E50X(vstore_q)(cpu, intraseg_i(ea, 014), 0);
}


E50I(lpid)
{
#ifdef DEBUG
uint16_t a = G_A(cpu);

  logop1o(op, "*lpid", a);
#endif

  E50X(rxm_check)(cpu);
}


static inline void E50X(save_rf)(cpu_t *cpu, uint32_t *rsavptr, uint16_t reg)
{
  for(int n = 0; n < 040; ++n)
  {
// todo q endianswap for 8/9 10/11 
    rstore_d(cpu, *rsavptr, E50X(fetch_rs)(cpu, reg));
    reg += 1;
    (*rsavptr) += 2;
  }
}

E50I(hlt)
{
  logop1(op, "*hlt");

  E50X(rxm_check)(cpu);

  uint32_t rsavptr = E50X(fetch_rs)(cpu, 040037);

  cpu->srf.mrf.tr7 = cpu->pb;

  if(rsavptr)
  {
#if !defined(MODEL)
    if(cpu->model.nrf == 4)
 #define hlt_nrf 0
#else
 #define hlt_nrf em50_nrf
#endif
#if !defined(MODEL) || hlt_nrf == 4
    {
      E50X(save_rf)(cpu, &rsavptr, 040200); // URF3
      E50X(save_rf)(cpu, &rsavptr, 040240); // URF4
      E50X(save_rf)(cpu, &rsavptr, 040100); // URF1
      E50X(save_rf)(cpu, &rsavptr, 040140); // URF2
      E50X(save_rf)(cpu, &rsavptr, 040300); // MRF2
      E50X(save_rf)(cpu, &rsavptr, 040340); // SRF
      E50X(save_rf)(cpu, &rsavptr, 040000); // MRF
      E50X(save_rf)(cpu, &rsavptr, 040040); // DRF
    }
#endif
#if !defined(MODEL)
    else if(cpu->model.nrf == 8)
#endif
#if !defined(MODEL) || (hlt_nrf == 8)
    {
      E50X(save_rf)(cpu, &rsavptr, 040100); // URF1
      E50X(save_rf)(cpu, &rsavptr, 040140); // URF2
      E50X(save_rf)(cpu, &rsavptr, 040200); // URF3
      E50X(save_rf)(cpu, &rsavptr, 040240); // URF4
      E50X(save_rf)(cpu, &rsavptr, 040300); // URF5
      E50X(save_rf)(cpu, &rsavptr, 040340); // URF6
      E50X(save_rf)(cpu, &rsavptr, 040400); // URF7
      E50X(save_rf)(cpu, &rsavptr, 040440); // URF8
      E50X(save_rf)(cpu, &rsavptr, 040040); // DRF
      E50X(save_rf)(cpu, &rsavptr, 040000); // MRF
      E50X(save_rf)(cpu, &rsavptr, 040500);
    }
#endif
#if !defined(MODEL)
    else
#endif
#if !defined(MODEL) || (hlt_nrf == 2)
    {
      E50X(save_rf)(cpu, &rsavptr, 040140); // URF2
      E50X(save_rf)(cpu, &rsavptr, 040100); // URF1
      E50X(save_rf)(cpu, &rsavptr, 040040); // DRF
      E50X(save_rf)(cpu, &rsavptr, 040000); // MRF
    }
#endif
  }

  longjmp(cpu->smode, smode_halt);
}


static inline uint16_t E50X(rsave)(cpu_t *cpu, uint32_t a)
{
  uint16_t mask = 0;

  for(uint16_t r = 013, m = 0x0800; m; m >>= 1, r--)
  {
    int e; // endianess corrected r
    switch(r) {
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
        e = r ^ 1;
        break;
      default:
        e = r;
    }
logmsg("-> rsave m %4.4x r %d\n", m, r);
    if(cpu->crs->r[e])
    {
      uint32_t v = cpu->crs->r[e];
      mask |= m;
//if(r == 0x0b || r == 0x09) v = (v >> 16) | (v << 16);
      E50X(vstore_d)(cpu, a, v);
logmsg("-> rsave m %4.4x v %8.8x\n", m, v);
    }

    a = intraseg_i(a, 2);
  }
  E50X(vstore_d)(cpu, a, G_XB(cpu));

  return mask;
}

E50I(rsav)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop1o(op, "rsav", ap);

  uint16_t mask = E50X(rsave)(cpu, intraseg_i(ap, 1));
  
  E50X(vstore_w)(cpu, ap, mask);
logmsg("-> rsav mask %4.4x\n", mask);
}

static inline void E50X(rrest)(cpu_t *cpu, uint16_t mask, uint32_t a)
{
  for(uint16_t r = 013, m = 0x0800; m; m >>= 1, r--)
  {
    int e; // endianess corrected r
    switch(r) {
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
        e = r ^ 1;
        break;
      default:
        e = r;
    }
logmsg("-> rrst m %4.4x r %d\n", m, r);
    if((mask & m))
    {
      uint32_t v = E50X(vfetch_d)(cpu, a);
//if(r == 0x0b || r == 0x09) v = (v >> 16) | (v << 16);
      cpu->crs->r[e] = v;
logmsg("-> rrst m %4.4x v %8.8x\n", m, v);
    }
    else
    {
      cpu->crs->r[e] = 0;
logmsg("-> rrst m %4.4x v zero\n", m);
    }

    a = intraseg_i(a, 2);
  }
  S_XB(cpu, E50X(vfetch_d)(cpu, a));
}

E50I(rrst)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop1o(op, "rrst", ap);

  uint16_t mask = E50X(vfetch_w)(cpu, ap);
logmsg("-> rrst mask %4.4x\n", mask);

  E50X(rrest)(cpu, mask, intraseg_i(ap, 1));
}


#ifndef EMDE
 #include __FILE__
#endif
