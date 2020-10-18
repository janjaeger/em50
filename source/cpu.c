/* CPU Execution
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

#include "io.h"

#include "cpu.h"

#include "prcex.h"


#ifdef HMDE

void missing_mem(cpu_t *cpu, uint32_t addr)
{
  E50X(missing_mem)(cpu, addr);
}

void mach_chk(cpu_t *cpu, uint32_t addr)
{
  E50X(mach_chk)(cpu, addr);
}

#endif


static inline endop_t E50X(run_cpu_status)(cpu_t *cpu, endop_t code)
{
  switch(cpu->halt.status) {

    case started:
      return endop_intrchk;

    case stepping:
      cpu->halt.status = stopped;

    case stopped:
      cpu_halt(cpu);
      if(cpu->crs->km.mode != km_cmde)
        set_cpu_mode(cpu, cpu->crs->km.mode);
      return endop_nointr1;
  }

  abort();
}


static inline void E50X(run_cpu_intrchk)(cpu_t *cpu)
{
__sync_synchronize();
  int32_t v;
  if((v = io_intvec(cpu)) < 0)
    return;

  logmsg("-> i/o interrupt vector %4.4x\n", v);

  if(cpu->crs->km.pxm)
    pxm_intrchk(cpu, v);
  else
  {
    cpu->crs->km.ie = 0;
//  if(!cpu->crs->km.vim)
//    io_clrai(cpu);
    uint16_t irc = E50X(vfetch_w)(cpu, cpu->crs->km.vim ? v : 063);
    if(!irc)
      longjmp(cpu->smode, smode_halt);
    E50X(vstore_wx)(cpu, irc, cpu->p, acc_wr);
    S_RB(cpu, intraseg_i(irc, 1));
  }

  longjmp(cpu->endop, endop_nointr5); // FIXME TODO CHECK
}


static inline void __attribute__ ((noreturn)) E50X(run_cpu_fault)(cpu_t *cpu)
{
ATOFF(cpu);

    cpu->crs->fcodeh     = cpu->fault.fcode;
    cpu->crs->faddr      = cpu->fault.faddr;
    cpu->srf.mrf.dswrma  = cpu->fault.faddr; // DSW ONLY SET FOR MISSING MEMORY AND MACHINE CHECK?
    cpu->srf.mrf.pswpb   = cpu->fault.pc;
    cpu->srf.mrf.pswkeys = cpu->fault.km;

  if(cpu->crs->km.pxm)
    pxm_fault(cpu);
  else
  {
    uint16_t ea = E50X(vfetch_w)(cpu, cpu->fault.offset);
    if(!ea)
      longjmp(cpu->smode, smode_halt);
    S_RB(cpu, ea + 1);
    E50X(vstore_wx)(cpu, ea, cpu->fault.pc & 0xffff, acc_wr);

logall("-> %s fault offset %2.2x vector %2.2x/%2.2x pc %8.8x keys %4.4x fcodeh %4.4x faddr %8.8x pb %8.8x\n",
  fault_name(cpu->fault.vecoff), cpu->fault.offset, cpu->fault.vector,cpu->fault.vecoff, cpu->fault.pc, cpu->fault.km.keys, cpu->fault.fcode, cpu->fault.faddr, cpu->pb);

    longjmp(cpu->endop, endop_nointr5); // FIXME TODO CHECK
  }
}


static inline void E50X(exec_inst)(cpu_t *cpu)
{
      cpu->exec = 0;
      cpu->inst = E50X(vfetch_i)(cpu);
      E50X(decode)(cpu, cpu->op);
      logopr(cpu);
}


void E50X(run_cpu)(cpu_t *cpu)
{
  do {
    endop_t code = setjmp(cpu->endop);
    cpu->exec = 0;
    do {
      if(!cpu_started(cpu))
        code = E50X(run_cpu_status)(cpu, code);
      switch(code)
      {
      case endop_fault:
        E50X(run_cpu_fault)(cpu);
      default:
      case endop_nointr5:
        E50X(exec_inst)(cpu);
        E50X(exec_inst)(cpu);
        E50X(exec_inst)(cpu);
        E50X(exec_inst)(cpu);
      case endop_nointr1:
        E50X(exec_inst)(cpu);
      case endop_intrchk:
//      E50X(timer_get)(cpu); // ABORT CHECK
        if(io_pending(cpu) && cpu->crs->km.ie)
          E50X(run_cpu_intrchk)(cpu);
      case endop_setjmp:
        for(int n = 0; n < 32; ++n)
          E50X(exec_inst)(cpu);
      }
      code = endop_run;
    } while(1);
  } while(1);
}


#ifndef EMDE
 #include __FILE__
#endif
