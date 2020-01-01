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


#define to31(_v)   (((int32_t)((_v) & 0xffff0000) >> 1) | ((_v) & 0x7fff))
#define from31(_v) (((int32_t)((_v) & 0x7fff8000) << 1) | ((_v) & 0x7fff))


static inline uint16_t add_w(uint16_t a, uint16_t b, int *o, int *c)
{
uint32_t l;
uint16_t r;
int i;

    l = (uint32_t)a + b;
    r = (uint16_t)l;

    i = (int)(((a & 0x7fff) + (b & 0x7fff)) >> 15);
    *c = (int)(l >> 16);

    *o = (i != *c);

    return r;
}

static inline uint16_t sub_w(uint16_t a, uint16_t b, int *o, int *c)
{
uint32_t l;
uint16_t r;
int i;

    l = (uint32_t)a + (~b & 0xffff) + 1;
    r = (uint16_t)l;

    i = (int)(((a & 0x7fff) + (~b & 0x7fff) + 1) >> 15);
    *c = (int)(l >> 16);

    *o = (i != *c);

    return r;
}

static inline uint32_t add_d(uint32_t a, uint32_t b, int *o, int *c)
{
uint64_t l;
uint32_t r;
int i;

    l = (uint64_t)a + b;
    r = (uint32_t)l;

    i = (int)(((a & 0x7fffffff) + (b & 0x7fffffff)) >> 31);
    *c = (int)(l >> 32);

    *o = (i != *c);

    return r;
}

static inline uint32_t add_d31(uint32_t a, uint32_t b, int *o, int *c)
{
uint64_t l;
uint32_t r;
int i;

    l = (uint64_t)a + b;
    r = (uint32_t)l;

    i = (int)(((a & 0x7fffffff) + (b & 0x7fffffff)) >> 30);
    *c = (int)(l >> 31);

    *o = (i != *c);

    return r;
}

static inline uint32_t sub_d(uint32_t a, uint32_t b, int *o, int *c)
{
uint64_t l;
uint32_t r;
int i;

    l = (uint64_t)a + ~b + 1;
    r = (uint32_t)l;

    i = (int)(((a & 0x7fffffff) + (~b & 0x7fffffff) + 1) >> 31);
    *c = (int)(l >> 32);

    *o = (i != *c);

    return r;
}

static inline uint32_t sub_d31(uint32_t a, uint32_t b, int *o, int *c)
{
uint64_t l;
uint32_t r;
int i;

    l = (uint64_t)a + ~b + 1;
    r = (uint32_t)l;

    i = (int)(((a & 0x7fffffff) + (~b & 0x7fffffff) + 1) >> 30);
    *c = (int)(l >> 31);

    *o = (i != *c);

    return r;
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

