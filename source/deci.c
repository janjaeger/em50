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

typedef __int128_t intdec_t;

#define dcw_dt_ls 0   /* Leading Separate */
#define dcw_dt_ts 1   /* Trailing Separate */
#define dcw_dt_pd 3   /* Packed Decimal */
#define dcw_dt_le 4   /* Leading Embedded */
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
assert_size(dcw_t, 4);

typedef struct {
  uint8_t fc; // Floating character in AH
  uint8_t cf; // Condition flag in AL
  uint8_t br; // Bytes remaining in BH
  uint8_t sc; // Suppression character in BL
  uint8_t sp;
  int sign;
  int sig;
} xed_iv;

static inline void xed_get_iv(cpu_t *cpu, xed_iv *iv)
{
uint16_t a = G_A(cpu);
uint16_t b = G_B(cpu);

  iv->fc = a >> 8;
  iv->cf = a & 0xff;
  iv->br = b >> 8;
  iv->sc = b & 0xff;
}

static inline void xed_set_iv(cpu_t *cpu, xed_iv *iv)
{
uint16_t a = iv->fc << 8 | iv->cf;
uint16_t b = iv->br << 8 | iv->sc;

  S_A(cpu, a);
  S_B(cpu, b);
}

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

static inline intdec_t dcw_scale(dcw_t dcw, int xmv, intdec_t value)
{
  int16_t scale = (dcw.g & 0x40) ? dcw.g | 0xff80 : dcw.g;

//printf("scale %d, value %jd\n", scale, (intmax_t)value);
  if(scale < 0)
  {
    intdec_t mult = pow(10, -scale);
    value *= mult;
  }
  else if(scale > 0)
  {
//static const int adj[] = { 1, 0, 1, 0, 0 };
//int lz = value ? dcw.f - log10(value >= 0 ? value : -value) - adj[dcw.e] : 0;
//if(lz) printf("lz %d v %jd\n", lz, (intmax_t)value);
//    if(!xmv && lz < scale)
//      scale += lz;
    intdec_t div = pow(10, scale < 40 ? scale : 39);
    if(!div) { div = 1; value = 0;}
// THIS IS WRONG FOR XCM, WHICH SHOULD IGNORE DCW.D
    intdec_t rnd = (!xmv || (xmv && dcw.d)) ? div / 2 : 0;
//  intdec_t rnd = (xmv && dcw.d) ? div / 2 : 0;
    value = (value + (value > 0 ? rnd : -rnd)) / div;
//printf("value %jd\n", (intmax_t)value);
  }

  return value;
}

#endif


#define noupd 0
#define update 1
static inline void E50X(store_bytes)(cpu_t *cpu, uint8_t *bytes, int count, uint32_t *far, int *fbr)
{
  if(count == 0)
    return;

  if(*fbr != 0)
  {
  uint16_t w = (E50X(vfetch_w)(cpu, *far) & 0xff00) | *bytes++;
    E50X(vstore_w)(cpu, (*far), w);
    inc_d(far);
    --count;
    *fbr = 0;
  }

  for(int n = count / 2; n > 0; --n, count -= 2)
  {
  uint16_t w = *bytes++ << 8;
           w |= *bytes++;
    E50X(vstore_w)(cpu, (*far), w);
    inc_d(far);
  }

  if(count > 0)
  {
  uint16_t w = (E50X(vfetch_w)(cpu, *far) & 0x00ff) | (*bytes << 8);
    E50X(vstore_w)(cpu, *far, w);
    *fbr = 8;
  }
}

static inline void E50X(store_ls)(cpu_t *cpu, int digits, intdec_t value, uint32_t *far, int *fbr)
{
uint8_t ls[digits + 1];

  logmsg("decimal E50X(store_ls) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)value);

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

  E50X(store_bytes)(cpu, ls, digits + 1, far, fbr);
}

static inline void E50X(store_ts)(cpu_t *cpu, int digits, intdec_t value, uint32_t *far, int *fbr)
{
uint8_t ls[digits + 1];

  logmsg("decimal E50X(store_ts) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)value);

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

  E50X(store_bytes)(cpu, ls, digits + 1, far, fbr);
}

static inline void E50X(store_pd)(cpu_t *cpu, int digits, intdec_t value, uint32_t *far, int *fbr)
{
  logmsg("decimal E50X(store_pd) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)value);

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

  E50X(store_bytes)(cpu, pd, bytes, far, fbr);
}

static inline void E50X(store_le)(cpu_t *cpu, int digits, intdec_t value, uint32_t *far, int *fbr)
{
uint8_t ls[digits];

  logmsg("decimal E50X(store_le) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)value);

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

  E50X(store_bytes)(cpu, ls, digits, far, fbr);
}

static inline void E50X(store_te)(cpu_t *cpu, const int digits, intdec_t value, uint32_t *far, int *fbr)
{
uint8_t ls[digits];

  logmsg("decimal E50X(store_te) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)value);

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

  E50X(store_bytes)(cpu, ls, digits, far, fbr);
}

static inline void E50X(load_bytes)(cpu_t *cpu, uint8_t *bytes, int count, uint32_t *far, int *fbr)
{
  if(count == 0)
    return;

  if(*fbr != 0)
  {
    *bytes++ = E50X(vfetch_w)(cpu, (*far)) & 0xff;
    inc_d(far);
    --count;
    *fbr = 0;
  }

  for(int n = count / 2; n > 0; --n, count -= 2)
  {
  uint16_t w = E50X(vfetch_w)(cpu, (*far));
    inc_d(far);
    *bytes++ = w >> 8;
    *bytes++ = w & 0xff;
  }

  if(count > 0)
  {
    *bytes = E50X(vfetch_w)(cpu, *far) >> 8;
    *fbr = 8;
  }
}

static inline intdec_t E50X(load_ls)(cpu_t *cpu, int digits, uint32_t *far, int *fbr)
{
intdec_t r = 0;
uint8_t ls[digits + 1];

  E50X(load_bytes)(cpu, ls, digits + 1, far, fbr);
  for(int n = 1; n <= digits; n++)
  {
    r *= 10;
    r += ls[n] & 0xf;
  }
  // space = 0x20, + = 0x2b, { = 0x7b 1011
  //               - = 0x2d, } = 0x7d 1101
  if((ls[0] & 0b100))
    r = -r;

  logmsg("decimal E50X(load_ls) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)r);

  return r;
}

static inline intdec_t E50X(load_ts)(cpu_t *cpu, int digits, uint32_t *far, int *fbr)
{
intdec_t r = 0;
uint8_t ts[digits + 1];

  E50X(load_bytes)(cpu, ts, digits + 1, far, fbr);
  for(int n = 0; n < digits; n++)
  {
    r *= 10;
    r += ts[n] & 0xf;
  }
  // space = 0x20, + = 0x2b, { = 0x7b 1011
  //               - = 0x2d, } = 0x7d 1101
  if((ts[digits] & 0b100))
    r = -r;

  logmsg("decimal E50X(load_ts) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)r);

  return r;
}

static inline intdec_t E50X(load_pd)(cpu_t *cpu, int digits, uint32_t *far, int *fbr)
{
  if(!digits)
    return 0;

  if(!(digits & 1))
    ++digits;

  int bytes = (digits + 1) / 2;
  uint8_t pd[bytes];

  E50X(load_bytes)(cpu, pd, bytes, far, fbr);

  intdec_t value = 0;

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

  logmsg("decimal E50X(load_pd) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)value);

  return value;
}

static inline intdec_t E50X(load_le)(cpu_t *cpu, int digits, uint32_t *far, int *fbr)
{
intdec_t r;
uint8_t le[digits];

  E50X(load_bytes)(cpu, le, digits, far, fbr);

  uint8_t t = le[0] & 0x7f;
  int neg = t == '}' || t == '-' || (t >= 'J' && t <= 'R');
  r = ((t >= 'J' && t <= 'R') ? t - 'I' : (t == '}' || t == '-' || t == '{' || t == '+') ? 0 : t) & 0xf;
  for(int n = 1; n < digits; n++)
  {
    r *= 10;
    r += le[n] & 0xf;
  }
  // space = 0x20, + = 0x2b, { = 0x7b 1011
  //               - = 0x2d, } = 0x7d 1101
  if(neg)
    r = -r;

  logmsg("decimal E50X(load_le) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)r);

  return r;
}

static inline intdec_t E50X(load_te)(cpu_t *cpu, int digits, uint32_t *far, int *fbr)
{
intdec_t r = 0;
uint8_t te[digits];

  E50X(load_bytes)(cpu, te, digits, far, fbr);

  for(int n = 0; n < digits - 1; n++)
  {
    r += te[n] & 0xf;
    r *= 10;
  }
  // space = 0x20, + = 0x2b, { = 0x7b 1011
  //               - = 0x2d, } = 0x7d 1101

  uint8_t l = te[digits - 1] & 0x7f;
  int neg = l == '}' || l == '-' || (l >= 'J' && l <= 'R');
  r += ((l >= 'J' && l <= 'R') ? l - 'I' : (l == '}' || l == '-' || l == '{' || l == '+') ? 0 : l) & 0xf;

  if(neg)
    r = -r;

  logmsg("decimal E50X(load_te) far %8.8x digits %d value %jd\n", *far, digits, (intmax_t)r);

  return r;
}

static inline intdec_t E50X(load_decimal)(cpu_t *cpu, dcw_t dcw, int far, uint32_t *far0, int *fbr0)
{
int type = dcw_type(dcw, far);
int sign = dcw_sign(dcw, far);
int digits = dcw_digits(dcw, far);

intdec_t r;

  if(!digits)
    return 0;

  switch(type) {

    case dcw_dt_ls:
      r = E50X(load_ls)(cpu, digits, far0, fbr0);
      break;

    case dcw_dt_ts:
      r = E50X(load_ts)(cpu, digits, far0, fbr0);
      break;

    case dcw_dt_pd:
      r = E50X(load_pd)(cpu, digits, far0, fbr0);
      break;

    case dcw_dt_le:
      r = E50X(load_le)(cpu, digits, far0, fbr0);
      break;

    case dcw_dt_te:
      r = E50X(load_te)(cpu, digits, far0, fbr0);
      break;

    default:
      abort();
  }

  if(sign)
    r = -r;

  return r;
}

static inline void E50X(store_decimal)(cpu_t *cpu, dcw_t dcw, int far, intdec_t value, uint32_t *far1, int *fbr1)
{
int type = dcw_type(dcw, far);
int abs = dcw_abs(dcw);
int digits = dcw_digits(dcw, far);

  if(value < 0 && abs)
    value = -value;

  switch(type) {

    case dcw_dt_ls:
      E50X(store_ls)(cpu, digits, value, far1, fbr1);
      break;

    case dcw_dt_ts:
      E50X(store_ts)(cpu, digits, value, far1, fbr1);
      break;

    case dcw_dt_pd:
      E50X(store_pd)(cpu, digits, value, far1, fbr1);
      break;

    case dcw_dt_le:
      E50X(store_le)(cpu, digits, value, far1, fbr1);
      break;

    case dcw_dt_te:
      E50X(store_te)(cpu, digits, value, far1, fbr1);
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

static inline void E50X(xed_zs)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far0, int *fbr0, uint32_t *far1, int *fbr1)
{
  logmsg("decimal xed Zero Suppress %d digits\n", m);

  if(m == 0)
    return;

  uint8_t d[m+1];

  E50X(load_bytes)(cpu, d, m, far0, fbr0);

  int n;
  for(n = 0; !iv->sig && n < m && ((d[n] & 0x7f) == '0' || (d[n] & 0x7f) == '+' || (d[n] & 0x7f) == '-'); ++n)
    d[n] = iv->sc;
  if(n < m)
  {
    if(((n > 0) | !iv->cf) && iv->fc)
    {
      for(int i = m; i > n; --i)
        d[i] = d[i-1];
      d[n] = iv->fc;
      iv->cf = 1;
      ++m;
    }
    iv->sig = 1;
  }

  E50X(store_bytes)(cpu, d, m, far1, fbr1);
}

static inline void E50X(xed_il)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far1, int *fbr1)
{
  logmsg("decimal xed Insert Literal 0x%02x '%c'\n", m, m & 0x7f);

  E50X(store_bytes)(cpu, &m, 1, far1, fbr1);
}

static inline void E50X(xed_ss)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m)
{
  logmsg("decimal xed Set suppression character 0x%02x '%c'\n", m, m & 0x7f);
  iv->sc = m;
}

static inline void E50X(xed_ics)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far1, int *fbr1)
{
  logmsg("decimal xed Insert Character 0x%02x '%c'\n", m, m & 0x7f);
  E50X(store_bytes)(cpu, iv->sig ? &m : &iv->sc, 1, far1, fbr1);
}

static inline void E50X(xed_id)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far0, int *fbr0, uint32_t *far1, int *fbr1)
{
  logmsg("decimal xed Insert %d digits\n", m);

  if((!iv->cf || !iv->sig) && iv->fc)
  {
  uint8_t d[m + 1];
    d[0] = iv->fc;
    iv->cf = 1;
    E50X(load_bytes)(cpu, d + 1, m, far0, fbr0);
    E50X(store_bytes)(cpu, d, m + 1, far1, fbr1);
  }
  else
  {
  uint8_t d[m];
    E50X(load_bytes)(cpu, d, m, far0, fbr0);
    E50X(store_bytes)(cpu, d, m, far1, fbr1);
  }
  iv->sig = 1;
}

static inline void E50X(xed_icm)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far1, int *fbr1)
{
  logmsg("*decimal xed Insert Character 0x%02x '%c' if minus\n", m, m & 0x7f);
  E50X(store_bytes)(cpu, iv->sign ? &m : &iv->sc, 1, far1, fbr1);
  iv->cf = 1;
}

static inline void E50X(xed_icp)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far1, int *fbr1)
{
  logmsg("*decimal xed Insert Character 0x%02x '%c' if plus\n", m, m & 0x7f);
  E50X(store_bytes)(cpu, iv->sign ? &iv->sc : &m, 1, far1, fbr1);
  iv->cf = 1;
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
  logmsg("*decimal xed Jump if zero (%d)\n", iv->cf);
  if(iv->cf)
    *spa += m;
}

static inline void E50X(xed_fs)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far1, int *fbr1)
{
  logmsg("*decimal xed Fill with %d suppression characters\n", m);
  uint8_t d[m];
  memset(d, iv->sc, m);
  E50X(store_bytes)(cpu, d, m, far1, fbr1);
}

static inline void E50X(xed_sf)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far1, int *fbr1)
{
  logmsg("*decimal xed Set significance\n");
  if(iv->fc)
  {
    E50X(store_bytes)(cpu, &iv->fc, 1, far1, fbr1);
    iv->cf = 1;
  }
  iv->sig = 1;
}

static inline void E50X(xed_is)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far1, int *fbr1)
{
  logmsg("*decimal xed Insert Sign\n");
  uint8_t s = iv->sign ? 0255 : 0253;
  E50X(store_bytes)(cpu, &s, 1, far1, fbr1);
}

static inline void E50X(xed_sd)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far0, int *fbr0, uint32_t *far1, int *fbr1)
{
  logmsg("*decimal xed Suppress %d digits\n", m);
  uint8_t d[m];
  E50X(load_bytes)(cpu, d, m, far0, fbr0);

  if(!iv->sig)
    for(int n = 0; n < m && (d[n] & 0x7f) == '0'; ++n)
      d[n] = iv->sc;
  else
    for(int n = 0; n < m; ++n)
      if((d[n] & 0x7f) == '0')
        d[n] = iv->sc;

  E50X(store_bytes)(cpu, d, m, far1, fbr1);
}

static inline void E50X(xed_ebs)(cpu_t *cpu, xed_iv *iv, uint32_t *spa, uint8_t m, uint32_t *far0, int *fbr0, uint32_t *far1, int *fbr1)
{
  logmsg("decimal xed Embed sign for %d digits\n", m);
  uint8_t d[m];
  E50X(load_bytes)(cpu, d, m, far0, fbr0);

  if(iv->sign)
    for(int n = 0; n < m; ++n)
      d[n] = ((d[n] & 0x7f) == '0') ? 0375 /* '}' */ : d[n] + 'I' - '0';

  E50X(store_bytes)(cpu, d, m, far1, fbr1);
}


E50I(xmv)
{
dcw_t cw = {.w = G_L(cpu) };

uint32_t far0 = G_FAR(cpu, 0);
int fbr0 = G_FBR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int fbr1 = G_FBR(cpu, 1);
#ifdef DEBUG
int flr0 = G_FLR(cpu, 0);
int flr1 = G_FLR(cpu, 1);

  logop1(op, "*xmv");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, fbr0, flr0, far1, fbr1, flr1);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  intdec_t value = E50X(load_decimal)(cpu, cw, 0, &far0, &fbr0);

  value = dcw_scale(cw, 1, value);

  E50X(store_decimal)(cpu, cw, 1, value, &far1, &fbr1);

  S_FAR(cpu, 0, far0);
  S_FBR(cpu, 0, fbr0);
  S_FAR(cpu, 1, far1);
  S_FBR(cpu, 1, fbr1);
}


E50I(xdtb)
{
dcw_t cw = {.w = G_L(cpu) };

uint32_t far0 = G_FAR(cpu, 0);
int fbr0 = G_FBR(cpu, 0);
#ifdef DEBUG
uint32_t far1 = G_FAR(cpu, 1);
int fbr1 = G_FBR(cpu, 1);
int flr0 = G_FLR(cpu, 0);
int flr1 = G_FLR(cpu, 1);

  logop1(op, "*xdtb");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, fbr0, flr0, far1, fbr1, flr1);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  intdec_t r = E50X(load_decimal)(cpu, cw, 0, &far0, &fbr0);

  switch(cw.h) {
    case 0b00:
      S_A(cpu, r);
      break;
    case 0b01:
      S_L(cpu, r);
      break;
    default:
      logall("xdtb h %d invalid\n", cw.h);
      break;
    case 0b10:
      S_L(cpu, r >> 32);
      S_E(cpu, r & 0xffffffff);
      break;
  }
}


E50I(xbtd)
{
dcw_t cw = {.w = G_L(cpu) };

uint32_t far0 = G_FAR(cpu, 0);
int fbr0 = G_FBR(cpu, 0);
#ifdef DEBUG
int flr0 = G_FLR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int fbr1 = G_FBR(cpu, 1);
int flr1 = G_FLR(cpu, 1);

  logop1(op, "*xbtd");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, fbr0, flr0, far1, fbr1, flr1);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  intdec_t r;

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

  E50X(store_decimal)(cpu, cw, 0, r, &far0, &fbr0);

  S_FAR(cpu, 0, far0);
  S_FBR(cpu, 0, fbr0);
}


E50I(xed)
{
uint32_t spa = G_XB(cpu);
uint16_t sp;
xed_iv iv = { 0 };
xed_get_iv(cpu, &iv);
iv.sc = cpu->crs->km.ascii ? 040: 0240;

uint32_t far0 = G_FAR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int fbr0 = G_FBR(cpu, 0);
int fbr1 = G_FBR(cpu, 1);
#ifdef DEBUG
dcw_t cw = {.w = G_L(cpu) };
int flr0 = G_FLR(cpu, 0);
int flr1 = G_FLR(cpu, 1);

  logop1(op, "*xed");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, fbr0, flr0, far1, fbr1, flr1);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  uint8_t s;
  E50X(load_bytes)(cpu, &s, 1, &far0, &fbr0);
  if((s & 0x7f) == '-')
    iv.sign = 1;

  do {
    sp = E50X(vfetch_w)(cpu, spa);
    inc_d(&spa);
    int op = (sp >> 8) & 037;
    uint8_t m = sp & 0377;

    logmsg("-> sp = %4.4x: %o %o %o sig = %d\n", sp, sp >> 15, op, m, iv.sig);

    switch(op) {
      case XED_ZS:
        E50X(xed_zs)(cpu, &iv, &spa, m, &far0, &fbr0, &far1, &fbr1);
        break;
      case XED_IL:
        E50X(xed_il)(cpu, &iv, &spa, m, &far1, &fbr1);
        break;
      case XED_SS:
        E50X(xed_ss)(cpu, &iv, &spa, m);
        break;
      case XED_ICS:
        E50X(xed_ics)(cpu, &iv, &spa, m, &far1, &fbr1);
        break;
      case XED_ID:
        E50X(xed_id)(cpu, &iv, &spa, m, &far0, &fbr0, &far1, &fbr1);
        break;
      case XED_ICM:
        E50X(xed_icm)(cpu, &iv, &spa, m, &far1, &fbr1);
        break;
      case XED_ICP:
        E50X(xed_icp)(cpu, &iv, &spa, m, &far1, &fbr1);
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
        E50X(xed_fs)(cpu, &iv, &spa, m, &far1, &fbr1);
        break;
      case XED_SF:
        E50X(xed_sf)(cpu, &iv, &spa, m, &far1, &fbr1);
        break;
      case XED_IS:
        E50X(xed_is)(cpu, &iv, &spa, m, &far1, &fbr1);
        break;
      case XED_SD:
        E50X(xed_sd)(cpu, &iv, &spa, m, &far0, &fbr0, &far1, &fbr1);
        break;
      case XED_EBS:
        E50X(xed_ebs)(cpu, &iv, &spa, m, &far0, &fbr0, &far1, &fbr1);
        break;
      default:;
    }

    S_XB(cpu, spa);
    xed_set_iv(cpu, &iv);

  } while(!(sp & 0x8000));

}


E50I(xad)
{
dcw_t cw = {.w = G_L(cpu) };

uint32_t far0 = G_FAR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int fbr0 = G_FBR(cpu, 0);
int fbr1 = G_FBR(cpu, 1);
#ifdef DEBUG
int flr0 = G_FLR(cpu, 0);
int flr1 = G_FLR(cpu, 1);

  logop1(op, "*xad");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, fbr0, flr0, far1, fbr1, flr1);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  intdec_t v1 = E50X(load_decimal)(cpu, cw, 0, &far0, &fbr0);
  uint32_t farx = far1;
  int fbrx = fbr1;
  intdec_t v2 = E50X(load_decimal)(cpu, cw, 1, &farx, &fbrx);

  v1 = dcw_scale(cw, 0, v1);

#if 0
  if(cw.b)
    v1 = pow(10, cw.a) + v1;
#endif

  intdec_t r = v1 + v2;

  E50X(store_decimal)(cpu, cw, 1, r, &far1, &fbr1);

  SET_CC(cpu, r);
}


E50I(xcm)
{
dcw_t cw = {.w = G_L(cpu) };

uint32_t far0 = G_FAR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int fbr0 = G_FBR(cpu, 0);
int fbr1 = G_FBR(cpu, 1);
#ifdef DEBUG
int flr0 = G_FLR(cpu, 0);
int flr1 = G_FLR(cpu, 1);

  logop1(op, "*xcm");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, fbr0, flr0, far1, fbr1, flr1);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  intdec_t v1 = E50X(load_decimal)(cpu, cw, 0, &far0, &fbr0);
  intdec_t v2 = E50X(load_decimal)(cpu, cw, 1, &far1, &fbr1);

  if(v2 != 0)
    v1 = dcw_scale(cw, 1, v1);

  _SET_CC(cpu, v2, v1);
}


E50I(xdv)
{
dcw_t cw = {.w = G_L(cpu) };

uint32_t far0 = G_FAR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int fbr0 = G_FBR(cpu, 0);
int fbr1 = G_FBR(cpu, 1);
#ifdef DEBUG
int flr0 = G_FLR(cpu, 0);
int flr1 = G_FLR(cpu, 1);

  logop1(op, "*xdv");
  logmsg("-> far0: %8.8x flr %8.8x fbr %d\n", G_FAR(cpu, 0), G_FLR(cpu, 0), G_FBR(cpu, 0));
  logmsg("-> far1: %8.8x flr %8.8x fbr %d\n", G_FAR(cpu, 1), G_FLR(cpu, 1), G_FBR(cpu, 1));
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, fbr0, flr0, far1, fbr1, flr1);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

//__int128_t div = E50X(load_decimal)(cpu, cw, 0);
  intdec_t div = E50X(load_decimal)(cpu, cw, 0, &far0, &fbr0);

  div = dcw_scale(cw, 0, div);

  if(!div) // TODO
    return;

  uint32_t farx = far1;
  int fbrx = fbr1;
  intdec_t value = E50X(load_decimal)(cpu, cw, 1, &farx, &fbrx);

  int ql, rl;
  if(cw.a < cw.f)
  {
    ql = cw.f - cw.a;
    rl = cw.a;
  }
  else
  {
    ql = cw.f;
    rl = 0;
  }
  

  intdec_t quotient = value / div;

  cw.f = ql;
  E50X(store_decimal)(cpu, cw, 1, quotient, &far1, &fbr1);

#if 0
  far1 += cw.f / 2;
//S_FAR(cpu, 1, far1);
  if((cw.f % 2))
    fbr1 = fbr1 ? 0 : 8;
//S_FBR(cpu, 1, fbr1);
#endif

  intdec_t remainder = value % div;

  if(rl)
  {
    cw.f = rl;
    E50X(store_decimal)(cpu, cw, 1, remainder, &far1, &fbr1);
  }

logmsg("val %jd, div %jd, qnt %jd, rem %jd\n", (intmax_t)value, (intmax_t)div, (intmax_t)quotient, (intmax_t)remainder);
}


E50I(xmp)
{
dcw_t cw = {.w = G_L(cpu) };

uint32_t far0 = G_FAR(cpu, 0);
uint32_t far1 = G_FAR(cpu, 1);
int fbr0 = G_FBR(cpu, 0);
int fbr1 = G_FBR(cpu, 1);
#ifdef DEBUG
int flr0 = G_FLR(cpu, 0);
int flr1 = G_FLR(cpu, 1);

  logop1(op, "*xmp");
  logmsg("-> %8.8x+%x %x %8.8x+%x %x\n", far0, fbr0, flr0, far1, fbr1, flr1);
  logmsg("-> cw %8.8x f1l %d f1s %d f2s %d rs %d round %d f1t %d f2l %d scale %d f2t %d\n",
                cw.w, cw.a,  cw.b,  cw.c,  cw.t, cw.d,    cw.e,  cw.f,  cw.g,    cw.h);
#endif

  intdec_t value = E50X(load_decimal)(cpu, cw, 0, &far0, &fbr0);

//value = dcw_scale(cw, 0, value);

  uint32_t farx = far1;
  int fbrx = fbr1;
  intdec_t multiplier = E50X(load_decimal)(cpu, cw, 1, &farx, &fbrx);

  intdec_t result = value * multiplier;

  E50X(store_decimal)(cpu, cw, 1, result, &far1, &fbr1);
logmsg("val %jd, mul %jd, res %jd\n", (intmax_t)value, (intmax_t)multiplier, (intmax_t)result);
}


#ifndef EMDE
 #include __FILE__
#endif
