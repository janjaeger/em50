/* Skip Instructions
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


#ifndef I_MODE
// SS1  1000001000010000 (S, R, V mode form)
E50I(ss1)
{
int isset = (cpu->sys->sswitches & 0100000) != 0;

  logop1(op, "ss1");

  E50X(rxm_check)(cpu);

  if(isset)
    ++cpu->p;
}


// SR1  1000000000010000 (S, R, V mode form)
E50I(sr1)
{
int isset = (cpu->sys->sswitches & 0100000) != 0;

  logop1(op, "ss1");

  E50X(rxm_check)(cpu);

  if(!isset)
    ++cpu->p;
}


// SS2  1000001000001000 (S, R, V mode form)
E50I(ss2)
{
int isset = (cpu->sys->sswitches & 040000) != 0;

  logop1(op, "ss2");

  E50X(rxm_check)(cpu);

  if(isset)
    ++cpu->p;
}


// SR2  1000000000001000 (S, R, V mode form)
E50I(sr2)
{
int isset = (cpu->sys->sswitches & 040000) != 0;

  logop1(op, "sr2");

  E50X(rxm_check)(cpu);

  if(!isset)
    ++cpu->p;
}


// SR3  1000000000000100 (S, R, V mode form)
E50I(sr3)
{
int isset = (cpu->sys->sswitches & 020000) != 0;

  logop1(op, "sr3");

  E50X(rxm_check)(cpu);

  if(!isset)
    ++cpu->p;
}


// SS3  1000001000000100 (S, R, V mode form)
E50I(ss3)
{
int isset = (cpu->sys->sswitches & 020000) != 0;

  logop1(op, "ss3");

  E50X(rxm_check)(cpu);

  if(isset)
    ++cpu->p;
}


// SS4  1000001000000010 (S, R, V mode form)
E50I(ss4)
{
int isset = (cpu->sys->sswitches & 010000) != 0;

  logop1(op, "ss4");

  E50X(rxm_check)(cpu);

  if(isset)
    ++cpu->p;
}


// SR4  1000000000000010 (S, R, V mode form)
E50I(sr4)
{
int isset = (cpu->sys->sswitches & 010000) != 0;

  logop1(op, "sr4");

  E50X(rxm_check)(cpu);

  if(!isset)
    ++cpu->p;
}


// SSS  1000001000011110 (S, R, V mode form)
E50I(sss)
{
int isset = (cpu->sys->sswitches & 0170000) != 0;

  logop1(op, "sss");

  E50X(rxm_check)(cpu);

  if(isset)
    ++cpu->p;
}


// SSR  1000000000011110 (S, R, V mode form)
E50I(ssr)
{
int isset = (cpu->sys->sswitches & 0170000) != 0;

  logop1(op, "ssr");

  E50X(rxm_check)(cpu);

  if(!isset)
    ++cpu->p;
}
#endif

// NOP  1000001000000000 (S, R, V mode form)
E50I(nop)
{
  logop1(op, "nop");
}


// SGT  1000000010010000 (S, R, V mode form)
E50I(sgt)
{
int16_t a = (int16_t)G_A(cpu);

  logop1(op, "sgt");

  if(a > 0)
    ++cpu->p;
}


// SKP  1000000000000000 (S, R, V mode form)
E50I(skp)
{
  logop1(op, "skp");

  ++cpu->p;
}


// SLE  1000001010010000 (S, R, V mode form)
E50I(sle)
{
int16_t a = (int16_t)G_A(cpu);

  logop1(op, "sle");

  if(a <= 0)
    ++cpu->p;
}


// SLN  1000001001000000 (S, R, V mode form)
E50I(sln)
{
int16_t a = G_A(cpu);

  logop1(op, "sln");

  if((a & 1))
    ++cpu->p;
}


// SLZ  1000000001000000 (S, R, V mode form)
E50I(slz)
{
int16_t a = (int16_t)G_A(cpu);

  logop1(op, "slz");

  if(!(a & 1))
    ++cpu->p;
}


// SMCR 1000000010000000 (S, R, V mode form)
E50I(smcr)
{
int16_t mc = 0; // TODO

  logop1(op, "smcr");

  if(!mc)   
    ++cpu->p;
}


// SMCS 1000001010000000 (S, R, V mode form)
E50I(smcs)
{
int16_t mc = 0; // TODO

  logop1(op, "smcs");

  if(mc)   
    ++cpu->p;
}

// SMI  1000001100000000 (S, R, V mode form) -- also slt
E50I(smi)
{
int16_t a = (int16_t)G_A(cpu);

  logop1(op, "smi");

  if(a < 0)
    ++cpu->p;
}


// SNZ  1000001000100000 (S, R, V mode form) -- also sne
E50I(snz)
{
uint16_t a = G_A(cpu);

  logop1(op, "snz");

  if(a != 0)
    ++cpu->p;
}


// SPL  1000000100000000 (S, R, V mode form) -- also sge
E50I(spl)
{
int16_t a = (int16_t)G_A(cpu);

  logop1(op, "spl");

  if(a >= 0)
    ++cpu->p;
}


// SRC  1000000000000001 (S, R, V mode form)
E50I(src)
{
int cbit = cpu->crs->km.cbit;

  logop1(op, "src");

  if(!cbit)
    ++cpu->p;
}


// SSC  1000001000000001 (S, R, V mode form)
E50I(ssc)
{
int cbit = cpu->crs->km.cbit;

  logop1(op, "ssc");

  if(cbit)
    ++cpu->p;
}


// SZE  1000000000100000 (S, R, V mode form) -- also seq
E50I(sze)
{
uint16_t a = G_A(cpu);

  logop1(op, "sze");

  if(a == 0)
    ++cpu->p;
}


// SAR  100000001011 N\4 (S, R, V mode form)
E50I(sar)
{
int bit = op[1] & 0b1111;
uint16_t a = G_A(cpu);

  logop1o(op, "sar", bit);

  if(!(a & (0100000 >> bit)))
    ++cpu->p;
}


// SAS  100000101011 N\4 (S, R, V mode form)
E50I(sas)
{
int bit = op[1] & 0b1111;
uint16_t a = G_A(cpu);

  logop1o(op, "sas", bit);

  if((a & (0100000 >> bit)))
    ++cpu->p;
}


// SNR  100000001010 N\4 (S, R, V mode form)
E50I(snr)
{
int sswitch = op[1] & 0b1111;
int isset = (cpu->sys->sswitches & (0100000 >> sswitch)) != 0;

  logop1o(op, "snr", sswitch);

  E50X(rxm_check)(cpu);

  if(!isset)
    ++cpu->p;
}


// SNS  100000101010 N\4 (S, R, V mode form)
E50I(sns)
{
int sswitch = op[1] & 0b1111;
int isset = (cpu->sys->sswitches & (0100000 >> sswitch)) != 0;

  logop1o(op, "sns", sswitch);

  E50X(rxm_check)(cpu);

  if(isset)
    ++cpu->p;
}


E50I(caz)
{
int16_t a = G_A(cpu);

  logop1(op, "caz");

  int car, ovf;
  sub_w(a, 0, &car, &ovf);
  cpu->crs->km.link = car;
  SET_CC(cpu, a);

  if(CC_LT(cpu))
    cpu->p += 2;
  else
    if(CC_EQ(cpu))
      ++cpu->p;
}


E50I(cas)
{
int16_t a = G_A(cpu);
uint32_t ea = E50X(ea)(cpu, op);
int16_t s = E50X(vfetch_w)(cpu, ea);

  logopxoo(op, "cas", ea, s & 0xffff);

  int car, ovf;
  sub_w(a, s, &car, &ovf);
  cpu->crs->km.link = car;
  _SET_CC(cpu, a, s);

  if(CC_LT(cpu))
    cpu->p += 2;
  else
    if(CC_EQ(cpu))
      ++cpu->p;
}


E50I(cls)
{
int32_t l = G_L(cpu);
uint32_t ea = E50X(ea)(cpu, op);
int32_t s = E50X(vfetch_d)(cpu, ea);

  logop2oo(op, "cls", ea, s);

  int car, ovf;
  sub_d(l, s, &car, &ovf);
  cpu->crs->km.link = car;
  _SET_CC(cpu, l, s);

  if(CC_LT(cpu))
    cpu->p += 2;
  else
    if(CC_EQ(cpu))
      ++cpu->p;
}


E50I(irx)
{
uint16_t x = G_X(cpu);

  logop1(op, "irx");

  S_X(cpu, ++x);

  if(x == 0)
    ++cpu->p;
}


E50I(drx)
{
uint16_t x = G_X(cpu);

  logop1(op, "drx");

  S_X(cpu, --x);

  if(x == 0)
    ++cpu->p;
}


E50I(irs)
{
uint32_t ea = E50X(ea)(cpu, op);
uint16_t s = E50X(vfetch_w)(cpu, ea);

  logopxoo(op, "irs", ea, s);

  E50X(vstore_w)(cpu, ea , ++s);

  if(s == 0)
    ++cpu->p;
}


#ifndef EMDE
 #include __FILE__
#endif
