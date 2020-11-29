/* Integer Instructions
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


#ifndef _int_h
#define _int_h

static inline int32_t to31(int32_t v)
{
  return ((int32_t)(v & 0xffff0000) >> 1) | (v & 0x7fff);
}

static inline int32_t to31h(int32_t v)
{
  return (v & 0xffff0000) | ((v & 0x7fff) << 1);
}


static inline int32_t fr31(int32_t v)
{
  return ((int32_t)(v & 0x7fff8000) << 1) | (v & 0x7fff);
}

static inline int32_t fr31h(int32_t v)
{
  return (v & 0xffff0000) | ((v & 0xfffe) >> 1);
}


static inline int16_t _add_w(uint16_t a, uint16_t b, uint16_t c, int *eq, int *lt, int *car, int *ovf)
{
  uint32_t t = a + b + c;
  uint16_t r = t & 0xffff;

  if(eq)
    *eq = (r == 0) ? 1 : 0;
  if(lt)
    *lt = ((~a ^ b) & (a ^ r) & 0x8000) ? ((~r & 0x8000) >> 15) & 1 : ((r & 0x8000) >> 15) & 1;
  if(car)
    *car = (t >> 16) & 1;
  if(ovf)
    *ovf = (((~a ^ b) & (a ^ r) & 0x8000) >> 15) & 1;

  return r;
}

static inline uint16_t add_w(uint16_t a, uint16_t b, int *eq, int *lt, int *car, int *ovf)
{
  return _add_w(a, b, 0, eq, lt, car, ovf);
}

static inline uint16_t sub_w(uint16_t a, uint16_t b, int *eq, int *lt, int *car, int *ovf)
{
  return _add_w(a, -(int16_t)b, 0, eq, lt, car, ovf);
}

static inline uint16_t sba_w(uint16_t a, uint16_t b, int *eq, int *lt, int *car, int *ovf)
{
  return _add_w(a, ~b, 1, eq, lt, car, ovf);
}

static inline int32_t _add_d(uint32_t a, uint32_t b, uint32_t c, int *eq, int *lt, int *car, int *ovf)
{
  uint64_t t = (uint64_t)a + (uint64_t)b + (uint64_t)c;
  uint32_t r = t;

  if(eq)
    *eq = (r == 0) ? 1 : 0;
  if(lt)
    *lt = ((~a ^ b) & (a ^ r) & 0x80000000) ? ((~r & 0x80000000) >> 31) & 1 : ((r & 0x80000000) >> 31) & 1;
  if(car)
    *car = (uint64_t)(t >> 32) & 1;
  if(ovf)
    *ovf = (((~a ^ b) & (a ^ r) & 0x80000000) >> 31) & 1;

  return r;
}

static inline uint32_t add_d(uint32_t a, uint32_t b, int *eq, int *lt, int *car, int *ovf)
{
  return _add_d(a, b, 0, eq, lt, car, ovf);
}

static inline uint32_t sub_d(uint32_t a, uint32_t b, int *eq, int *lt, int *car, int *ovf)
{
  return _add_d(a, -(int32_t)b, 0, eq, lt, car, ovf);
}

static inline uint32_t sba_d(uint32_t a, uint32_t b, int *eq, int *lt, int *car, int *ovf)
{
  return _add_d(a, ~b, 1, eq, lt, car, ovf);
}

static inline uint32_t add_d31(uint32_t a, uint32_t b, int *eq, int *lt, int *car, int *ovf)
{
  return fr31h(add_d(to31h(a), to31h(b), eq, lt, car, ovf));
}

static inline uint32_t sba_d31(uint32_t a, uint32_t b, int *eq, int *lt, int *car, int *ovf)
{
  return fr31h(sba_d(to31h(a), to31h(b), eq, lt, car, ovf));
}

#endif

E50I(dbl);
E50I(sgl);
E50I(sca);
E50I(chs);
E50I(a1a);
E50I(a2a);
E50I(aca);
E50I(add);
E50I(adl);
E50I(adll);
E50I(s1a);
E50I(s2a);
E50I(tca);
E50I(tcl);
E50I(sub);
E50I(ssm);
E50I(ssp);
E50I(sbl);
E50I(div);
E50I(dvl);
E50I(mpl);
E50I(mpy);
E50I(pid);
E50I(pida);
E50I(pidl);
E50I(pim);
E50I(pima);
E50I(piml);
E50I(nrm);


E50I(a);
E50I(adlr);
E50I(ah);
E50I(c);
E50I(ch);
E50I(d);
E50I(dh);
E50I(dh1);
E50I(dh2);
E50I(dr1);
E50I(dr2);
E50I(dm);
E50I(dmh);
E50I(ih1);
E50I(ih2);
E50I(ir1);
E50I(ir2);
E50I(im);
E50I(imh);
E50I(m);
E50I(mh);
E50I(pid);
E50I(pidh);
E50I(pim);
E50I(pimh);
E50I(s);
E50I(sh);
E50I(tc);
E50I(tch);
E50I(tm);
E50I(tmh);

