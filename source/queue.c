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


#include "emu.h"

#include "mode.h"

#include "opcode.h"

#include "queue.h"


#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


#if defined V_MODE || defined I_MODE
static inline void E50X(qstore_w)(cpu_t *cpu, uint32_t qadr, uint16_t value)
{
uint32_t addr = qadr & 0x0fffffff;

  if((qadr & 0x80000000))
    E50X(vstore_w)(cpu, addr, value);
  else
  {
    E50X(rxm_check)(cpu);
    rstore_w(cpu, addr, value);
  }
}

static inline uint16_t E50X(qfetch_w)(cpu_t *cpu, uint32_t qadr)
{
uint32_t addr = qadr & 0x0fffffff;

  if((qadr & 0x80000000))
    return E50X(vfetch_w)(cpu, addr);
  else
  {
    E50X(rxm_check)(cpu);
    return rfetch_w(cpu, addr);
  }
}
#endif


/* ABQ
 * Add Entry to Bottom of Queue
 * 1100001111001110 (V mode form)
 * AP\32
 */
#if defined V_MODE || defined I_MODE
E50I(abq)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*abq", ap);

uint16_t t1 = E50X(vfetch_w)(cpu, ap + 0); // TOP
uint16_t t2 = E50X(vfetch_w)(cpu, ap + 1); // BOTTOM
uint16_t t3 = E50X(vfetch_w)(cpu, ap + 2); // V SEGMENT
uint16_t t4 = E50X(vfetch_w)(cpu, ap + 3); // MASK

  logmsg("-> abq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x\n", t1, t2, t3, t4);

  uint16_t t5 = (t2 & ~t4) | ((t2 + 1) & t4);
  logmsg("-> abq -> %4.4x\n", t5);

  if(t1 == t5)
    cpu->crs->km.eq = 1;
  else
  {
#if defined I_MODE
    E50X(qstore_w)(cpu, ((t3 << 16) | t2), G_RH(cpu, op_dr(op)));
#else
    E50X(qstore_w)(cpu, ((t3 << 16) | t2), G_A(cpu));
#endif
    E50X(vstore_w)(cpu, ap + 1, t5);
    cpu->crs->km.eq = 0;
  }
  cpu->crs->km.lt = 0;
}
#endif


/* ATQ
 * Add Entry to Top of Queue
 * 1100001111001111 (V mode form)
 * AP\32
 */
#if defined V_MODE || defined I_MODE
E50I(atq)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*atq", ap);

uint16_t t1 = E50X(vfetch_w)(cpu, ap + 0); // TOP
uint16_t t2 = E50X(vfetch_w)(cpu, ap + 1); // BOTTOM
uint16_t t3 = E50X(vfetch_w)(cpu, ap + 2); // V SEGMENT
uint16_t t4 = E50X(vfetch_w)(cpu, ap + 3); // MASK

  logmsg("-> atq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x\n", t1, t2, t3, t4);

  t1 = (t1 & ~t4) | ((t1 - 1) & t4);
  logmsg("-> atq -> %4.4x\n", t1);

  if(t1 == t2)
    cpu->crs->km.eq = 1;
  else
  {
#if defined I_MODE
    E50X(qstore_w)(cpu, ((t3 << 16) | t1), G_RH(cpu, op_dr(op)));
#else
    E50X(qstore_w)(cpu, ((t3 << 16) | t1), G_A(cpu));
#endif
    E50X(vstore_w)(cpu, ap + 0, t1);
    cpu->crs->km.eq = 0;
  }
  cpu->crs->km.lt = 0;
}
#endif


#if defined V_MODE || defined I_MODE
E50I(rbq)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*rbq", ap);

uint16_t t1 = E50X(vfetch_w)(cpu, ap + 0); // TOP
uint16_t t2 = E50X(vfetch_w)(cpu, ap + 1); // BOTTOM
uint16_t t3 = E50X(vfetch_w)(cpu, ap + 2); // V SEGMENT
uint16_t t4 = E50X(vfetch_w)(cpu, ap + 3); // MASK

  logmsg("-> rbq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x\n", t1, t2, t3, t4);

  if(t1 == t2)
  {
#if defined I_MODE
    S_RH(cpu, op_dr(op), 0);
#else
    S_A(cpu, 0);
#endif
    cpu->crs->km.eq = 1;
  }
  else
  {
    t2 = (t2 & ~t4) | ((t2 - 1) & t4);
#if defined I_MODE
    S_RH(cpu, op_dr(op), E50X(qfetch_w)(cpu, (t3 << 16) | t2));
#else
    S_A(cpu, E50X(qfetch_w)(cpu, (t3 << 16) | t2));
#endif
    E50X(vstore_w)(cpu, ap + 1, t2);
    cpu->crs->km.eq = 0;
  }
  cpu->crs->km.lt = 0;

  logmsg("-> rbq -> %4.4x\n", t2);
}
#endif


#if defined V_MODE || defined I_MODE
E50I(rtq)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*rtq", ap);

uint16_t t1 = E50X(vfetch_w)(cpu, ap + 0); // TOP
uint16_t t2 = E50X(vfetch_w)(cpu, ap + 1); // BOTTOM
uint16_t t3 = E50X(vfetch_w)(cpu, ap + 2); // V SEGMENT
uint16_t t4 = E50X(vfetch_w)(cpu, ap + 3); // MASK

  logmsg("-> rtq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x\n", t1, t2, t3, t4);

  if(t1 == t2)
  {
#if defined I_MODE
    S_RH(cpu, op_dr(op), 0);
#else
    S_A(cpu, 0);
#endif
    cpu->crs->km.eq = 1;
  }
  else
  {
#if defined I_MODE
    S_RH(cpu, op_dr(op), E50X(qfetch_w)(cpu, (t3 << 16) | t1));
#else
    S_A(cpu, E50X(qfetch_w)(cpu, (t3 << 16) | t1));
#endif
    t1 = (t1 & ~t4) | ((t1 + 1) & t4);
    E50X(vstore_w)(cpu, ap, t1);
    cpu->crs->km.eq = 0;
  }
  cpu->crs->km.lt = 0;

  logmsg("-> rtq -> %4.4x\n", t1);
}
#endif


#if defined V_MODE || defined I_MODE
E50I(tstq)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*tstq", ap);

uint16_t t1 = E50X(vfetch_w)(cpu, ap + 0); // TOP
uint16_t t2 = E50X(vfetch_w)(cpu, ap + 1); // BOTTOM
uint16_t t4 = E50X(vfetch_w)(cpu, ap + 3); // MASK

#ifdef DEBUG
uint16_t t3 = E50X(vfetch_w)(cpu, ap + 2); // V SEGMENT
  logmsg("-> tstq t1 %4.4x t2 %4.4x t3 %4.4x t4 %4.4x\n", t1, t2, t3, t4);
#endif

#if defined I_MODE
  S_RH(cpu, op_dr(op), (t2 - t1) & t4);
#else
  S_A(cpu, (t2 - t1) & t4);
#endif

  cpu->crs->km.eq = (t1 == t2) ? 1 : 0;
  cpu->crs->km.lt = 0;
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
