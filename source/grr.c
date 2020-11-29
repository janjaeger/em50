/* General Register Relative Instructions
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

#include "int.h"


#ifdef I_MODE

E50I(lip)
{
  int dr = op_dr(op);
  uint32_t ea = E50X(ea)(cpu, op);

  logop2oo(op, "*lip", dr, ea);

#if 0
  uint32_t r = E50X(vfetch_il)(cpu, ea);
#else
  uint32_t r = E50X(vfetch_d)(cpu, ea);

  if((r & 0x80000000))
    E50X(pointer_fault)(cpu, r >> 16, ea);
#endif

  S_R(cpu, dr, r);
}


E50I(aip)
{
  int dr = op_dr(op);
  uint32_t ea = E50X(ea)(cpu, op);

  logop2oo(op, "*aip", dr, ea);

  uint32_t r = G_R(cpu, dr) & (ea_s|ea_w);
  uint32_t s = E50X(vfetch_il)(cpu, ea);

  int eq, lt, car, ovf;

  r = add_d(s, r, &eq, &lt, &car, &ovf);

  cpu->crs->km.eq = eq;
  cpu->crs->km.lt = lt;
  cpu->crs->km.link = car;
  cpu->crs->km.cbit = ovf;

  S_R(cpu, dr, r);

  if(ovf)
    E50X(int_ovf)(cpu);

}


E50I(lcc_ccp)
{
int dr = op_dr(op);
int tm = op_tm(op);
int sr = op_sr(op);
int br = op_br(op);

  if(tm == 0 && br == 0)
  {
  logop2o(op, "*ccp", dr);
  
    uint32_t d = G_R(cpu, dr);
    d = ((d & 0x0fffffff) << 1) | ((d & ea_e) >> 28);

    uint32_t s = G_R(cpu, sr);
    s = ((s & 0x0fffffff) << 1) | ((s & ea_e) >> 28);

    _SET_CC(cpu, d, s);
  }
  else
  {
    logop2o(op, "*lcc", dr);

    uint32_t r = E50X(ea)(cpu, op);
    uint16_t c = E50X(vfetch_w)(cpu, r);

    if((r & ea_e))
      c &= 0x00ff;
    else
      c >>= 8;

    SET_CC(cpu, c);

    S_RH(cpu, dr, c);
  }
}

E50I(scc_acp)
{
int dr = op_dr(op);
int tm = op_tm(op);
//  sr = op_sr(op);
int br = op_br(op);

  if(tm == 0 && br < 2)
  {
    int32_t s = E50X(efetch_d)(cpu, op);

    logop2oo(op, "*acp", dr, s);
  
    uint32_t d = G_R(cpu, dr);

    uint32_t p = ((d & 0x0fffffff) << 1) | ((d & ea_e) >> 28);
    p += s;
    d = (d & 0xe0000000) | (p >> 1) | ((p & 1) << 28);

    S_R(cpu, dr, d);
  }
  else
  {
    logop2o(op, "*scc", dr);

    uint32_t r = E50X(ea)(cpu, op);

    uint16_t c = G_RH(cpu, dr) & 0xff;

    uint16_t s = E50X(vfetch_w)(cpu, r);
    if((r & ea_e))
      s = (s & 0xff00) | c;
    else
      s = (s & 0x00ff) | (c << 8);

    E50X(vstore_w)(cpu, r, s & 0x1fffffff);
  }
}

E50I(dcp)
{
int dr = op_dr(op);

  logop2o(op, "*dcp", dr);

  uint32_t r = G_R(cpu, dr);

  uint32_t p = ((r & 0x0fffffff) << 1) | ((r & ea_e) >> 28);

  if(p)
    --p;
  else
    p = 0x1ffffffe;

  r = (r & 0xe0000000) | ((p >> 1) & 0x0fffffff) | ((p & 1) << 28);

  S_R(cpu, dr, r);
}

E50I(icp)
{
int dr = op_dr(op);

  logop2o(op, "*icp", dr);

  uint32_t r = G_R(cpu, dr);

  uint32_t p = ((r & 0x0fffffff) << 1) | ((r & ea_e) >> 28);

  if(p < 0x1fffffff)
    ++p;
  else
    p = 1;

  r = (r & 0xe0000000) | ((p >> 1) & 0x0fffffff) | ((p & 1) << 28);

  S_R(cpu, dr, r);
}

E50I(tcnp)
{
  logop2(op, "*tcnp");

  uint32_t r = E50X(efetch_d)(cpu, op);

  cpu->crs->km.eq = (r & 0x1fffffff) == 0 ? 1 : 0;
  cpu->crs->km.lt = 0;
}

E50I(tcnpr)
{
int dr = op_dr(op);

  logop1o(op, "*tcnp", dr);

  uint32_t r = G_R(cpu, dr);

  cpu->crs->km.eq = (r & 0x1fffffff) == 0 ? 1 : 0;
  cpu->crs->km.lt = 0;
}

#endif


#ifndef EMDE
 #include __FILE__
#endif
