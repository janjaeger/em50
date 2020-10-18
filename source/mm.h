/* Memmory Management
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


#include "faults.h"

#ifndef _mm_h
#define _mm_h

#if 0
 #define dbgmsg(...) logmsg(__VA_ARGS__)
#else
 #define dbgmsg(...)
#endif


#define w14(_x) ((_x) & 0x3fff)
#define w15(_x) ((_x) & 0x7fff)
#define w16(_x) ((_x) & 0xffff)

#define page_cross_x(_a, _n) (((_a) & em50_page_offm) > (em50_page_offm - (_n)))
#define page_cross_d(_a) page_cross_x(_a, 1)
#define page_cross_q(_a) page_cross_x(_a, 3)


typedef enum {
  acc_wr = 0,
  acc_rd = 1,
  acc_ex = 3,
  acc_rx = 4,
  acc_wx = 5,
  acc_gt = 6,
  acc_nn = 7,
  acc_io = 8
} acc_t;


static inline uint32_t intraseg_o(uint32_t a, uint16_t o)
{
#if 1
dw_t w;

  w.d = a;
  w.l = o;

  return w.d;
#else
  return (a & 0xffff0000) | (o & 0x0000ffff);
#endif
}


static inline uint32_t intraseg_i(uint32_t a, uint16_t o)
{
#if 1
dw_t w;

  w.d = a;
  w.l += o;

  return w.d;
#else
  return (a & 0xffff0000) | ((a + o) & 0x0000ffff);
#endif
}


static inline uint32_t inc_d(uint32_t *a)
{
dw_t *w = (dw_t *)a;
  ++(w->l);
  return w->d;
}


static inline phys_t sdt_addr(dtar_t dtar)
{
  return (((dtar & sdt_l) >> 1) | (dtar & sdt_l));
}


static inline uint32_t sdt_len(dtar_t dtar)
{
  return (dtar >> 22);
}


static inline void mm_itlb(cpu_t *cpu, uint32_t vaddr)
{
  uint32_t x = CACHE_INDEX(vaddr, acc_nn);

  cpu->tlb.v[x^0] = 0xffffffff;
  cpu->tlb.v[x^1] = 0xffffffff;

  if(vaddr < (IOTLB_SIZE << em50_page_shift))
  {
    uint32_t i = IOTLB_INDEX(vaddr);
    cpu->iotlb.i[i] = 0xffffffff;
  }
}


static inline uint16_t tfetch_w(cpu_t *cpu, uint16_t addr)
{
logmsg("\n@TFETCH %4.4x\n", addr);
//if(addr >= (cpu->crs->km.sm ? 010 : 040))
//  printf("BUG addr %4.4x\n", addr);

  addr &= 037;
  cpu->atr++;

  switch(addr) {
    case 0:   // X
      return G_X(cpu);
    case 1:   // A
      return G_A(cpu);
    case 2:   // B
      return G_B(cpu);
    case 3:   // S / Y
      return G_Y(cpu);
    case 4:   // FAC H
      return G_FACH(cpu);
    case 5:   // FAC L
      return G_FACL(cpu);
    case 6:   // FAC E
      return G_FACE(cpu);
    case 7:   // PC
      return cpu->p;
    case 010: // DTAR3 H
      return cpu->crs->dtar[3-3] >> 16;
    case 011: // FCODE H
      return cpu->crs->fcodeh;
    case 012: // FADDR L
      return cpu->crs->faddrl;
    case 013: // FAR0 H
      return cpu->crs->far0 >> 16;
    case 014: // SB H
      return cpu->crs->sb >> 16;
    case 015: // SB L
      return cpu->crs->sb & 0xffff;
    case 016: // LB H
      return cpu->crs->lb >> 16;
    case 017: // LB L
      return cpu->crs->lb & 0xffff;
    default:
      return cpu->srf.drf.dma_h[((addr & ~1) << 1) | ((addr & 1) ^ 1)];
  }
}


static inline void tstore_w(cpu_t *cpu, uint16_t addr, uint16_t val)
{
logmsg("\n@TSTORE %4.4x %4.4x\n", addr, val);
  addr &= 037;
  cpu->atr++;

  switch(addr) {
    case 0:
      S_X(cpu, val);
      return;
    case 1:
      S_A(cpu, val);
      return;
    case 2:
      S_B(cpu, val);
      return;
    case 3:
      S_Y(cpu, val);
      return;
    case 4:
      S_FACH(cpu, val);
      return;
    case 5:
      S_FACL(cpu, val);
      return;
    case 6:
      S_FACE(cpu, val);
      return;
    case 7:
      cpu->p = val;
      return;
    case 010:
      cpu->crs->dtar[3-3] = (val << 16) | (cpu->crs->dtar[3-3] & 0xffff);
      return;
    case 011:
      cpu->crs->fcodeh = val;
      return;
    case 012:
      cpu->crs->faddrl = val;
      return;
    case 013:
      cpu->crs->far0 = (val << 16) | (cpu->crs->far0 & 0xffff);
      return;
    case 014:
      cpu->crs->sb = (val << 16) | (cpu->crs->sb & 0xffff);
      return;
    case 015:
      cpu->crs->sb = (cpu->crs->sb & 0xffff0000) | val;
      return;
    case 016:
      cpu->crs->lb = (val << 16) | (cpu->crs->lb & 0xffff);
      return;
    case 017:
      cpu->crs->lb = (cpu->crs->lb & 0xffff0000) | val;
      return;
    default:
      cpu->srf.drf.dma_h[((addr & ~1) << 1) | ((addr & 1) ^ 1)] = val;
      return;
  }
}


static inline uint32_t tfetch_d(cpu_t *cpu, uint32_t addr)
{
dw_t w;

  w.h = tfetch_w(cpu, addr);
  w.l = tfetch_w(cpu, addr + 1);

  return w.d;
}


static inline uint64_t tfetch_q(cpu_t *cpu, uint32_t addr)
{
qw_t q;

  q.h = tfetch_d(cpu, addr);
  q.l = tfetch_d(cpu, addr + 2);

  return q.q;
}


static inline void tstore_d(cpu_t *cpu, uint32_t addr, uint32_t val)
{
dw_t w;

  w.d = val;

  tstore_w(cpu, addr, w.h);
  tstore_w(cpu, addr + 1, w.l);

  return;
}


static inline void tstore_q(cpu_t *cpu, uint32_t addr, uint64_t val)
{
qw_t q;

  q.q = val;

  tstore_d(cpu, addr, q.h);
  tstore_d(cpu, addr + 2, q.l);

  return;
}


static inline void rstore_w(cpu_t *cpu, uint32_t addr, uint16_t val)
{
if(addr < 0100) logmsg("\n@STORE %4.4x %4.4hx\n", addr, val);
  store_w(physad(cpu, addr), val);
}


static inline void rstore_d(cpu_t *cpu, uint32_t addr, uint32_t val)
{
if(addr < 0100) logmsg("\n@STORE %4.4x %8.8x\n", addr, val);
  store_d(physad(cpu, addr), val);
}


static inline void rstore_q(cpu_t *cpu, uint32_t addr, uint64_t val)
{
if(addr < 0100) logmsg("\n@STORE %4.4x %16.16jx\n", addr, (uintmax_t)val);
  store_q(physad(cpu, addr), val);
}


static inline uint16_t rfetch_w(cpu_t *cpu, uint32_t addr)
{
//return fetch_w(physad(cpu, addr));
  uint16_t w = fetch_w(physad(cpu, addr));
if(addr < 0100) logmsg("\n@FETCH %4.4x %4.4hx\n", addr, w);
  return w;
}


static inline uint32_t rfetch_d(cpu_t *cpu, uint32_t addr)
{
//return fetch_d(physad(cpu, addr));
  uint32_t d = fetch_d(physad(cpu, addr));
if(addr < 0100) logmsg("\n@FETCH %4.4x %8.8x\n", addr, d);
  return d;
}


static inline uint64_t rfetch_q(cpu_t *cpu, uint32_t addr)
{
//return fetch_q(physad(cpu, addr));
  uint64_t q = fetch_q(physad(cpu, addr));
if(addr < 0100) logmsg("\n@FETCH %4.4x %16.16jx\n", addr, (uintmax_t)q);
  return q;
}


#endif


#undef WXX
#if defined E16S
 #define WXX(_a) w14(_a)
#elif defined E32S || defined E32R
 #define WXX(_a) w15(_a)
#elif defined E64R
 #define WXX(_a) w16(_a)
#else
 #define WXX(_a) (_a)
#endif


#undef ATON
#undef ISAT
#undef ATOFF
#if defined E16S || defined E32S || defined E32R || defined E64R
 #define ATON(_c, _a)
 #define ISAT(_c, _a) (WXX(_a) < ((_c)->crs->km.sm ? 010 : 040))
 #define ATOFF(_c)
#elif defined E64V
 #define ATON(_c, _a) ((_c)->atr = ((((_a) & (ea_s|ea_w)) < ((_c)->crs->km.sm ? 010 : 040)) ? (((_a) & (ea_s|ea_w)) + 1) : 0))
 #define ATOFF(_c) ((_c)->atr = 0)
 #define ISAT(_c, _a) ((_c)->atr && (((_a) & (ea_s|ea_w)) < ((_c)->crs->km.sm ? 010 : 040)))
#elif defined E32I
 #define ATON(_c, _a)
 #define ISAT(_c, _a) (0)
 #define ATOFF(_c)
#else
 #error No mode defined
#endif


static inline uint32_t E50X(fetch_sdw)(cpu_t *cpu, uint32_t vaddr, void (*sfault)(cpu_t *, uint16_t, uint32_t))
{
  uint32_t d = ea_dtar(vaddr);
  uint32_t s = ea_segd(vaddr);
//if(ea_fault(vaddr)) PRINTF("bug3\n");

  if(s >= G_DTAR_L(cpu, d))
  {
logall("FAULT dtar %8.8x len %x addr %8.8x ie %d\n", G_DTAR_L(cpu, d), G_DTAR_A(cpu, d), vaddr, cpu->crs->km.ie);
    sfault(cpu, segment_fault_dtar, vaddr);
    return sdw_f;
  }

  dbgmsg("\naddr(%8.8x) dtar(%d) seg(%03x) page(%02x) offset(%03x)\n", vaddr, d, s, p, o);

  dbgmsg("dtar(%d) %8.8x len(%03x) page(%08x)\n", d, G_DTAR(cpu, d), G_DTAR_L(cpu, d), G_DTAR_A(cpu, d));

  uint32_t sdt = G_DTAR_A(cpu, d);

  dbgmsg("sdtl %d segno %d\n", G_DTAR_L(cpu, d), s);
  sdt += s << 1;

  uint32_t sdw = rfetch_d(cpu, sdt);

  if((sdw & sdw_f))
  {
logall("FAULT raddr %8.8x sdw %8.8x vaddr %8.8x ie %d\n", sdt, sdw, vaddr, cpu->crs->km.ie);
    sfault(cpu, segment_fault_sdw, vaddr);
  }

  return sdw;
}


static inline void E50X(acc_check)(cpu_t *cpu, uint32_t sdw, uint32_t vaddr, acc_t acc)
{
int ring = ea_ring(vaddr);

  if(ring == 0)
    return;

int access = sdw_aaa(sdw, ring);

  switch(acc) {
    case acc_wr:
      if(access != aaa_rdwr &&
         access != aaa_all)
         E50X(access_fault)(cpu, vaddr);
      break;

    case acc_rd:
      if(access != aaa_read &&
         access != aaa_rdwr &&
         access != aaa_rdex &&
         access != aaa_all)
         E50X(access_fault)(cpu, vaddr);
      break;

    case acc_ex:
      if(access != aaa_rdex &&
         access != aaa_all)
         E50X(access_fault)(cpu, vaddr);
      break;

    case acc_gt:
      if(access != aaa_gate &&
         access != aaa_read &&
         access != aaa_rdwr &&
         access != aaa_rdex &&
         access != aaa_all)
         E50X(access_fault)(cpu, vaddr);
      break;

    default:
      break;
  }
}


static inline int32_t E50X(xlatv2r)(cpu_t *cpu, uint32_t sdw, uint32_t vaddr, acc_t acc, void (*pfault)(cpu_t *, uint32_t))
{
  uint32_t p = ea_page(vaddr);
  uint32_t o = ea_off(vaddr);
  uint32_t h = sdw_a(sdw);

  dbgmsg("sdt(%8.8x) %8.8x sdw(%8.8x)\n", sdt, rfetch_d(cpu, sdt), h);

  dbgmsg("hmap(%4.4x) pmt(%8.8x)\n", rfetch_w(cpu, h + p), rfetch_d(cpu, h + (p << 1)));

  uint32_t r;
#if !defined(MODEL)
  if(cpu->model.have_pmt)
#endif
#if !defined(MODEL) || defined(em50_have_pmt)
  {
    uint32_t pmt = rfetch_d(cpu, h + (p << 1));
    if(!(pmt & pmt_r))
    {
logall("FAULT raddr %8.8x pmt %8.8x vaddr %8.8x ie %d\n", h + (p << 1), pmt, vaddr, cpu->crs->km.ie);
      if(pfault)
        pfault(cpu, vaddr);
      return -1;
    }
    pmt |= pmt_u;
    if(acc == acc_wr || acc == acc_wx)
      pmt &= ~pmt_m;
    rstore_d(cpu, h + (p << 1), pmt);
#if !defined(MODEL)
    if(cpu->model.have_pmt == pmtx)
#endif
#if !defined(MODEL) || defined(em50_have_pmtx)
      r = ((pmt & pmt_phyx) << 10) | o;
#endif
#if !defined(MODEL)
    else
#endif
#if !defined(MODEL) || !defined(em50_have_pmtx)
      r = ((pmt & pmt_phy) << 10) | o;
#endif
  }
#endif
#if !defined(MODEL)
  else
#endif
#if !defined(MODEL) || !defined(em50_have_pmt)
  {
    uint16_t hmap = rfetch_w(cpu, h + p);
    if(!(hmap & hmap_r))
    {
logall("FAULT raddr %8.8x hmap %4.4x vaddr %8.8x ie %d\n", h + p, hmap, vaddr, cpu->crs->km.ie);
      if(pfault)
        pfault(cpu, vaddr);
      return -1;
    }
    hmap |= hmap_u;
    if(acc == acc_wr || acc == acc_wx)
      hmap &= ~hmap_m;
    rstore_w(cpu, h + p, hmap);
    r = ((hmap & hmap_phy) << 10) | o;
  }
#endif

  return r;
}


static inline uint32_t E50X(v2rx)(cpu_t *cpu, uint32_t vaddr, acc_t acc)
{
//if((vaddr & 0x0ffffffe) == 0xc0207ec)
//{
//	printf("ZZZ %4.4x\n", cpu->crs->ownerl);
//	logall("ZZZ %4.4x\n", cpu->crs->ownerl);
//}

  vaddr |= cpu->pb & ea_r;

  uint32_t x = CACHE_INDEX(vaddr, acc);

  if(acc != acc_io)
  {
dbgmsg("v %8.8x r %8.8x i %d a %8.8x\n", cpu->tlb.v[x], cpu->tlb.r[x], x, vaddr);
    if(cpu->tlb.v[x] == (vaddr & ~0x3ff))
    {
      E50X(acc_check)(cpu, cpu->tlb.s[x], vaddr, acc);
      return cpu->tlb.r[x] | (vaddr & 0x3ff);
    }
  }

  uint32_t sdw = E50X(fetch_sdw)(cpu, vaddr, E50X(segment_fault));
  E50X(acc_check)(cpu, sdw, vaddr, acc);

  uint32_t r = E50X(xlatv2r)(cpu, sdw, vaddr, acc, E50X(page_fault));

  dbgmsg("\n-> v %8.8x -> r %8.8x\n", vaddr, r);

  if(acc != acc_io)
  {
    cpu->tlb.v[x] = vaddr & ~0x3ff;
    cpu->tlb.r[x] = r & ~0x3ff;
    cpu->tlb.s[x] = sdw;
  }

  return r;
}


static inline uint32_t E50X(v2r)(cpu_t *cpu, uint32_t vaddr, acc_t acc)
{
if((vaddr & ea_f)) PRINTF("bug2\n");
#if defined E16S || defined E32S || defined E32R || defined E64R
  vaddr |= (cpu->b << 16);
#else
  vaddr |= (cpu->b << 16) & ea_r;
#endif

  if(!cpu->crs->km.sm)
    return vaddr & 0x0fffffff;

  return E50X(v2rx)(cpu, vaddr, acc);
}


#if defined(HMDE)
static inline int32_t i2r(cpu_t *cpu, uint32_t vaddr)
{
  if(!cpu->crs->km.mio)
    return vaddr & 0x0fffffff;

  int32_t r = cpu->iotlb.i[IOTLB_INDEX(vaddr)] | (vaddr & em50_page_offm);
logmsg("i2r [%d] %8.8x %8.8x\n", IOTLB_INDEX(vaddr), vaddr, r);
  if(r >= 0)
    return r;
  else
  {
    uint32_t sdw = E50X(fetch_sdw)(cpu, vaddr, NULL);
    if((sdw & sdw_f))
      return -1;
    r = E50X(xlatv2r)(cpu, sdw, vaddr, acc_io, NULL);
logmsg("i2r > %8.8x\n", r);
    if(r >= 0)
    {
      cpu->iotlb.i[IOTLB_INDEX(vaddr)] = r & em50_page_mask;
      return r;
    }
  }

  return -1;
}
#endif


static inline uint16_t E50X(vfetch_wx)(cpu_t *cpu, uint32_t addr, acc_t acc)
{
uint16_t r;
logmsg("\n\n*** " E50S " %4.4x vfetch_w %8.8x %4.4x %s***\n\n", cpu->crs->ownerl, addr, rfetch_w(cpu, E50X(v2r)(cpu, addr, acc_nn)), ISAT(cpu, addr) ? "ATR " : "");

  if(!ISAT(cpu, addr))
    r = rfetch_w(cpu, E50X(v2r)(cpu, addr, acc));
  else
    r = tfetch_w(cpu, addr);

  return r;
}


static inline uint16_t E50X(vfetch_w)(cpu_t *cpu, uint32_t addr)
{
  return E50X(vfetch_wx)(cpu, WXX(addr), acc_rd);
}


static inline uint32_t E50X(vfetch_dx)(cpu_t *cpu, uint32_t addr, acc_t acc)
{
uint32_t r;
logmsg("\n\n*** " E50S " %4.4x vfetch_d %8.8x %8.8x %s***\n\n", cpu->crs->ownerl, addr, rfetch_d(cpu, E50X(v2r)(cpu, addr, acc_nn)), ISAT(cpu, addr) ? "ATR " : "");

  if(!page_cross_d(addr))
    if(!ISAT(cpu, addr))
      r = rfetch_d(cpu, E50X(v2r)(cpu, addr, acc));
    else
      r = tfetch_d(cpu, addr);
  else
    r = (E50X(vfetch_wx)(cpu, addr, acc) << 16) | E50X(vfetch_wx)(cpu, intraseg_i(addr, 1), acc);

  return r;
}


static inline uint32_t E50X(vfetch_d)(cpu_t *cpu, uint32_t addr)
{
  return E50X(vfetch_dx)(cpu, WXX(addr), acc_rd);
}


static inline uint64_t E50X(vfetch_qx)(cpu_t *cpu, uint32_t addr, acc_t acc)
{
uint64_t r;
logmsg("\n\n*** " E50S " %4.4x vfetch_q %8.8x %16.16jx %s***\n\n", cpu->crs->ownerl, addr, (uintmax_t)rfetch_q(cpu, E50X(v2r)(cpu, addr, acc_nn)), ISAT(cpu, addr) ? "ATR " : "");

  if(!page_cross_q(addr))
    if(!ISAT(cpu, addr))
      r = rfetch_q(cpu, E50X(v2r)(cpu, addr, acc));
    else
      r = tfetch_q(cpu, addr);
  else
    r = ((uint64_t)E50X(vfetch_dx)(cpu, addr, acc) << 32) | E50X(vfetch_dx)(cpu, intraseg_i(addr, 2), acc);

  return r;
}


static inline uint64_t E50X(vfetch_q)(cpu_t *cpu, uint32_t addr)
{
  return E50X(vfetch_qx)(cpu, WXX(addr), acc_rd);
}


static inline uint16_t E50X(vfetch_i)(cpu_t *cpu)
{

  ++cpu->c;

  if(!cpu->exec)
  {
    cpu->po = cpu->pb;
#if defined V_MODE || defined I_MODE
    uint16_t r = to_be_16(E50X(vfetch_wx)(cpu, cpu->pb, acc_ex));
#else
    uint16_t r = to_be_16(E50X(vfetch_wx)(cpu, cpu->p, acc_ex));
#endif
    cpu->p++;
    return r;
  }
  else
  {
#if defined V_MODE || defined I_MODE
    uint16_t r = to_be_16(E50X(vfetch_wx)(cpu, cpu->exec, acc_ex));
#else
    uint16_t r = to_be_16(E50X(vfetch_wx)(cpu, cpu->ep, acc_ex));
#endif
    cpu->ep++;
    return r;
  }
}


static inline uint16_t E50X(vfetch_iw)(cpu_t *cpu)
{
uint16_t r;

  if(!cpu->exec)
  {
#if defined V_MODE || defined I_MODE
    r = E50X(vfetch_wx)(cpu, cpu->pb, acc_ex);
#else
    r = E50X(vfetch_wx)(cpu, cpu->p, acc_ex);
#endif
    cpu->p++;
  }
  else
  {
#if defined V_MODE || defined I_MODE
    r = E50X(vfetch_wx)(cpu, cpu->exec, acc_ex);
#else
    r = E50X(vfetch_wx)(cpu, cpu->ep, acc_ex);
#endif
    cpu->ep++;
  }

  return r;
}


static inline uint16_t E50X(vfetch_iw_be)(cpu_t *cpu)
{
  return to_be_16(E50X(vfetch_iw)(cpu));
}


static inline uint32_t E50X(vfetch_id)(cpu_t *cpu)
{
  if(!cpu->exec)
  {
#if defined V_MODE || defined I_MODE
    uint32_t r = E50X(vfetch_dx)(cpu, cpu->pb, acc_ex);
#else
    uint32_t r = E50X(vfetch_dx)(cpu, cpu->p, acc_ex);
#endif
    cpu->p += 2;
    return r;
  }
  else
  {
#if defined V_MODE || defined I_MODE
    uint32_t r = E50X(vfetch_dx)(cpu, cpu->exec, acc_ex);
#else
    uint32_t r = E50X(vfetch_dx)(cpu, cpu->ep, acc_ex);
#endif
    cpu->ep += 2;
    return r;
  }
}


static inline uint32_t E50X(vfetch_id_be)(cpu_t *cpu)
{
  return to_be_32(E50X(vfetch_id)(cpu));
}


static inline void E50X(vstore_wx)(cpu_t *cpu, uint32_t addr, uint16_t val, acc_t acc)
{
logmsg("\n\n*** " E50S " %4.4x vstore_w %8.8x %4.4x %s***\n\n", cpu->crs->ownerl, addr, val, ISAT(cpu, addr) ? "ATR " : "");
  if(!ISAT(cpu, addr))
    rstore_w(cpu, E50X(v2r)(cpu, addr, acc), val);
  else
    tstore_w(cpu, addr, val);
}


static inline void E50X(vstore_w)(cpu_t *cpu, uint32_t addr, uint16_t val)
{
  E50X(vstore_wx)(cpu, WXX(addr), val, acc_wr);
}


static inline void E50X(vstore_dx)(cpu_t *cpu, uint32_t addr, uint32_t val, acc_t acc)
{
logmsg("\n\n*** " E50S " %4.4x vstore_d %8.8x %8.8x %s***\n\n", cpu->crs->ownerl, addr, val, ISAT(cpu, addr) ? "ATR " : "");
  if(!page_cross_d(addr))
    if(!ISAT(cpu, addr))
      rstore_d(cpu, E50X(v2r)(cpu, addr, acc), val);
    else
      tstore_d(cpu, addr, val);
  else
  {
    E50X(vstore_wx)(cpu, addr, val >> 16, acc);
    E50X(vstore_wx)(cpu, intraseg_i(addr, 1), val, acc);
  }
}


static inline void E50X(vstore_d)(cpu_t *cpu, uint32_t addr, uint32_t val)
{
  E50X(vstore_dx)(cpu, WXX(addr), val, acc_wr);
}


static inline void E50X(vstore_qx)(cpu_t *cpu, uint32_t addr, uint64_t val, acc_t acc)
{
logmsg("\n\n*** " E50S " %4.4x vstore_q %8.8x %16.16jx %s***\n\n", cpu->crs->ownerl, addr, (uintmax_t)val, ISAT(cpu, addr) ? "ATR " : "");
  if(!page_cross_q(addr))
    if(!ISAT(cpu, addr))
      rstore_q(cpu, E50X(v2r)(cpu, addr, acc), val);
    else
      tstore_q(cpu, addr, val);
  else
  {
    E50X(vstore_dx)(cpu, addr, val >> 32, acc);
    E50X(vstore_dx)(cpu, intraseg_i(addr, 2), val, acc);
  }
}


static inline void E50X(vstore_q)(cpu_t *cpu, uint32_t addr, uint64_t val)
{
  E50X(vstore_qx)(cpu, WXX(addr), val, acc_wr);
}


static inline uint16_t E50X(vfetch_is)(cpu_t *cpu, uint16_t addr)
{
#if defined E16S || defined E32S || defined E32R
  int n = 8;
  do {
#endif

    ATON(cpu, WXX(addr));
    addr = E50X(vfetch_w)(cpu, ISAT(cpu, addr) ? addr : (cpu->b << 16) | addr);

#if defined E16S || defined E32S || defined E32R

#if defined E16S
    if(ad_x(addr))
      addr += G_X(cpu);
#endif

    if(--n < 0)
      E50X(rxm_fault)(cpu);

  } while(ad_i(addr));
#endif

logmsg("\n\nIndirect short *** %4.4x:%4.4x ***\n\n", (addr<<1), addr);

  return addr;
}


static inline uint32_t E50X(vfetch_il)(cpu_t *cpu, uint32_t addr)
{
  addr |= cpu->pb & ea_r;
  uint32_t i = E50X(vfetch_d)(cpu, addr) | (addr & ea_r);

logmsg("\n\nIndirect long *** %4.4x:%4.4x:%8.8x ***\n\n", (addr<<1), addr, i);

  if(i & ea_f)
    E50X(pointer_fault)(cpu, i >> 16, addr);

  return i;
}


static inline uint32_t E50X(fetch_ap)(cpu_t *cpu, uint8_t *ptr, uint16_t *bit)
{
uint32_t ap = fetch_d(ptr);
int b = (ap >> 24) & 0b11;
uint32_t a = intraseg_i(G_ZB(cpu, b), (ap & 0x00ffffff)) | (cpu->pb & ea_r);

logmsg("\n\nAP *** %8.8x %6.6X(%d)***\n\n", ap, ap & 0x00ffffff, b);

  if((ap & 0x08000000))
  {
    uint32_t i = E50X(vfetch_il)(cpu, a);

    if(bit)
      *bit = (i & ea_e) ? (E50X(vfetch_w)(cpu, intraseg_i(a, 2)) >> 12) : 0;

logmsg("\n\nIndirect ap *** %8.8x:%4.4x ***\n\n", i, bit ? *bit : 0);

    return i;
  }
  else
{
    if(bit)
      *bit = ap >> 28;

logmsg("\n\nap *** %8.8x ***\n\n", a);
}

  return a;
}


static inline uint32_t E50X(vfetch_iap)(cpu_t *cpu, uint16_t *bit)
{
  cpu->arg_ap = E50X(vfetch_id_be)(cpu);

  return E50X(fetch_ap)(cpu, cpu->op + 2, bit);
}
