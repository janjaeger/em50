/* Character Instructions
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

#include "char.h"

#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


static inline uint8_t E50X(load_byte)(cpu_t *cpu, uint32_t *far0, int *fbr0, int *flr0)
{
uint16_t w = E50X(vfetch_w)(cpu, *far0);

uint8_t r;

  if(*fbr0)
  {
    r = w & 0xff;
    *fbr0 = 0;
  }
  else
  {
    r = w >> 8;
    *fbr0 = 8;
  }

  if(!(*fbr0))
    inc_d(far0);
  --(*flr0);

  return r;
}

static inline void E50X(store_byte)(cpu_t *cpu, uint8_t b, uint32_t *far1, int *fbr1)
{
uint16_t w = E50X(vfetch_w)(cpu, *far1);

  if(*fbr1)
  {
    w &= 0xff00;
    w |= b;
    *fbr1 = 0;
  }
  else
  {
    w &= 0x00ff;
    w |= b << 8;
    *fbr1 = 8;
  }

  E50X(vstore_w)(cpu, *far1, w);

  if(!(*fbr1))
    inc_d(far1);
}

#define ZED_CPC 0b00
static inline void E50X(zed_cpc)(cpu_t *cpu, uint8_t m, uint32_t *far0, int *fbr0, int *flr0, uint32_t *far1, int *fbr1)
{
uint8_t c = cpu->crs->km.ascii ? 040: 0240;
int count = *flr0 < m ? *flr0 : m;
int n = 0;

  for(; n < count; ++n)
    E50X(store_byte)(cpu, E50X(load_byte)(cpu, far0, fbr0, flr0), far1, fbr1);

  for(; n < m; ++n)
    E50X(store_byte)(cpu, c, far1, fbr1);
}

#define ZED_INL 0b01
static inline void E50X(zed_inl)(cpu_t *cpu, uint8_t m, uint32_t *far1, int *flr1)
{
  E50X(store_byte)(cpu, m, far1, flr1);
}

#define ZED_SKC 0b10
static inline void E50X(zed_skc)(cpu_t *cpu, uint8_t m, uint32_t *far0, int *fbr0, int *flr0)
{
int count = *flr0 < m ? *flr0 : m;
int n = 0;

  for(; n < count; ++n)
    E50X(load_byte)(cpu, far0, fbr0, flr0);
}

#define ZED_BLK 0b11
static inline void E50X(zed_blk)(cpu_t *cpu, uint8_t m, uint32_t *far1, int *fbr1)
{
uint8_t c = cpu->crs->km.ascii ? 040: 0240;

  for(int i = 0; i < m; ++i)
    E50X(store_byte)(cpu, c, far1, fbr1);
}


static inline void E50X(move)(cpu_t *cpu, uint32_t src, int srb, int srl, uint32_t dst, int dsb, int dsl)
{
  while(srl && dsl)
  {
  uint16_t w;
    
    w = E50X(vfetch_w)(cpu, src);

    if(dsb == 0 && srb == 0 && dsl > 1 && srl > 1)
    {
      E50X(vstore_w)(cpu, dst, w);
      inc_d(&src);
      inc_d(&dst);
      srl -= 2;
      dsl -= 2;
    }
    else if(dsb != 0 && srb != 0)
    {
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0xff00) | (w & 0x00ff);
      E50X(vstore_w)(cpu, dst, t);
      inc_d(&src); srb = 0;
      inc_d(&dst); dsb = 0;
      --srl;
      --dsl;
    }
    else if(dsb != 0 && srb == 0)
    {
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0xff00) | (w >> 8);
      E50X(vstore_w)(cpu, dst, t);
      srb = 8;
      inc_d(&dst); dsb = 0;
      --srl;
      --dsl;
    }
    else if(dsb == 0 && srb != 0)
    {
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0x00ff) | (w << 8);
      E50X(vstore_w)(cpu, dst, t);
      inc_d(&src); srb = 0;
      dsb = 8;
      --srl;
      --dsl;
    }
    else /* dsb == 0 && srb == 0 && srl == 1 */
    {
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0x00ff) | (w & 0xff00);
      E50X(vstore_w)(cpu, dst, t);
      srb = 8;
      dsb = 8;
      --srl;
      --dsl;
    }
  }

  uint16_t w = cpu->crs->km.ascii ? 020040: 0120240;

  while(dsl)
  {
    if(dsb == 0 && dsl > 1)
    {
      E50X(vstore_w)(cpu, dst, w);
      inc_d(&dst);
      dsl -= 2;
    }
    else if(dsb != 0)
    {
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0xff00) | (w & 0x00ff);
      E50X(vstore_w)(cpu, dst, t);
      inc_d(&dst); dsb = 0;
      --dsl;
    }
    else /* dsl == 1 */
    {
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0x00ff) | (w & 0xff00);
      E50X(vstore_w)(cpu, dst, t);
      dsb = 8;
      --dsl;
    }
  }

  S_FAR(cpu, 0, src);
  S_FBR(cpu, 0, srb);
  S_FLR(cpu, 0, srl);
  S_FAR(cpu, 1, dst);
  S_FBR(cpu, 1, dsb);
  S_FLR(cpu, 1, dsl);
}

static inline void E50X(compare)(cpu_t *cpu, uint32_t src, int srb, int srl, uint32_t dst, int dsb, int dsl)
{
  cpu->crs->km.eq = 1;
  cpu->crs->km.lt = 0;

  while(srl && dsl)
  {
  uint16_t t, w = E50X(vfetch_w)(cpu, src);

    if(dsb == 0 && srb == 0 && dsl > 1 && srl > 1)
    {
      t = E50X(vfetch_w)(cpu, dst);
      inc_d(&src);
      inc_d(&dst);
      srl -= 2;
      dsl -= 2;
    }
    else if(dsb != 0 && srb != 0)
    {
      w &= 0x00ff;
      t = E50X(vfetch_w)(cpu, dst);
      t &= 0x00ff;
      inc_d(&src); srb = 0;
      inc_d(&dst); dsb = 0;
      --srl;
      --dsl;
    }
    else if(dsb != 0 && srb == 0)
    {
      w >>= 8;
      t = E50X(vfetch_w)(cpu, dst);
      t &= 0x00ff;
      srb = 8;
      inc_d(&dst); dsb = 0;
      --srl;
      --dsl;
    }
    else if(dsb == 0 && srb != 0)
    {
      w &= 0x00ff;
      t = E50X(vfetch_w)(cpu, dst);
      t >>= 8;
      inc_d(&src); srb = 0;
      dsb = 8;
      --srl;
      --dsl;
    }
    else /* dsb == 0 && srb == 0 && srl == 1 */
    {
      w &= 0xff00;
      t = E50X(vfetch_w)(cpu, dst);
      t &= 0xff00;
      srb = 8;
      dsb = 8;
      --srl;
      --dsl;
    }

    if(w != t)
    {
      if((w & 0xff00) == (t & 0xff00))
      {
        S_FAR(cpu, 0, src-1);
        S_FBR(cpu, 0, srb ? 0 : 8);
        S_FLR(cpu, 0, srl+1);
        S_FAR(cpu, 1, dst-1);
        S_FBR(cpu, 1, dsb ? 0 : 8);
        S_FLR(cpu, 1, dsl+1);
      }
      else
      {
        S_FAR(cpu, 0, src-2);
        S_FLR(cpu, 0, srl+2);
        S_FAR(cpu, 1, dst-2);
        S_FLR(cpu, 1, dsl+2);
      }
      cpu->crs->km.eq = 0;
      cpu->crs->km.lt = w < t ? 1 : 0;
      return;
    }
  }

  while(dsl)
  {
    uint16_t t, w = cpu->crs->km.ascii ? 020040: 0120240;

//  S_FAR(cpu, 1, dst);
//  S_FBR(cpu, 1, dsb);
//  S_FLR(cpu, 1, dsl);

    if(dsb == 0 && dsl > 1)
    {
      t = E50X(vfetch_w)(cpu, dst);
      inc_d(&dst);
      dsl -= 2;
    }
    else if(dsb != 0)
    {
      w &= 0x00ff;
      t = E50X(vfetch_w)(cpu, dst);
      t &= 0x00ff;
      inc_d(&dst); dsb = 0;
      --dsl;
    }
    else /* dsb == 0 && dsl == 1 */
    {
      w &= 0xff00;
      t = E50X(vfetch_w)(cpu, dst);
      t &= 0xff00;
      dsb = 8;
      --dsl;
    }

    if(w != t)
    {
      if((w & 0xff00) == (t & 0xff00))
      {
        S_FAR(cpu, 1, dst-1);
        S_FBR(cpu, 1, dsb ? 0 : 8);
        S_FLR(cpu, 1, dsl+1);
      }
      else
      {
        S_FAR(cpu, 1, dst-2);
        S_FLR(cpu, 1, dsl+2);
      }
      cpu->crs->km.eq = 0;
      cpu->crs->km.lt = w < t ? 1 : 0;
      return;
    }
  }

  while(srl)
  {
    uint16_t w, t = cpu->crs->km.ascii ? 020040: 0120240;

//  S_FAR(cpu, 0, src);
//  S_FBR(cpu, 0, srb);
//  S_FLR(cpu, 0, srl);

    if(srb == 0 && srl > 1)
    {
      w = E50X(vfetch_w)(cpu, src);
      inc_d(&src);
      srl -= 2;
    }
    else if(srb != 0)
    {
      t &= 0x00ff;
      w = E50X(vfetch_w)(cpu, src);
      w &= 0x00ff;
      inc_d(&src); srb = 0;
      --srl;
    }
    else /* srb == 0 && srl == 1 */
    {
      t &= 0xff00;
      w = E50X(vfetch_w)(cpu, src);
      w &= 0xff00;
      srb = 8;
      --srl;
    }

    if(w != t)
    {
      if((w & 0xff00) == (t & 0xff00))
      {
        S_FAR(cpu, 0, src-1);
        S_FBR(cpu, 0, srb ? 0 : 8);
        S_FLR(cpu, 0, srl+1);
      }
      else
      {
        S_FAR(cpu, 0, src-2);
        S_FLR(cpu, 0, srl+2);
      }
      cpu->crs->km.eq = 0;
      cpu->crs->km.lt = w < t ? 1 : 0;
      return;
    }
  }

  S_FAR(cpu, 0, src);
  S_FBR(cpu, 0, srb);
  S_FLR(cpu, 0, srl);
  S_FAR(cpu, 1, dst);
  S_FBR(cpu, 1, dsb);
  S_FLR(cpu, 1, dsl);
}


static inline uint8_t E50X(trtab)(cpu_t *cpu, uint32_t trt, uint8_t c)
{
uint32_t x = intraseg_i(trt, (c >> 1));
uint16_t t = E50X(vfetch_w)(cpu, x);

  return (c & 1) ? t & 0xff : t >> 8;
}

static inline uint16_t E50X(trtab_w)(cpu_t *cpu, uint32_t trt, uint16_t w)
{
sw_t r = {.w = w};

  r.l = E50X(trtab)(cpu, trt, r.l);
  r.h = E50X(trtab)(cpu, trt, r.h);

  return r.w;
}

static inline uint16_t E50X(trtab_h)(cpu_t *cpu, uint32_t trt, uint16_t w)
{
sw_t r = {.w = w};

  r.h = E50X(trtab)(cpu, trt, r.h);

  return r.w;
}

static inline uint16_t E50X(trtab_l)(cpu_t *cpu, uint32_t trt, uint16_t w)
{
sw_t r = {.w = w};

  r.l = E50X(trtab)(cpu, trt, r.l);

  return r.w;
}

static inline void E50X(translate)(cpu_t *cpu, uint32_t src, int srb, int srl, uint32_t dst, int dsb, int dsl, uint32_t trt)
{
  while(srl && dsl)
  {
  uint16_t w;
    
    w = E50X(vfetch_w)(cpu, src);

    if(dsb == 0 && srb == 0 && srl > 1 && dsl > 1)
    {
      w = E50X(trtab_w)(cpu, trt, w);
      E50X(vstore_w)(cpu, dst, w);
      inc_d(&src);
      inc_d(&dst);
      srl -= 2;
      dsl -= 2;
    }
    else if(dsb != 0 && srb != 0)
    {
      w = E50X(trtab_l)(cpu, trt, w);
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0xff00) | (w & 0x00ff);
      E50X(vstore_w)(cpu, dst, t);
      inc_d(&src); srb = 0;
      inc_d(&dst); dsb = 0;
      --srl;
      --dsl;
    }
    else if(dsb != 0 && srb == 0)
    {
      w = E50X(trtab_h)(cpu, trt, w);
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0xff00) | (w >> 8);
      E50X(vstore_w)(cpu, dst, t);
      srb = 8;
      inc_d(&dst); dsb = 0;
      --srl;
      --dsl;
    }
    else if(dsb == 0 && srb != 0)
    {
      w = E50X(trtab_l)(cpu, trt, w);
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0x00ff) | (w << 8);
      E50X(vstore_w)(cpu, dst, t);
      inc_d(&src); srb = 0;
      dsb = 8;
      --srl;
      --dsl;
    }
    else /* dsb == 0 && srb == 0 && srl == 1 */
    {
      w = E50X(trtab_h)(cpu, trt, w);
      uint16_t t = E50X(vfetch_w)(cpu, dst);
      t = (t & 0x00ff) | (w & 0xff00);
      E50X(vstore_w)(cpu, dst, t);
      srb = 8;
      dsb = 8;
      --srl;
      --dsl;
    }
  }
  S_FAR(cpu, 0, src);
//S_FLR(cpu, 0, srl);
  S_FBR(cpu, 0, srb);
  S_FAR(cpu, 1, dst);
  S_FLR(cpu, 1, dsl);
  S_FBR(cpu, 1, dsb);
}


#if defined V_MODE || defined I_MODE
E50I(ldc)
{
int f = FAR(op);
uint32_t far = G_FAR(cpu, f);
int flr = G_FLR(cpu, f);
int bit = G_FBR(cpu, f);

  logop1o(op, "ldc", f);
  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));

  if(flr == 0)
  {
    cpu->crs->km.eq = 1;
    cpu->crs->km.lt = 0;
    return;
  }

  uint16_t c = E50X(vfetch_w)(cpu, far);

  if(bit != 0)
    c &= 0xff;
  else
    c >>= 8;

  logmsg("-> char %2.2x '%c'\n", c, isprint(c &0x7f) ? c & 0x7f : '.');

  --flr;
  S_FLR(cpu, f, flr);
  if(bit != 0)
  {
    S_FBR(cpu, f, 0);
    S_FAR(cpu, f, inc_d(&far));
  }
  else
    S_FBR(cpu, f, 8);

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
#ifdef I_MODE
  S_RH(cpu, op_dr(op), c);
#else
  S_A(cpu, c);
#endif
  cpu->crs->km.eq = 0;
  cpu->crs->km.lt = 0;
}
#endif


E50I(stc)
{
int f = FAR(op);
uint32_t far = G_FAR(cpu, f);
int flr = G_FLR(cpu, f);
int bit = G_FBR(cpu, f);

  logop1o(op, "stc", f);
  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));

  if(flr == 0)
  {
    cpu->crs->km.eq = 1;
    cpu->crs->km.lt = 0;
    return;
  }

  uint16_t c = E50X(vfetch_w)(cpu, far);

  if(bit != 0)
  {
    c &= 0xff00;
#ifdef I_MODE
    c |= G_RH(cpu, op_dr(op)) & 0xff;
#else
    c |= G_A(cpu) & 0xff;
#endif
  }
  else
  {
    c &= 0x00ff;
#ifdef I_MODE
    c |= G_RH(cpu, op_dr(op)) << 8;
#else
    c |= G_A(cpu) << 8;
#endif
  }
  E50X(vstore_w)(cpu, far, c);

  --flr;
  S_FLR(cpu, f, flr);
  if(bit != 0)
  {
    S_FBR(cpu, f, 0);
    S_FAR(cpu, f, inc_d(&far));
  }
  else
    S_FBR(cpu, f, 8);

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
  cpu->crs->km.eq = 0;
  cpu->crs->km.lt = 0;
}


E50I(zfil)
{
int f = FAR(op);
uint32_t far = G_FAR(cpu, f);
int flr = G_FLR(cpu, f);
int bit = G_FBR(cpu, f);
#ifdef I_MODE
uint16_t c = G_RH(cpu, 2) & 0xff;
#else
uint16_t c = G_A(cpu) & 0xff;
#endif

  logop1oo(op, "zfil", f, c);
  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));

  c |= c << 8;

  while(flr)
  {
    if(bit != 0)
    {
      uint16_t w = E50X(vfetch_w)(cpu, far);
      w &= 0xff00;
      w |= c & 0xff;
      E50X(vstore_w)(cpu, far, w);
      --flr;
      bit = 0;
      inc_d(&far);
    }
    else
    {
      if(flr > 1)
      {
        E50X(vstore_w)(cpu, far, c);
        flr -= 2;
        inc_d(&far);
      }
      else /* flr == 1 */
      {
        uint16_t w = E50X(vfetch_w)(cpu, far);
        w &= 0x00ff;
        w |= c & 0xff00;
        E50X(vstore_w)(cpu, far, w);
        --flr;
        bit = 8;
      }
    }
  }

  S_FLR(cpu, f, 0);
  S_FAR(cpu, f, far);
  S_FBR(cpu, f, bit);

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}


E50I(zmvd)
{
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int len = G_FLR(cpu, 1);

  logop1o(op, "zmvd", len);
  logmsg("-> %8.8x+%x %8.8x+%x %x\n", src, srb, dst, dsb, len);

  E50X(move)(cpu, src, srb, len, dst, dsb, len);
}


E50I(zmv)
{
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "zmv");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);

  if(dsl < srl)
    srl = dsl;

  E50X(move)(cpu, src, srb, srl, dst, dsb, dsl);
}


E50I(zcm)
{
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "zcm");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);

  E50X(compare)(cpu, src, srb, srl, dst, dsb, dsl);
}


E50I(ztrn)
{
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 1);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);
uint32_t trt = G_XB(cpu);

  logop1(op, "ztrn");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x %8.8x\n", src, srb, srl, dst, dsb, dsl, trt);

  E50X(translate)(cpu, src, srb, srl, dst, dsb, dsl, trt);
}


E50I(zed)
{
uint32_t spa = G_XB(cpu);
uint16_t sp;

uint32_t far0 = G_FAR(cpu, 0);
int fbr0 = G_FBR(cpu, 0);
int flr0 = G_FLR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int fbr1 = G_FBR(cpu, 1);

  logop1(op, "zed");

  do {
    sp = E50X(vfetch_w)(cpu, spa);
    inc_d(&spa);
    int op = (sp >> 8) & 0b11;
    uint8_t m = sp & 0377;

    logmsg("-> sp = %4.4x\n", sp);

    switch(op) {
      case ZED_CPC:
        E50X(zed_cpc)(cpu, m, &far0, &fbr0, &flr0, &far1, &fbr1);
        break;
      case ZED_INL:
        E50X(zed_inl)(cpu, m, &far1, &fbr1);
        break;
      case ZED_SKC:
        E50X(zed_skc)(cpu, m, &far0, &fbr0, &flr0);
        break;
      case ZED_BLK:
        E50X(zed_blk)(cpu, m, &far1, &fbr1);
        break;
      default:;
    }

    S_XB(cpu, spa);
    S_FAR(cpu, 0, far0);
    S_FBR(cpu, 0, fbr0);
    S_FLR(cpu, 0, flr0);
    S_FAR(cpu, 1, far1);
    S_FBR(cpu, 1, fbr1);

  } while(!(sp & 0x8000));
}


#ifndef EMDE
 #include __FILE__
#endif
