/* Integrity Instructions
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


E50I(rmc)
{
  logop1(op, "rmc");

  E50X(rxm_check)(cpu);

  cpu->crs->km.mcm = km_mcn;

  longjmp(cpu->endop, endop_nointr5); // FIXME nointr1
}


E50I(emcm)
{
  logop1(op, "emcm");

  E50X(rxm_check)(cpu);

  cpu->crs->km.mcm = km_mca;
}


E50I(lmcm)
{
  logop1(op, "lmcm");

  E50X(rxm_check)(cpu);

  cpu->crs->km.mcm = km_mcn;
}


E50I(mdwc)
{
  logop1(op, "*mdwc");

  E50X(rxm_check)(cpu);
}


#ifndef EMDE
 #include __FILE__
#endif
