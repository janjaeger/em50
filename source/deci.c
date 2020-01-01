/* Decimal Instructions
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

#include "deci.h"


#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


#ifndef _deci_c
#define _deci_c

#define dcw_dt_ls 0   /* Leading Separate */
#define dcw_dt_ts 1   /* Trailing Separate */
#define dcw_dt_pd 3   /* Packed Decimal */
#define dcw_dt_le 4   /* Leadning Embedded */
#define dcw_dt_te 5   /* Trailing Embedded */

typedef union {
  struct {
  uint32_t h:3,   /* f2 type */
           g:7,   /* Scale */
           f:6,   /* Number of digits in f2 */
           e:3,   /* f1 type */
           d:1,   /* round flag (xmv) */
           t:1,   /* result forced positive */
           r2:1,  /* reserved */
           c:1,   /* f2 sign inversed */
           b:1,   /* f1 sign inversed */
           r1:2,  /* reserved */
           a:6;   /* Number of digits in f1 */
  };
  uint32_t w;
} dcw_t;

typedef struct {
  uint8_t sc;
  uint8_t fc;
  int sp;
  int sign;
  int sig;
} xed_iv;

static inline unsigned int dcw_type(dcw_t dcw, int far)
{
  return far ? dcw.h : dcw.e;
}

static inline unsigned int dcw_sign(dcw_t dcw, int far)
{
  return far ? dcw.c : dcw.b;
}

static inline unsigned int dcw_abs(dcw_t dcw)
{
  return dcw.t;
}

static inline unsigned int dcw_digits(dcw_t dcw, int far)
{
  return far ? dcw.f : dcw.a;
}

static inline int64_t dcw_scale(dcw_t dcw, int xmv, int64_t value)
{
  int16_t scale = (dcw.g & 0x40) ? dcw.g | 0xff80 : dcw.g;
  
  if(scale < 0)
  {
    int64_t mult = pow(10, -scale);
    value *= mult;
  }
  else if(scale > 0)
  {
    int64_t div = pow(10, scale);
// THIS IS WRONG FOR XCM, WHICH SHOULD IGNORE DCW.D
    int64_t rnd = (!xmv || (xmv && dcw.d)) ? div / 2 : 0;
    value = (value + (value > 0 ? rnd : -rnd)) / div;
  }

  return value;
}

#endif


#define noupd 0
#define update 1
static inline void E50X(store_bytes)(cpu_t *cpu, uint8_t *bytes, const int far, int count, const int updfar)
{
  if(count == 0)
    return;

uint32_t f = G_FAR(cpu, far);
int bit = G_FBR(cpu, far);

  if(bit != 0)
  {
  uint16_t w = (E50X(vfetch_w)(cpu, f) & 0xff00) | *bytes++;
    E50X(vstore_w)(cpu, f++, w);
    --count;
    bit = 0;
  }

  for(int n = count / 2; n > 0; --n, count -= 2)
  {
  uint16_t w = *bytes++ << 8;
           w |= *bytes++;
    E50X(vstore_w)(cpu, f++, w);
  }

  if(count > 0)
  {
  uint16_t w = (E50X(vfetch_w)(cpu, f) & 0x00ff) | (*bytes << 8);
    E50X(vstore_w)(cpu, f, w);
    bit = 8;
  }

  if(updfar)
  {
    S_FBR(cpu, far, bit);
    S_FAR(cpu, far, f);
  }
}

static inline void E50X(store_ls)(cpu_t *cpu, int far, int digits, int64_t value)
{
uint8_t ls[digits + 1];

  logmsg("decimal E50X(store_ls) far %d digits %d value %jd\n", far, digits, (intmax_t)value);

  ls[0] = value < 0 ? '-' : '+';
  if(!cpu->crs->km.ascii)
    ls[0] |= 0x80;

  if(value < 0)
    value = -value;

  for(int n = digits; n > 0; --n)
  {
    ls[n] = '0' + (value % 10);
    value /= 10;
    if(!cpu->crs->km.ascii)
      ls[n] |= 0x80;
  }

  E50X(store_bytes)(cpu, ls, far, digits + 1, noupd);
}

static inline void E50X(store_ts)(cpu_t *cpu, int far, int digits, int64_t value)
{
uint8_t ls[digits + 1];

  logmsg("decimal E50X(store_ts) far %d digits %d value %jd\n", far, digits, (intmax_t)value);

  int neg = value < 0;

  if(neg)
    value = -value;

  for(int n = digits - 1; n >= 0; --n)
  {
    ls[n] = '0' + (value % 10);
    value /= 10;
    if(!cpu->crs->km.ascii)
      ls[n] |= 0x80;
  }

  ls[digits] = neg ? '-' : '+';

  if(!cpu->crs->km.ascii)
    ls[digits] |= 0x80;

  E50X(store_bytes)(cpu, ls, far, digits + 1, noupd);
}

static inline void E50X(store_pd)(cpu_t *cpu, int far, int digits, int64_t value)
{
  logmsg("decimal E50X(store_pd) far %d digits %d value %jd\n", far, digits, (intmax_t)value);

  if(!(digits & 1))
    ++digits;

  int bytes = (digits + 1) / 2;
  uint8_t pd[bytes];

  int neg = value < 0;

  if(value < 0)
    value = -value;

  value *= 10;

  for(int n = bytes - 1; n >= 0; --n)
  {
  int d = value % 100;
    value /= 100;

    pd[n] = (d / 10) << 4 | d % 10;
  }

  pd[bytes - 1] |= neg ? 0x0d : 0x0c;

  E50X(store_bytes)(cpu, pd, far, bytes, noupd);
}

static inline void E50X(store_le)(cpu_t *cpu, int far, int digits, int64_t value)
{
uint8_t ls[digits];

  logmsg("decimal E50X(store_le) far %d digits %d value %jd\n", far, digits, (intmax_t)value);

  int neg = value < 0;

  if(neg)
    value = -value;

  for(int n = digits - 1; n >= 0; --n)
  {
    ls[n] = '0' + (value % 10);
    value /= 10;
    if(!cpu->crs->km.ascii)
      ls[n] |= 0x80;
  }

  if(neg)
  {
    if((ls[0] & 0xf) == 0)
      ls[0] = '}';
    else
      ls[0] += 'J' - '1';
    if(!cpu->crs->km.ascii)
      ls[0] |= 0x80;
  }

  E50X(store_bytes)(cpu, ls, far, digits, noupd);
}

static inline void E50X(store_te)(cpu_t *cpu, const int far, const int digits, int64_t value)
{
uint8_t ls[digits];

  logmsg("decimal E50X(store_te) far %d digits %d value %jd\n", far, digits, (intmax_t)value);

  int neg = value < 0;

  if(neg)
    value = -value;

  for(int n = digits - 1; n >= 0; --n)
  {
    ls[n] = '0' + (value % 10);
    value /= 10;
    if(!cpu->crs->km.ascii)
      ls[n] |= 0x80;
  }

  if(neg)
  {
    if((ls[digits - 1] & 0xf) == 0)
      ls[digits - 1] = '}';
    else
      ls[digits - 1] += 'J' - '1';
    if(!cpu->crs->km.ascii)
      ls[digits - 1] |= 0x80;
  }

  E50X(store_bytes)(cpu, ls, far, digits, noupd);
}

static inline void E50X(load_bytes)(cpu_t *cpu, uint8_t *bytes, const int far, int count, const int updfar)
{
  if(count == 0)
    return;

uint32_t f = G_FAR(cpu, far);
int bit = G_FBR(cpu, far);

  if(bit != 0)
  {
    *bytes++ = E50X(vfetch_w)(cpu, f++) & 0xff;
    --count;
    bit = 0;
  }

  for(int n = count / 2; n > 0; --n, count -= 2)
  {
  uint16_t w = E50X(vfetch_w)(cpu, f++);
    *bytes++ = w >> 8;
    *bytes++ = w & 0xff;
  }

  if(count > 0)
  {
    *bytes = E50X(vfetch_w)(cpu, f) >> 8;
    bit = 8;
  }

  if(updfar)
  {
    S_FBR(cpu, far, bit);
    S_FAR(cpu, far, f);
  }
}

static inline int64_t E50X(load_ls)(cpu_t *cpu, int far, int digits)
{
int64_t r = 0;
uint8_t ls[digits + 1];

  E50X(load_bytes)(cpu, ls, far, digits + 1, noupd);
  for(int n = 1; n <= digits; n++)
  {
    r *= 10;
    r += ls[n] & 0xf;
  }
  // space = 0x20, + = 0x2b, { = 0x7b 1011
  //               - = 0x2d, } = 0x7d 1101
  if((ls[0] & 0b100))
    r = -r;

  logmsg("decimal E50X(load_ls) far %d digits %d value %jd\n", far, digits, (intmax_t)r);

  return r;
}

static inline int64_t E50X(load_ts)(cpu_t *cpu, int far, int digits)
{
int64_t r = 0;
uint8_t ts[digits + 1];

  E50X(load_bytes)(cpu, ts, far, digits + 1, noupd);
  for(int n = 0; n < digits; n++)
  {
    r *= 10;
    r += ts[n] & 0xf;
  }
  // space = 0x20, + = 0x2b, { = 0x7b 1011
  //               - = 0x2d, } = 0x7d 1101
  if((ts[digits] & 0b100))
    r = -r;

  logmsg("decimal E50X(load_ts) far %d digits %d value %jd\n", far, digits, (intmax_t)r);

  return r;
}

static inline int64_t E50X(load_pd)(cpu_t *cpu, int far, int digits)
{
  if(!digits)
    return 0;

  if(!(digits & 1))
    ++digits;

  int bytes = (digits + 1) / 2;
  uint8_t pd[bytes];

  E50X(load_bytes)(cpu, pd, far, bytes, noupd);

  uint64_t value = 0;

  int n;
  for(n = 0; n < bytes - 1; ++n)
  {
    value *= 100;

    value += ((pd[n] & 0xf0) >> 4) * 10 + (pd[n] & 0x0f);
  }
  value *= 10;
  value += (pd[n] & 0xf0) >> 4;
  if((pd[n] & 0x0f) == 0x0d)
    value = -value;

  logmsg("decimal E50X(load_pd) far %d digits %d value %jd\n", far, digits, (intmax_t)value);

  return value;
}

static inline int64_t E50X(load_le)(cpu_t *cpu, int far, int digits)
{
int64_t r;
uint8_t le[digits];

  E50X(load_bytes)(cpu, le, far, digits, noupd);

  uint8_t t = le[0] & 0x7f;
  int neg = t == '}' || t == '-' || (t >= 'J' && t <= 'R');
  r = (t >= 'J' && t <= 'R') ? t - 'I' : (t == '}' || t == '-') ? 0 : t & 0xf;

  for(int n = 1; n < digits; n++)
  {
    r *= 10;
    r += le[n] & 0xf;
  }
  // space = 0x20, + = 0x2b, { = 0x7b 1011
  //               - = 0x2d, } = 0x7d 1101
  if(neg)
    r = -r;

  logmsg("decimal E50X(load_le) far %d digits %d value %jd\n", far, digits, (intmax_t)r);

  return r;
}

static inline int64_t E50X(load_te)(cpu_t *cpu, int far, int digits)
{
int64_t r = 0;
uint8_t te[digits];

  E50X(load_bytes)(cpu, te, far, digits, noupd);

  for(int n = 0; n < digits - 1; n++)
  {
    r += te[n] & 0xf;
    r *= 10;
  }
  // space = 0x20, + = 0x2b, { = 0x7b 1011
  //               - = 0x2d, } = 0x7d 1101

  uint8_t l = te[digits - 1] & 0x7f;
  int neg = l == '}' || l == '-' || (l >= 'J' && l <= 'R');
  r += (l >= 'J' && l <= 'R') ? l - 'I' : (l == '}' || l == '-') ? 0 : l & 0xf;

  if(neg)
    r = -r;

  logmsg("decimal E50X(load_te) far %d digits %d value %jd\n", far, digits, (intmax_t)r);

  return r;
}

static inline int64_t E50X(load_decimal)(cpu_t *cpu, dcw_t dcw, int far)
{
int type = dcw_type(dcw, far);
int sign = dcw_sign(dcw, far);
int digits = dcw_digits(dcw, far);

int64_t r;

  switch(type) {

    case dcw_dt_ls:
      r = E50X(load_ls)(cpu, far, digits);
      break;

    case dcw_dt_ts:
      r = E50X(load_ts)(cpu, far, digits);
      break;

    case dcw_dt_pd:
      r = E50X(load_pd)(cpu, far, digits);
      break;

    case dcw_dt_le:
      r = E50X(load_le)(cpu, far, digits);
      break;

    case dcw_dt_te:
      r = E50X(load_te)(cpu, far, digits);
      break;

    default:
      abort();
  }

  if(sign)
    r = -r;

  return r;
}

static inline void E50X(store_decimal)(cpu_t *cpu, dcw_t dcw, int far, int64_t value)
{
int type = dcw_type(dcw, far);
int abs = dcw_abs(dcw);
int digits = dcw_digits(dcw, far);

  if(value < 0 && abs)
    value = -value;

  switch(type) {

    case dcw_dt_ls:
      E50X(store_ls)(cpu, far, digits, value);
      break;

    case dcw_dt_ts:
      E50X(store_ts)(cpu, far, digits, value);
      break;

    case dcw_dt_pd:
      E50X(store_pd)(cpu, far, digits, value);
      break;

    case dcw_dt_le:
      E50X(store_le)(cpu, far, digits, value);
      break;

    case dcw_dt_te:
      E50X(store_te)(cpu, far, digits, value);
      break;

    default:
      abort();
  }
}

#define XED_ZS  000
#define XED_IL  001
#define XED_SS  002
#define XED_ICS 003
#define XED_ID  004
#define XED_ICM 005
#define XED_ICP 006
#define XED_SFC 007
#define XED_SFP 010
#define XED_SFM 011
#define XED_SFS 012
#define XED_JZ  013
#define XED_FS  014
#define XED_SF  015
#define XED_IS  016
#define XED_SD  017
#define XED_EBS 020

static inline void E50X(xed_zs)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("decimal xed Zero Suppress %d digits\n", m);

  if(m == 0)
    return;

  uint8_t d[m+1];

  E50X(load_bytes)(cpu, d, 0, m, update);

  int n;
  for(n = 0; !iv->sig && n < m && ((d[n] & 0x7f) == '0' || (d[n] & 0x7f) == '+' || (d[n] & 0x7f) == '-'); ++n)
    d[n] = iv->sc;
  if(n < m)
  {
    if(((n > 0) | !iv->sp) && iv->fc)
    {
      for(int i = m; i > n; --i)
        d[i] = d[i-1];
      d[n] = iv->fc;
      iv->sp = 1;
      ++m;
    }
    iv->sig = 1;
  }

  E50X(store_bytes)(cpu, d, 1, m, update);
}

static inline void E50X(xed_il)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("decimal xed Insert Literal 0x%02x '%c'\n", m, m & 0x7f);

  E50X(store_bytes)(cpu, &m, 1, 1, update);
}

static inline void E50X(xed_ss)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("decimal xed Set suppression character 0x%02x '%c'\n", m, m & 0x7f);
  iv->sc = m;
}

static inline void E50X(xed_ics)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("decimal xed Insert Character 0x%02x '%c'\n", m, m & 0x7f);
  E50X(store_bytes)(cpu, iv->sig ? &m : &iv->sc, 1, 1, update);
}

static inline void E50X(xed_id)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("decimal xed Insert %d digits\n", m);

  if((!iv->sp || !iv->sig) && iv->fc)
  {
  uint8_t d[m + 1];
    d[0] = iv->fc;
    iv->sp = 1;
    E50X(load_bytes)(cpu, d + 1, 0, m, update);
    E50X(store_bytes)(cpu, d, 1, m + 1, update);
  }
  else
  {
  uint8_t d[m];
    E50X(load_bytes)(cpu, d, 0, m, update);
    E50X(store_bytes)(cpu, d, 1, m, update);
  }
  iv->sig = 1;
}

static inline void E50X(xed_icm)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Insert Character 0x%02x '%c' if minus\n", m, m & 0x7f);
  E50X(store_bytes)(cpu, iv->sign ? &m : &iv->sc, 1, 1, update);
  iv->sp = 1;
}

static inline void E50X(xed_icp)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Insert Character 0x%02x '%c' if plus\n", m, m & 0x7f);
  E50X(store_bytes)(cpu, iv->sign ? &iv->sc : &m, 1, 1, update);
  iv->sp = 1;
}

static inline void E50X(xed_sfc)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("decimal xed Set floating character 0x%02x '%c'\n", m, m & 0x7f);
  iv->fc = m;
}

static inline void E50X(xed_sfp)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Set floating character 0x%02x '%c' if plus\n", m, m & 0x7f);
  iv->fc = iv->sign ? iv->sc : m;
}

static inline void E50X(xed_sfm)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Set floating character 0x%02x '%c' if minus\n", m, m & 0x7f);
  iv->fc = iv->sign ? m : iv->sc;
}

static inline void E50X(xed_sfs)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Set floating to sign\n");
  if(iv->sign)
    iv->fc = 0255; // '-'
  else
    iv->fc = 0253; // '+'
}

static inline void E50X(xed_jz)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Jump if zero\n");
  if(!iv->sc)
    *spa += m;
}

static inline void E50X(xed_fs)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Fill with %d suppression characters\n", m);
  uint8_t d[m];
  memset(d, iv->sc, m);
  E50X(store_bytes)(cpu, d, 1, m, update);
}

static inline void E50X(xed_sf)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Set significance\n");
  if(iv->fc)
  {
    E50X(store_bytes)(cpu, &iv->fc, 1, 1, update);
    iv->sp = 1;
  }
  iv->sig = 1;
}

static inline void E50X(xed_is)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Insert Sign\n");
  uint8_t s = iv->sign ? 0255 : 0253;
  E50X(store_bytes)(cpu, &s, 1, 1, update);
}

static inline void E50X(xed_sd)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("*decimal xed Suppress %d digits\n", m);
  uint8_t d[m];
  E50X(load_bytes)(cpu, d, 0, m, update);

  if(!iv->sig)
    for(int n = 0; n < m && (d[n] & 0x7f) == '0'; ++n)
      d[n] = iv->sc;
  else
    for(int n = 0; n < m; ++n)
      if((d[n] & 0x7f) == '0')
        d[n] = iv->sc;

  E50X(store_bytes)(cpu, d, 1, m, update);
}

static inline void E50X(xed_ebs)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("decimal xed Embed sign for %d digits\n", m);
  uint8_t d[m];
  E50X(load_bytes)(cpu, d, 0, m, update);

  if(iv->sign)
    for(int n = 0; n < m; ++n)
      d[n] = ((d[n] & 0x7f) == '0') ? 0375 /* '}' */ : d[n] + 'I' - '0';

  E50X(store_bytes)(cpu, d, 1, m, update);
}


E50I(xmv)
{
dcw_t cw = {.w = G_L(cpu) };

#ifdef DEBUG
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 1);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "*xmv");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  int64_t value = E50X(load_decimal)(cpu, cw, 0);

  value = dcw_scale(cw, 1, value);

  E50X(store_decimal)(cpu, cw, 1, value);
}


E50I(xdtb)
{
dcw_t cw = {.w = G_L(cpu) };

#ifdef DEBUG
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "*xdtb");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  int64_t r = E50X(load_decimal)(cpu, cw, 0);

  switch(cw.h) {
    case 0b00:
      S_A(cpu, r);
      break;
    case 0b01:
      S_L(cpu, r);
      break;
    default:
      logall("xdtb h %d invalid\n", cw.h);
    case 0b10:
      S_L(cpu, r >> 32);
      S_E(cpu, r & 0xffffffff);
      break;
  }
}


E50I(xbtd)
{
dcw_t cw = {.w = G_L(cpu) };

#ifdef DEBUG
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "*xbtd");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  int64_t r;

  switch(cw.h) {
    case 0b00:
      r = (int16_t)G_EH(cpu);
      break;
    case 0b01:
      r = (int32_t)G_E(cpu);
      break;
    default:
      logall("xbtd h %d invalid\n", cw.h);
    case 0b10:
      r = (int64_t)G_DAC(cpu, 1);
      break;
  }

  E50X(store_decimal)(cpu, cw, 0, r);
}


E50I(xed)
{
uint32_t spa = G_XB(cpu);
uint16_t sp;
xed_iv iv = { .sc = cpu->crs->km.ascii ? 040: 0240, .fc = 0, .sp = 0, .sign = 0, .sig = 0 };

#ifdef DEBUG
dcw_t cw = {.w = G_L(cpu) };
uint32_t far0 = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "*xed");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, srb, srl, far1, dsb, dsl);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  logmsg("todo\n");

  uint8_t s;
  E50X(load_bytes)(cpu, &s, 0, 1, update);
  if((s & 0x7f) == '-')
    iv.sign = 1;

  do {
    sp = E50X(vfetch_w)(cpu, spa++);
    int op = (sp >> 8) & 037;
    uint8_t m = sp & 0377;

    logmsg("-> sp = %4.4x: %o %o %o sig = %d\n", sp, sp >> 15, op, m, iv.sig);

    switch(op) {
      case XED_ZS:
        E50X(xed_zs)(cpu, &iv, &spa, m);
        break;
      case XED_IL:
        E50X(xed_il)(cpu, &iv, &spa, m);
        break;
      case XED_SS:
        E50X(xed_ss)(cpu, &iv, &spa, m);
        break;
      case XED_ICS:
        E50X(xed_ics)(cpu, &iv, &spa, m);
        break;
      case XED_ID:
        E50X(xed_id)(cpu, &iv, &spa, m);
        break;
      case XED_ICM:
        E50X(xed_icm)(cpu, &iv, &spa, m);
        break;
      case XED_ICP:
        E50X(xed_icp)(cpu, &iv, &spa, m);
        break;
      case XED_SFC:
        E50X(xed_sfc)(cpu, &iv, &spa, m);
        break;
      case XED_SFP:
        E50X(xed_sfp)(cpu, &iv, &spa, m);
        break;
      case XED_SFM:
        E50X(xed_sfm)(cpu, &iv, &spa, m);
        break;
      case XED_SFS:
        E50X(xed_sfs)(cpu, &iv, &spa, m);
        break;
      case XED_JZ:
        E50X(xed_jz)(cpu, &iv, &spa, m);
        break;
      case XED_FS:
        E50X(xed_fs)(cpu, &iv, &spa, m);
        break;
      case XED_SF:
        E50X(xed_sf)(cpu, &iv, &spa, m);
        break;
      case XED_IS:
        E50X(xed_is)(cpu, &iv, &spa, m);
        break;
      case XED_SD:
        E50X(xed_sd)(cpu, &iv, &spa, m);
        break;
      case XED_EBS:
        E50X(xed_ebs)(cpu, &iv, &spa, m);
        break;
      default:;
    }

    S_XB(cpu, spa);

  } while(!(sp & 0x8000));
}


E50I(xad)
{
dcw_t cw = {.w = G_L(cpu) };

#ifdef DEBUG
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "*xad");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  int64_t v1 = E50X(load_decimal)(cpu, cw, 0);
  int64_t v2 = E50X(load_decimal)(cpu, cw, 1);

logmsg("xad v1 %jd v2 %jd\n", (intmax_t)v1, (intmax_t)v2);
  v1 = dcw_scale(cw, 0, v1);

  int64_t r = v1 + v2;

logmsg("xad v1 %jd v2 %jd r %jd\n", (intmax_t)v1, (intmax_t)v2, (intmax_t)r);
  E50X(store_decimal)(cpu, cw, 1, r);
}


E50I(xcm)
{
dcw_t cw = {.w = G_L(cpu) };

#ifdef DEBUG
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "*xcm");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  int64_t v1 = E50X(load_decimal)(cpu, cw, 0);
  int64_t v2 = E50X(load_decimal)(cpu, cw, 1);

  if(v2 != 0)
    v1 = dcw_scale(cw, 1, v1);

  _SET_CC(cpu, v2, v1);
}


E50I(xdv)
{
dcw_t cw = {.w = G_L(cpu) };

#ifdef DEBUG
uint32_t src = G_FAR(cpu, 0);
int srb = G_FBR(cpu, 0);
int srl = G_FLR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int dsb = G_FBR(cpu, 1);
int dsl = G_FLR(cpu, 1);

  logop1(op, "*xdv");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  int64_t value = E50X(load_decimal)(cpu, cw, 0);

  value = dcw_scale(cw, 0, value);

  int div = E50X(load_decimal)(cpu, cw, 1);
  if(div) // TODO
    value /= div;

  E50X(store_decimal)(cpu, cw, 1, value);
}


E50I(xmp)
{
dcw_t cw = {.w = G_L(cpu) };

#ifdef DEBUG
uint32_t src = G_FAR(cpu, 0);
uint32_t dst = G_FAR(cpu, 1);
int srb = G_FBR(cpu, 0);
int dsl = G_FLR(cpu, 1);
int srl = G_FLR(cpu, 0);
int dsb = G_FBR(cpu, 1);

  logop1(op, "*xmp");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", src, srb, srl, dst, dsb, dsl);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  int64_t value = E50X(load_decimal)(cpu, cw, 0);

  value = dcw_scale(cw, 0, value);

  value *= E50X(load_decimal)(cpu, cw, 1);

  E50X(store_decimal)(cpu, cw, 1, value);
}


#ifndef EMDE
 #include __FILE__
#endif
