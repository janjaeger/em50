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


//        fraction exponent  fraction+  unused
// single     1-24    25-32
// double     1-48    49-64
// quad       1-48    49-64  65-112    113-128


// value = fraction * 2^(exponent - 128)

// fraction = two's complement value

// exponent = 128 - trueexponent

#ifndef _flpt_h
#define _flpt_h


#ifdef DEBUG
#define logdac(_c, _s, _a) \
do { \
  em50_dbl dac = { .q = G_DAC(_c, _a) }; \
  double dbl = to_dbl(dac.q); \
  logmsg("-> flpt " _s " sign %d mantissa %11.11lx exponent %4.4x (%e)\n", dac.sign, (long unsigned int)dac.mantissa, dac.exponent, dbl); \
} while (0)
#else
#define logdac(_c, _s, _a)
#endif


#ifdef FLOAT128
typedef union {
  __int128_t  i;
  __uint128_t u;
  struct {
    int64_t  il;
    int64_t  ih;
  };
  struct {
    uint64_t ul;
    uint64_t uh;
  };
} xint128;
assert_size(xint128, 16);

static inline __float128 to_qad(__uint128_t qad)
{
em50_qad v = {.q = qad};
ieee_qad r = {.sign = v.dbl.sign, .mantissa = v.dbl.mantissa, .exponent = v.dbl.exponent};

  if(!v.dbl.mantissa && !v.qex.mantissa && !v.dbl.exponent) return r.qad;

  if(!(r.sign = v.dbl.sign))
  {
    r.mantissa = ((__uint128_t)v.dbl.mantissa << (17+48+1)) | ((__uint128_t)v.qex.mantissa << (17+1));
    r.exponent = 16383 + v.dbl.exponent - 129;
  }
  else
  {
    if(v.dbl.mantissa == 0 && v.qex.mantissa == 0)
    {
      r.exponent  = 16383 + v.dbl.exponent - 128;
      r.mantissa = 0;
    }
    else
    {
      r.exponent  = 16383 + v.dbl.exponent - 129;
      xint128 m = {.uh = v.dbl.mantissa | 0xffffc00000000000ULL, .ul = 0};
      m.i >>= 16;
      m.ul |= v.qex.mantissa;
      m.i = -m.i;
      r.mantissa = m.u << 17;
    }
  }

  return r.qad;
}

static inline __uint128_t from_qad(__float128 qad)
{
ieee_qad v = {.qad = qad};
em50_qad r;

  if(!v.mantissa && !v.exponent) return v.qad;

  if(!(r.dbl.sign = v.sign))
  {
    r.dbl.exponent  = 129 + v.exponent - 16383;
    r.dbl.mantissa = (v.mantissa >> (17+48+1)) | (1ULL << 46);
    r.qex.mantissa = v.mantissa >> (17+1);
  }
  else
  {
    if(v.mantissa == 0)
    {
      r.dbl.exponent  = 128 + v.exponent - 16383;
      r.dbl.mantissa = 0;
      r.qex.mantissa = 0;
    }
    else
    {
      r.dbl.exponent  = 129 + v.exponent - 16383;
      xint128 m = {.u = v.mantissa};
      m.uh |= 0xffff000000000000ULL;
      m.i = -m.i;
      r.dbl.mantissa = m.u >> (17+48+1);
      r.qex.mantissa = m.u >> (17+1);
    }
  }

  r.qex.reserved = 0;

  return r.q;
}
#endif

static inline FLOAT to_dbl(uint64_t dbl)
{
em50_dbl v = {.q = dbl};
ieee_dbl r = {.sign = v.sign, .mantissa = v.mantissa, .exponent = v.exponent};

  if(!r.mantissa && !r.exponent) return r.dbl;

  if(!(r.sign = v.sign))
  {
    r.exponent = 1023 + v.exponent - 129;
    r.mantissa = ((uint64_t)v.mantissa << 6); // 6 = 52 - 47 + 1
  }
  else
  {
    if(v.mantissa == 0)
    {
      r.exponent  = 1023 + v.exponent - 128;
      r.mantissa = 0;
    }
    else
    {
      r.exponent  = 1023 + v.exponent - 129;
      int64_t m = v.mantissa | 0xffffc00000000000ULL;
      m = -m;
      r.mantissa = m;
      r.mantissa = ((uint64_t)r.mantissa << 6);
    }
  }

//logall("-> flpt to_flt em50 sign %d mantissa %11.11zx exponent %4.4x (%e) true %d dac %16.16zx\n", v.sign, v.mantissa, v.exponent, r.dbl, v.exponent - 128, v.q);
//logall("-> flpt to_flt ieee sign %d mantissa %11.11zx exponent %4.4x (%e) true %d dbl %16.16zx\n", r.sign, r.mantissa, r.exponent, r.dbl, r.exponent - 1023, r.q);

  return r.dbl;
}

static inline void ins_qex(double *dbl, uint64_t qex)
{
em50_qex v = {.q = qex};
ieee_dbl *r = (ieee_dbl*)dbl;

  r->mantissa |= v.mantissa >> (47 - (52-47)); 
}

static inline double fto_dbl(uint32_t flt)
{
em50_flt f = {.d = flt};
em50_dbl d;

  f2d(&d, &f);

  return to_dbl(d.q);
}

static inline uint64_t from_dbl(double dbl)
{
ieee_dbl v = {.dbl = dbl};
em50_dbl r;

  if(!v.mantissa && !v.exponent) return v.q;

  if(!(r.sign = v.sign))
  {
    r.exponent  = 129 + v.exponent - 1023;
//  v.mantissa += 0b100000;
    r.mantissa = (v.mantissa >> 6) | (1ULL << 46);
  }
  else
  {
    if(v.mantissa == 0)
    {
      r.exponent  = 128 + v.exponent - 1023;
      r.mantissa = 0;
    }
    else
    {
      r.exponent  = 129 + v.exponent - 1023;
//    v.mantissa += 0b100000;
      r.mantissa = (v.mantissa >> 6);
      int64_t m = r.mantissa | 0xffffc00000000000ULL;
      m = -m;
      r.mantissa = m;
    }
  }

//logall("-> flpt from_flt ieee sign %d mantissa %12.12zx exponent %4.4x (%e) true %d dbl %16.16zx\n", v.sign, v.mantissa, v.exponent, v.dbl, v.exponent - 1023, v.q);
//logall("-> flpt from_flt em50 sign %d mantissa %12.12zx exponent %4.4x (%e) true %d dac %16.16zx\n", r.sign, r.mantissa, r.exponent, v.dbl, r.exponent - 128, r.q);
  return r.q;
}

static inline uint64_t from_dbl_rnd(double dbl)
{
ieee_dbl v = {.dbl = dbl};

  if(v.mantissa)
    v.mantissa += 0b100000;

  return from_dbl(v.dbl);
}

static inline uint64_t qex_dbl(double dbl)
{
ieee_dbl v = {.dbl = dbl};
em50_qex r;

  r.reserved = 0;
  r.mantissa = v.mantissa << (47 - (52-47));

  return r.q;
}

static inline uint32_t ffrom_dbl_rnd(double dbl)
{
em50_dbl d = {.q = from_dbl_rnd(dbl)};
em50_flt f;

  d2f(&f, &d);

  return f.d;
}

#endif


E50I(bfeq);
E50I(bfge);
E50I(bfgt);
E50I(bfle);
E50I(bflt);
E50I(bfne);

E50I(lfeq);
E50I(lfge);
E50I(lfgt);
E50I(lfle);
E50I(lflt);
E50I(lfne);

E50I(fsgt);
E50I(fsle);
E50I(fsmi);
E50I(fsnz);
E50I(fspl);
E50I(fsze);

E50I(fld);
E50I(dfld);
E50I(fst);
E50I(dfst);
E50I(flx);
E50I(dflx);
E50I(qflx);
E50I(flot);
E50I(flta);
E50I(fltl);
E50I(int);
E50I(inta);
E50I(intl);
E50I(fdbl);
E50I(fcm);
E50I(dfcm);
E50I(dfcs);
E50I(fcs);
E50I(fdv);
E50I(fmp);
E50I(dfmp);
E50I(dfdv);
E50I(fad);
E50I(dfad);
E50I(fsb);
E50I(dfsb);

E50I(frn);
E50I(frnm);
E50I(frnp);
E50I(frnz);

E50I(fl);
E50I(dfl);
E50I(fa);
E50I(dfa);
E50I(fs);
E50I(dfs);
E50I(fm);
E50I(dfm);
E50I(dfd);
E50I(fd);
E50I(fc);
E50I(dfc);
E50I(dble);
E50I(flt);
E50I(flth);
E50I(inth);

E50I(qfxx);
E50I(qfld);
E50I(qfst);
E50I(qfcm);
E50I(qfad);
E50I(qfsb);
E50I(qfmp);
E50I(qfdv);
E50I(qfc);
E50I(fcdq);
E50I(qinq);
E50I(qiqr);

E50I(drn);
E50I(drnz);
E50I(drnp);
