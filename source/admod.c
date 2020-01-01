/* Addressing Mode Instructions
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


E50I(e16s)
{
  logop1(op, "e16s");
  set_cpu_mode(cpu, km_e16s);
}


E50I(e32i)
{
  logop1(op, "e32i");
  set_cpu_mode(cpu, km_e32i);
}


E50I(e32s)
{
  logop1(op, "e32s");
  set_cpu_mode(cpu, km_e32s);
}


E50I(e32r)
{
  logop1(op, "e32r");
  set_cpu_mode(cpu, km_e32r);
}


E50I(e64r)
{
  logop1(op, "e64r");
  set_cpu_mode(cpu, km_e64r);
}


E50I(e64v)
{
  logop1(op, "e64v");
  set_cpu_mode(cpu, km_e64v);
}


#ifndef EMDE
 #include __FILE__
#endif
