/* Field Register Instructions
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

#include "field.h"


E50I(eafa)
{
uint16_t bit;
uint32_t ap = E50X(vfetch_iap)(cpu, &bit);
int f = FAR(op);

  logop2oo(op, "eafa", f, ap);

  S_FAR(cpu, f, ap & ~ea_e);
  S_FBR(cpu, f, bit);

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}


E50I(stfa)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);
int f = FAR(op);
uint32_t far = G_FAR(cpu, f);
int bit = G_FBR(cpu, f);

  logop2oo(op, "stfa", f, ap);

  if(bit)
  {
    E50X(vstore_d)(cpu, ap, far | ea_e);
    E50X(vstore_w)(cpu, ap + 2, bit << 12);
  }
  else
    E50X(vstore_d)(cpu, ap, far);

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}


// ALFA 000000101100F001 (V mode format)
/* ALFA
 * Add L to FAR
 * 000000101100F001 (V mode format)
 */
#ifdef V_MODE
E50I(alfa)
{
int32_t l = G_L(cpu);
int f = FAR(op);
uint32_t far = G_FAR(cpu, f);
int bit = G_FBR(cpu, f);

  uint64_t bits = far << 4 | bit;

  bits += l;
  bit = bits & 0xf;
  l = bits >> 4;

  logop2o(op, "alfa", f);

  logmsg("-> far%d in: %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
  S_FAR(cpu, f, intraseg_o(far, l & 0xffff));
  S_FBR(cpu, f, bit);
  logmsg("-> far%d out: %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}
#endif


#ifdef I_MODE
E50I(arfa)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
int f = FAR(op);
uint32_t far = G_FAR(cpu, f);
int bit = G_FBR(cpu, f);

  uint64_t bits = far << 4 | bit;

  bits += r;
  bit = bits & 0xf;
  r = bits >> 4;

  logop2o3(op, "arfa", f, dr, r);

  logmsg("-> far%d in: %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
  S_FAR(cpu, f, intraseg_o(far, r & 0xffff));
  S_FBR(cpu, f, bit);
  logmsg("-> far%d out: %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));

}
#endif


E50I(lfli)
{
#if 0
cpu->arg = vfetch_iw(cpu);
uint16_t im = fetch_w(op + 2);
#else
uint16_t im = E50X(vfetch_w)(cpu, cpu->pb); cpu->p++;
#endif
int f = FAR(op);

  logop2oo(op, "lfli", f, im);

  S_FLR(cpu, f, im);

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}


E50I(tlfl)
{
int f = FAR(op);
uint32_t l = G_L(cpu);

  logop1oo(op, "tlfl", f, l);

  S_FLR(cpu, f, l);

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}


#ifdef I_MODE
E50I(trfl)
{
int dr = op_dr(op);
int f = FAR(op);
uint32_t r = G_R(cpu, dr);

  logop1o3(op, "trfl", f, dr, r);

  S_FLR(cpu, f, r);

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}
#endif


E50I(tfll)
{
int f = FAR(op);

  logop2o(op, "tfll", f);

  S_L(cpu, G_FLR(cpu, f));

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}


#ifdef I_MODE
E50I(tflr)
{
int dr = op_dr(op);
int f = FAR(op);

  logop1oo(op, "flrl", f, dr);

  S_R(cpu, dr, G_FLR(cpu, f));

  logmsg("-> far%d %8.8x flr %8.8x fbr %d\n", f, G_FAR(cpu, f), G_FLR(cpu, f), G_FBR(cpu, f));
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
