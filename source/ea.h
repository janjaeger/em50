/* Address Formation and Translation
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


#include "mm.h"

#ifndef _ea_h
#define _ea_h


/* S, R and V instructions with 1101 in bits 3..6 cannot specify indexing */

#define op_i(_o) (((_o)[0] >> 7) & 1)
#define op_x(_o) (((_o)[0] >> 6) & 1)
#define op_s(_o) (((_o)[0] >> 1) & 1)
#define op_d(_o) ((((_o)[0] & 1) << 8) | (_o)[1])
#define op_o(_o) (((int16_t)((_o)[0] << 15) >> 7) | (_o)[1])
#define op_b(_o) ((_o)[1] & 0b11)
#define op_cb(_o) op_b(_o)
#define op_n(_o) (((_o)[0] & 0b00111100) == 0b00110100)
#define op_l(_o) (((_o)[2] << 8) | (_o)[3])
#define op_nx(_o) (op_n(_o) ? 0 : op_x(_o))
#define op_I  0b10000000
#define op_X  0b01000000
#define op_S  0b00000010
#define op_Y  0b00010000
#define op_BR 0b00000011
#define op_nX(_o) (op_n(_o) ? 0 : op_X)
#define op_nY(_o) (op_n(_o) ? 0 : op_Y)
#define op_IXS(_o) ((_o)[0] & (op_I | op_nX(_o) | op_S))
#define op_IX(_o)  ((_o)[0] & (op_I | op_nX(_o)))
#define op_IXY(_o) ((((_o)[0] & (op_I | op_X)) >> 5) | (((_o)[1] & op_Y) >> 4))
#define ad_I 0x8000
#define ad_X 0x4000
#define ad_i(_a) (_a & ad_I)
#define ad_x(_a) (_a & ad_X)

#define w16(_x) ((_x) & 0xffff)
#define w15(_x) ((_x) & 0x7fff)
#define w14(_x) ((_x) & 0x3fff)


#define op_DR  0b0000001110000000
#define op_DR0 0b00000011
#define op_DR1 0b10000000

#define op_TM  0b01100000
#define op_SR  0b00011100

#define op_dr(_o) ((((_o)[0] & op_DR0) << 1) | (((_o)[1] & op_DR1) >> 7))
#define op_tm(_o) (((_o)[1] & op_TM) >> 5)
#define op_sr(_o) (((_o)[1] & op_SR) >> 2)
#define op_br(_o) ((_o)[1] & op_BR)

#define op_dac(_o) ((_o)[0] & 1)

static inline char *ixs(op_t op)
{
static char *tab[8] = { "---", "--s", "-x-", "-xs", "i--", "i-s", "ix-", "ixs" };
  return tab[(op_i(op) << 2) | (op_x(op) << 1) | op_s(op)];
}

#define AB(_c, _x, _v) (intraseg_i(G_ ## _x(_c), (_v)))
#define AX(_c, _x, _v) (intraseg_i(G_ZB(_c, _x), (_v)))

#endif

#undef op_is_long
#if defined E16S || defined E32S
 #define op_is_long(_o) (0)
#elif defined E32R || defined E64R
 #define op_is_long(_o) (((_o[0] & 0b11) == 0b11) && ((_o[1] & 0b11110000) == 0))
#elif defined E64V
 #define op_is_long(_o) (((_o[0] & 0b11) == 0b11) && ((_o[1] & 0b11100000) == 0))
#elif defined E32I
 #define op_is_long(_o) (1)
#else
 #error No mode defined
#endif

#if defined E16S

static inline uint16_t E50X(ea)(cpu_t *cpu, op_t op)
{
uint16_t dis;

  dis = op_d(op);  // 0/D

  if(op_s(op))
    dis |= (cpu->exec ? cpu->ep : cpu->p) & 0x3e00;  // C/D

  if(op_nx(op))
    dis += G_X(cpu);  // -/D+X

  if(op_i(op))
    dis = E50X(vfetch_is)(cpu, dis);

  return dis & 0x3fff;
}

#elif defined E32S

static inline uint16_t E50X(ea)(cpu_t *cpu, op_t op)
{
uint16_t dis = op_d(op);

  switch(op_IXS(op))
  {
  case 0:
    break;                   // 0/D

  case op_S:
    dis |= (cpu->exec ? cpu->ep : cpu->p) & 0x7e00;  // C/D
    break;

  case op_X:
    dis += G_X(cpu);
    break;                   // 0/D+X

  case op_X | op_S:
    dis |= (cpu->exec ? cpu->ep : cpu->p) & 0x7e00;
    dis += G_X(cpu);         // C/D+X
    break;

  case op_I:
    dis = E50X(vfetch_is)(cpu, dis); // I(0/D);
    break;

  case op_I | op_S:
    dis |= (cpu->exec ? cpu->ep : cpu->p) & 0x7e00;
    dis =  E50X(vfetch_is)(cpu, dis); // I(C/D);
    break;

  case op_I | op_X:
    if(dis < 0100)
      dis =  E50X(vfetch_is)(cpu, dis + G_X(cpu)); // I(0/D+X);
    else
      dis =  E50X(vfetch_is)(cpu, dis) + G_X(cpu); // I(0/D)+X;
    break;

  case op_I | op_X | op_S:
    dis |= (cpu->exec ? cpu->ep : cpu->p) & 0x7e00;
    dis = E50X(vfetch_is)(cpu, dis);
    dis += G_X(cpu);              // I(C/D)+X;
    break;

  default:
abort();
  }

  return dis & 0x7fff;
}

#elif defined E32R || defined E64R

#undef WXX
#if defined E16S
 #define WXX(_a) w14(_a)
#elif defined E32S || defined E32R
 #define WXX(_a) w15(_a)
#elif defined E64R
 #define WXX(_a) w16(_a)
#endif


/* 64R Mode, Short Form, Table 3-3 Arch Ref Guide */

static inline uint16_t E50X(ixs_ea_short)(cpu_t *cpu, op_t op)
{
uint16_t dis;

  switch(op_IXS(op))
  {
  case 0:
    dis = op_d(op); // 0/D
    break;

  case op_X:
    dis = op_d(op) + G_X(cpu); // 0/D+X
    break;

  case op_I:
    dis = E50X(vfetch_is)(cpu, op_d(op)); // I(0/D);
    break;

  case op_I | op_X:
    dis = op_d(op);
    if(dis < 0100)
      dis =  E50X(vfetch_is)(cpu, dis + G_X(cpu)); // I(0/D+X);
    else
      dis =  E50X(vfetch_is)(cpu, dis) + G_X(cpu); // I(0/D)+X;
    break;

  case op_S:
    dis = (cpu->exec ? cpu->ep : cpu->p) + op_o(op);  // P+D;
    break;

  case op_X | op_S:
    dis = (cpu->exec ? cpu->ep : cpu->p) + op_o(op) + G_X(cpu); // P+D+X;
    break;

  case op_I | op_S:
    dis = E50X(vfetch_is)(cpu, (cpu->exec ? cpu->ep : cpu->p) + op_o(op)); // I(P+D);
    break;

  case op_I | op_X | op_S:
    dis = E50X(vfetch_is)(cpu, (cpu->exec ? cpu->ep : cpu->p) + op_o(op)) + G_X(cpu); // I(P+D)+X;
    break;

  default:
abort();
  }

  return dis;
}

static inline uint16_t E50X(ix_ea_arg)(cpu_t *cpu, op_t op)
{
    cpu->arg = E50X(vfetch_iw_be)(cpu);
    return op_l(op);
}

static inline uint16_t E50X(ix_ea_long)(cpu_t *cpu, op_t op)
{
uint16_t di;
uint16_t sp;
uint16_t ea;
int cb = op_cb(op);

  switch(op_IX(op) | cb)
  {
  case 0 | 0:  //(B)
    di = E50X(ix_ea_arg)(cpu, op);
    return di; // D

  case op_I | 0:  //(B)
    di = E50X(ix_ea_arg)(cpu, op);
    return E50X(vfetch_is)(cpu, di); // I(D)

  case op_X | 0:  //(B)
    di = E50X(ix_ea_arg)(cpu, op);
    return di + G_X(cpu); // D+X

  case op_I | op_X | 0:  //(B)
    di = E50X(ix_ea_arg)(cpu, op);
    return E50X(vfetch_is)(cpu, di + G_X(cpu)); // I(D+X)

  case 0 | 1:  //(C)
    di = E50X(ix_ea_arg)(cpu, op);
    sp = G_SP(cpu);
    return di + sp; // D+SP

  case op_I | 1: //(C)
    di = E50X(ix_ea_arg)(cpu, op);
    sp = G_SP(cpu);
    return E50X(vfetch_is)(cpu, di + sp); // I(D+SP)

  case op_X | 1: //(C)
    di = E50X(ix_ea_arg)(cpu, op);
    sp = G_SP(cpu);
    return di + sp + G_X(cpu); // D+SP+X

  case op_I | op_X | 1: //(C)
    di = E50X(ix_ea_arg)(cpu, op);
    sp = G_SP(cpu);
    return E50X(vfetch_is)(cpu, di + sp + G_X(cpu)); // I(D+SP+X)

  case 0 | 2: //(F)
    sp = G_SP(cpu);
    S_SP(cpu, sp + 1);
    return sp;  // SP @Postincrement

  case op_I | 2: //(F)
    sp = G_SP(cpu);
    ea = E50X(vfetch_is)(cpu, sp++);  // I(SP) @Postincrement
    S_SP(cpu, sp);
    return ea;

  case op_X | 2: //(F)
    sp = G_SP(cpu);
    ea = E50X(vfetch_is)(cpu, sp++) + G_X(cpu);  // I(SP)+X @Postincrement
    S_SP(cpu, sp);
    return ea;

  case op_I | op_X | 2: //(D)
    di = E50X(ix_ea_arg)(cpu, op);
    return E50X(vfetch_is)(cpu, di) + G_X(cpu); // I(D)+X

  case 0 | 3: //(G)
    sp = G_SP(cpu);
    S_SP(cpu, --sp);
    return sp;  // SP-1 #Predecrement

  case op_I | 3: //(G)
    sp = G_SP(cpu);
    ea = E50X(vfetch_is)(cpu, --sp);  // I(SP-1) #Predecrement
    S_SP(cpu, sp);
    return ea;

  case op_X | 3: //(G)
    sp = G_SP(cpu);
    ea = E50X(vfetch_is)(cpu, --sp) + G_X(cpu);  // I(SP-1)+X #Predecrement
    S_SP(cpu, sp);
    return ea;

  case op_I | op_X | 3: //(E)
    di = E50X(ix_ea_arg)(cpu, op);
    sp = G_SP(cpu);
    return E50X(vfetch_is)(cpu, di + sp) + G_X(cpu); // I(D+SP)+X

  default:
abort();
  }
}

static inline uint16_t E50X(ea)(cpu_t *cpu, op_t op)
{
uint16_t r;

  if(op_is_long(op))
    r = E50X(ix_ea_long)(cpu, op);
  else
    r = E50X(ixs_ea_short)(cpu, op);

  ATON(cpu, WXX(r));

  return r;
}

#elif defined E64V

/* 64V Mode, Short Form, Table 3-3 Arch Ref Guide */

#define nfetch_w(_c, _a) (_a)
static inline uint32_t E50X(ixs_ea_short)(cpu_t *cpu, op_t op)
{
uint16_t atr = cpu->crs->km.sm ? 010 : 040;
uint16_t dis;
int16_t off;

  switch(op_IXS(op))
  {
  case 0:
    dis = op_d(op);
    if(dis < atr)
        return dis;  // REG;
    if(dis < 0400)
        return AB(cpu, SB, dis); // SB + D;
    else
        return AB(cpu, LB, dis); // LB + D;

  case op_X:
    dis = op_d(op) + G_X(cpu);
    if(dis < atr)
      return dis; // REG;
    if(op_d(op) < 0400)
      return AB(cpu, SB, dis); // SB + D + X;
    else
      return AB(cpu, LB, dis); // LB + D + X;

  case op_I:
    dis = op_d(op);
    if(dis < atr)
      return AB(cpu, PB, E50X(vfetch_is)(cpu, dis)); // I(REG);
    else
      return AB(cpu, PB, E50X(vfetch_is)(cpu, AB(cpu, PB, dis))); // I(PB + D);

  case op_I | op_X:
    dis = op_d(op);
    if(dis + G_X(cpu) < atr)
      return AB(cpu, PB, E50X(vfetch_is)(cpu, dis + G_X(cpu))); // I(REG);
    if(dis < 0100)
      return AB(cpu, PB, E50X(vfetch_is)(cpu, AB(cpu, PB, dis + G_X(cpu)))); // I(PB + D + X);
    else
      return AB(cpu, PB, E50X(vfetch_is)(cpu, AB(cpu, PB, dis)) + G_X(cpu)); // I(PB + D) + X;

  case op_S:
    off = op_o(op);
    return AB(cpu, PB, (cpu->exec ? cpu->ep : cpu->p) + off);  // P + D;

  case op_X | op_S:
    off = op_o(op);
    return AB(cpu, PB, (cpu->exec ? cpu->ep : cpu->p) + off + G_X(cpu)); // P + D + X;

  case op_I | op_S:
    off = op_o(op);
    return AB(cpu, PB, E50X(vfetch_is)(cpu, AB(cpu, PB, (cpu->exec ? cpu->ep : cpu->p) + off))); // I(P + D);

  case op_I | op_X | op_S:
    off = op_o(op);
    return AB(cpu, PB, E50X(vfetch_is)(cpu, AB(cpu, PB, (cpu->exec ? cpu->ep : cpu->p) + off)) + G_X(cpu)); // I(P + D) + X;

  default:
abort();
  }
}

static inline uint32_t E50X(ix_ea_long)(cpu_t *cpu, op_t op)
{
#ifdef DEBUG
static const char *base[] = { "PB", "SB", "LB", "XB" };
#endif

// Non-indexing address formation
#if defined(em50_ea_earlier)
static const int nx[8] = { 0, 0, 0, 3, 3, 3, 3, 3 };
#elif defined(em50_ea_p750)
static const int nx[8] = { 0, 1, 0, 6, 4, 3, 6, 3 };
#elif defined(em50_ea_current)
static const int nx[8] = { 0, 0, 0, 0, 3, 3, 3, 3 };
#else
static const int nx[3][8] = {
//  0  1  2  3  4  5  6  7
  { 0, 0, 0, 3, 3, 3, 3, 3 },
  { 0, 1, 0, 6, 4, 3, 6, 3 },
  { 0, 0, 0, 0, 3, 3, 3, 3 }
};
#endif

  cpu->arg = E50X(vfetch_iw_be)(cpu);

  uint16_t di = op_l(op);

  int s = op_IXY(op);

  if(op_n(op))
     s = nx
#if !defined(MODEL)
           [cpu->model.ea]
#endif
                          [s];

if(op_b(op) /* && G_ZB(cpu, op_b(op)) */ ) logmsg("\n\n*** BASE(%s) %8.8x ***\n\n", base[op_b(op)], G_ZB(cpu, op_b(op)));


  switch(s)
  {
  case 0:
    return AX(cpu, op_b(op), di);

  case 1: // op_Y:
    return AX(cpu, op_b(op), di + G_Y(cpu));

  case 2: // op_X:
    return AX(cpu, op_b(op), di + G_X(cpu));

  case 3: // op_X | op_Y:
    return E50X(vfetch_il)(cpu, AX(cpu, op_b(op), di));

  case 4: // op_I:
    return E50X(vfetch_il)(cpu, AX(cpu, op_b(op), di + G_Y(cpu)));

  case 5: // op_I | op_Y:
    return intraseg_i(E50X(vfetch_il)(cpu, AX(cpu, op_b(op), di)), G_Y(cpu));

  case 6: // op_I | op_X:
    return E50X(vfetch_il)(cpu, AX(cpu, op_b(op), di + G_X(cpu)));

  case 7: // op_I | op_X | op_Y:
    return intraseg_i(E50X(vfetch_il)(cpu, AX(cpu, op_b(op), di)), G_X(cpu));

  default:
abort();
  }
}

static inline uint32_t E50X(ea)(cpu_t *cpu, op_t op)
{
uint32_t r;

  ATOFF(cpu);

  if(op_is_long(op))
  {
    r = E50X(ix_ea_long)(cpu, op);
  }
  else
  {
    r = E50X(ixs_ea_short)(cpu, op);
    ATON(cpu, r & 0xffff);
  }

  return r | (cpu->pb & ea_r);
}

#elif defined E32I

static inline uint32_t E50X(x_ea)(cpu_t *cpu, op_t op, int a)
{
dw_t r;

  int tm = op_tm(op);
  int br = op_br(op);
  int sr = op_sr(op);

logmsg("\nx_ea tm %d br %d sr %d\n", tm, br, sr);

  if(a)
    cpu->arg = E50X(vfetch_iw_be)(cpu);

  uint16_t d = from_be_16(cpu->arg);

  switch(tm) {
    case 3: // Indirect
      if(sr == 0)
        r.d = E50X(vfetch_il)(cpu, AX(cpu, br, d));
      else  // Indirect postindexed
        r.d = intraseg_i(E50X(vfetch_il)(cpu, AX(cpu, br, d)), G_RH(cpu, sr));
      break;
    case 2: // Indirect
      if(sr == 0)
        r.d = E50X(vfetch_il)(cpu, AX(cpu, br, d));
      else  // Indirect preindexed
        r.d = E50X(vfetch_il)(cpu, AX(cpu, br, d + G_RH(cpu, sr)));
      break;
    case 1: // Direct
      if(sr == 0)
        r.d = AX(cpu, br, d);
      else  // Indexed
        r.d = AX(cpu, br, d + G_RH(cpu, sr));
      break;
    case 0:
      switch(br) {
        case 0: // Register to Register
abort()   ;  // handled in efetch/estore
          break;
        case 1: 
          if(sr == 0) // Immediate type 1
abort()     ;  // handled in efetch/estore
          else        // Immediate type 2
abort()     ;  // handled in efetch/estore
          break;
        case 2:
          switch(sr) {
            case 0: // Immediate type 3
abort()       ;  // handled in efetch/estore
              break;
            case 1: // Floating register source (FR0)
logall("ea todo 1: tm %d br %d, sr %d\n", tm, br, sr);
              r.d = G_FAC(cpu, 0);
              break;
            case 3: // Floating register source (FR1)
logall("ea todo 2: tm %d br %d, sr %d\n", tm, br, sr);
              r.d = G_FAC(cpu, 1);
              break;
            default:
              E50X(uii_fault)(cpu, 0);
          }
          break;
        case 3: // General register relative
          r.h = G_RH(cpu, sr);
          r.l = G_RL(cpu, sr) + d;
          break;
        default:
          E50X(uii_fault)(cpu, 0);
      }
      break;
    default:
      E50X(uii_fault)(cpu, 0);
  }

  return r.d | (cpu->pb & ea_r);
}

static inline uint32_t E50X(ea)(cpu_t *cpu, op_t op)
{
  return E50X(x_ea)(cpu, op, 1);
}

static inline uint16_t E50X(efetch_w)(cpu_t *cpu, op_t op)
{
  int tm = op_tm(op);

int br = op_br(op);
int sr = op_sr(op);
logmsg("\nefetch_w tm %d br %d sr %d\n", tm, br, sr);
if(sr) logmsg("srh %x sr %x\n", G_RH(cpu, sr), G_R(cpu, sr)); 

  if(tm)
    return E50X(vfetch_w)(cpu, E50X(ea)(cpu, op));

//int br = op_br(op);

  if(br == 0)
    return G_RH(cpu, op_sr(op));

  if(br == 1)
  {
    cpu->arg = E50X(vfetch_iw_be)(cpu);
    return from_be_16(cpu->arg);
  }

  if(br == 3)
  {
logall("todo d16\n");
    return E50X(vfetch_w)(cpu, E50X(ea)(cpu, op));
  }

  logall("%8.8x\n", G_FAC(cpu, sr & 1));
  logall("todo w\n");

  E50X(uii_fault)(cpu, 0);
  return G_FAC(cpu, sr & 1);
}


static inline uint32_t E50X(efetch_d)(cpu_t *cpu, op_t op)
{
  int tm = op_tm(op);

int br = op_br(op);
int sr = op_sr(op);
logmsg("\nefetch_d tm %d br %d sr %d\n", tm, br, sr);
if(sr) logmsg("srh %x sr %x\n", G_RH(cpu, sr), G_R(cpu, sr)); 

  if(tm)
    return E50X(vfetch_d)(cpu, E50X(ea)(cpu, op));

//int br = op_br(op);

  if(br == 0)
    return G_R(cpu, op_sr(op));

  if(br == 1)
  {
    cpu->arg = E50X(vfetch_iw_be)(cpu);
    if(sr == 0)
      return from_be_16(cpu->arg) << 16;
    else
      return (int32_t)((int16_t)from_be_16(cpu->arg));
  }

  if(br == 3)
  {
logall("todo d32\n");
    return E50X(vfetch_d)(cpu, E50X(ea)(cpu, op));
  }

  switch(sr) {
    case 0:
    {
    uint16_t r = E50X(vfetch_iw)(cpu);
      return ((uint32_t)(r & 0xff00) << 16) | (r & 0x00ff);
    }
    case 1:
      return G_FAC(cpu, 0);
    case 3:
      return G_FAC(cpu, 1);
    default:
      E50X(uii_fault)(cpu, 0);
  }

  E50X(uii_fault)(cpu, 0);
  return 0;
}


static inline uint64_t E50X(efetch_q)(cpu_t *cpu, op_t op)
{
  int tm = op_tm(op);

int br = op_br(op);
int sr = op_sr(op);
logmsg("\nefetch_q tm %d br %d sr %d\n", tm, br, sr);
if(sr) logmsg("srh %x sr %x\n", G_RH(cpu, sr), G_R(cpu, sr)); 

  if(tm)
    return E50X(vfetch_q)(cpu, E50X(ea)(cpu, op));

//int br = op_br(op);

  if(br == 0)
    return G_R(cpu, op_sr(op));

  if(br == 1)
  {
    cpu->arg = E50X(vfetch_iw_be)(cpu);
    if(sr == 0)
      return from_be_16(cpu->arg) << 16;
    else
      return (int32_t)((int16_t)from_be_16(cpu->arg));
  }

  if(br == 3)
  {
logall("todo d64\n");
    return E50X(vfetch_q)(cpu, E50X(ea)(cpu, op));
  }

  switch(sr) {
    case 0:
    {
    uint16_t r = E50X(vfetch_iw)(cpu);
      return ((uint64_t)(r & 0xff00) << 48) | (r & 0x00ff);
      return r;
    }
    case 1:
      return G_DAC(cpu, 0);
    case 3:
      return G_DAC(cpu, 1);
    default:
      E50X(uii_fault)(cpu, 0);
  }

  logall("%16.16jx\n", (uintmax_t)G_DAC(cpu, sr & 1));
  logall("todo q\n");

  E50X(uii_fault)(cpu, 0);
  return 0;
}


static inline void E50X(estore_wx)(cpu_t *cpu, op_t op, uint16_t val, int a)
{
  int tm = op_tm(op);

int br = op_br(op);
int sr = op_sr(op);
logmsg("\nestore_w tm %d br %d sr %d\n", tm, br, sr);
if(sr) logmsg("srh %x sr %x\n", G_RH(cpu, sr), G_R(cpu, sr)); 

  if(tm)
  {
    E50X(vstore_w)(cpu, E50X(x_ea)(cpu, op, a), val);
    return;
  }

//int br = op_br(op);

  if(br == 0)
  {
    S_RH(cpu, op_sr(op), val);
    return;
  }

  if(br == 3)
  {
logmsg("grr\n");
    E50X(vstore_w)(cpu, E50X(x_ea)(cpu, op, a), val);
    return;
  }
//if(br == 1)
//{
//  cpu->arg = E50X(vfetch_iw_be)(cpu);
//  return from_be_16(cpu->arg);
//}

  logall("todo w\n");
  E50X(uii_fault)(cpu, 0);

}


static inline void E50X(estore_dx)(cpu_t *cpu, op_t op, uint32_t val, int a)
{
  int tm = op_tm(op);

int br = op_br(op);
int sr = op_sr(op);
logmsg("\nestore_dx tm %d br %d sr %d\n", tm, br, sr);
if(sr) logmsg("srh %x sr %x\n", G_RH(cpu, sr), G_R(cpu, sr)); 

  if(tm)
  {
    E50X(vstore_d)(cpu, E50X(x_ea)(cpu, op, a), val);
    return;
  }

//int br = op_br(op);

  if(br == 0)
  {
    S_R(cpu, op_sr(op), val);
    return;
  }

  if(br == 3)
  {
logmsg("grr\n");
    E50X(vstore_d)(cpu, E50X(x_ea)(cpu, op, a), val);
    return;
  }

  logall("todo D\n");
  E50X(uii_fault)(cpu, 0);

}


static inline void E50X(estore_qx)(cpu_t *cpu, op_t op, uint64_t val, int a)
{
  int tm = op_tm(op);

int br = op_br(op);
int sr = op_sr(op);
logmsg("\nestore_qx tm %d br %d sr %d\n", tm, br, sr);
if(sr) logmsg("srh %x sr %x\n", G_RH(cpu, sr), G_R(cpu, sr)); 

  if(tm)
  {
    E50X(vstore_q)(cpu, E50X(x_ea)(cpu, op, a), val);
    return;
  }

//int br = op_br(op);

  if(br == 0)
  {
    S_R(cpu, op_sr(op), val);
    return;
  }

  if(br == 3)
  {
logmsg("grr\n");
    E50X(vstore_q)(cpu, E50X(x_ea)(cpu, op, a), val);
    return;
  }

  logall("todo q\n");
  E50X(uii_fault)(cpu, 0);

}


static inline void E50X(estore_w)(cpu_t *cpu, op_t op, uint16_t val)
{
  E50X(estore_wx)(cpu, op, val, 1);
}


static inline void E50X(estore_d)(cpu_t *cpu, op_t op, uint32_t val)
{
  E50X(estore_dx)(cpu, op, val, 1);
}


static inline void E50X(estore_q)(cpu_t *cpu, op_t op, uint64_t val)
{
  E50X(estore_qx)(cpu, op, val, 1);
}


#else
 #error No mode defined
#endif
