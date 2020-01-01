/* Queue Instructions
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


#ifndef _queue_h
#define _queue_h

#include "io.h"

// QUICK FIX, QUEUES MUST BE UPDATED WITH INTERLOCKED ATOMICS
static inline int qvalidate(uint16_t *t1, uint16_t t2, uint16_t m)
{
  uint16_t m1 = *t1 & ~m;
  uint16_t m2 = t2 & ~m;

  if(m1 != m2)
  {
    *t1 = m2 | (*t1 & m);
    return 1;
  }
  else
    return 0;
}

static inline void qstore_w(cpu_t *cpu, uint32_t addr, uint16_t val)
{
  store_w(physad(cpu, addr), val);
}

static inline uint16_t qfetch_w(cpu_t *cpu, uint32_t addr)
{
  return fetch_w(physad(cpu, addr));
}

static inline int io_atq(cpu_t *cpu, uint32_t queue, uint16_t value)
{
uint16_t t1 = ifetch_w(cpu, queue + 0); // TOP
uint16_t t2 = ifetch_w(cpu, queue + 1); // BOTTOM
uint16_t t3 = ifetch_w(cpu, queue + 2); // SEGMENT
uint16_t t4 = ifetch_w(cpu, queue + 3); // MASK

#ifdef DEBUG
  int q = qvalidate(&t1, t2, t4);
  logmsg("io atq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x err %d\n", t1, t2, t3, t4, q);
#endif

  t1 = (t1 & ~t4) | ((t1 - 1) & t4);
  logmsg("io atq -> %4.4x\n", t1);

  if(t1 == t2)
    return -1;

  qstore_w(cpu, ((t3 << 16) | t1), value);
  istore_w(cpu, queue, t1);

  return 0;
}

static inline int io_abq(cpu_t *cpu, uint32_t queue, uint16_t value)
{
uint16_t t1 = ifetch_w(cpu, queue + 0); // TOP
uint16_t t2 = ifetch_w(cpu, queue + 1); // BOTTOM
uint16_t t3 = ifetch_w(cpu, queue + 2); // SEGMENT
uint16_t t4 = ifetch_w(cpu, queue + 3); // MASK

#ifdef DEBUG
  int q = qvalidate(&t2, t1, t4);
  logmsg("io abq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x err %d\n", t1, t2, t3, t4, q);
#endif

  uint16_t t5 = (t2 & ~t4) | ((t2 + 1) & t4);
  logmsg("io abq -> %4.4x\n", t5);

  if(t1 == t5)
    return -1;

  qstore_w(cpu, ((t3 << 16) | t2), value);
  istore_w(cpu, queue + 1, t2);

  return 0;
}

static inline int io_rbq(cpu_t *cpu, uint32_t queue, uint16_t *value)
{
uint16_t t1 = ifetch_w(cpu, queue + 0); // TOP
uint16_t t2 = ifetch_w(cpu, queue + 1); // BOTTOM
uint16_t t3 = ifetch_w(cpu, queue + 2); // SEGMENT
uint16_t t4 = ifetch_w(cpu, queue + 3); // MASK

#ifdef DEBUG
  int q = qvalidate(&t2, t1, t4);
  logmsg("io rbq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x err %d\n", t1, t2, t3, t4, q);
#endif

  if(t1 == t2)
    return -1;

  t2 = (t2 & ~t4) | ((t2 - 1) & t4);

  *value = qfetch_w(cpu, (t3 << 16) | t2);

  istore_w(cpu, queue + 1, t2);

  return 0;
}

static inline int io_rtq(cpu_t *cpu, uint32_t queue, uint16_t *value)
{
uint16_t t1 = ifetch_w(cpu, queue + 0); // TOP
uint16_t t2 = ifetch_w(cpu, queue + 1); // BOTTOM
uint16_t t3 = ifetch_w(cpu, queue + 2); // SEGMENT
uint16_t t4 = ifetch_w(cpu, queue + 3); // MASK

#ifdef DEBUG
  int q = qvalidate(&t1, t2, t4);
  logmsg("io rtq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x err %d\n", t1, t2, t3, t4, q);
#endif

  if(t1 == t2)
    return -1;

  *value = qfetch_w(cpu, (t3 << 16) | t1);

  t1 = (t1 & ~t4) | ((t1 + 1) & t4);

  istore_w(cpu, queue, t1);

  return 0;
}

static inline int io_tstq(cpu_t *cpu, uint32_t queue, uint16_t *value)
{
uint16_t t1 = ifetch_w(cpu, queue + 0); // TOP
uint16_t t2 = ifetch_w(cpu, queue + 1); // BOTTOM
uint16_t t4 = ifetch_w(cpu, queue + 3); // MASK

#ifdef DEBUG
uint16_t t3 = ifetch_w(cpu, queue + 2); // SEGMENT
  logmsg("io tstq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x\n", t1, t2, t3, t4);
#endif

  *value = (t2 - t1) & t4;

  return t1 == t2 ? -1 : 0;
}
#endif

#if defined E50I

E50I(abq);
E50I(atq);
E50I(rbq);
E50I(rtq);
E50I(tstq);

#endif
