/* Faults
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

#ifndef _faults_h
#define _faults_h

static inline char *fault_name(int o)
{
static char *name[] = {
  "rxm",
  "process",
  "page",
  "svc",
  "uii",
  "semaphore",
  "machchk",
  "missing",
  "ill",
  "access",
  "arith",
  "stack",
  "segment",
  "pointer" };
  return name[o >> 2];
}

#endif


static inline void __attribute__ ((noreturn)) E50X(rxm_fault)(cpu_t *cpu)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = cpu->po & ea_r;
  cpu->fault.faddr  = 0;
  cpu->fault.vector = (cpu->fault.ring >> 28) + offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 000;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 062;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(process_fault)(cpu_t *cpu, uint16_t abrt)
{
  cpu->fault.pc     = cpu->pb;
  cpu->fault.ring   = 0;
  cpu->fault.faddr  = 0;
  cpu->fault.vector = offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 004;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = abrt;
  cpu->fault.offset = 063;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(page_fault)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = 0;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = offsetin(pcb_t, pfault);
  cpu->fault.vecoff = 010;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 064;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(svc_fault)(cpu_t *cpu)
{
  cpu->fault.pc     = cpu->pb;
  cpu->fault.ring   = cpu->pb & ea_r;
  cpu->fault.faddr  = 0;
  cpu->fault.vector = (cpu->fault.ring >> 28) + offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 014;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 065;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(uii_fault)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = cpu->po & ea_r;
  cpu->fault.faddr  = addr,
  cpu->fault.vector = (cpu->fault.ring >> 28) + offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 020;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = cpu->p;
  cpu->fault.offset = 066;
  longjmp(cpu->endop, endop_fault);
}

#define semaphore_fault_under (0)
#define semaphore_fault_over  (1)
static inline void __attribute__ ((noreturn)) E50X(semaphore_fault)(cpu_t *cpu, uint16_t ovf, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = 0;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 024;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = ovf;
  cpu->fault.offset = 067;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(ill_fault)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = cpu->po & ea_r;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = (cpu->fault.ring >> 28) + offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 040;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = cpu->p;
  cpu->fault.offset = 072;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(mach_chk)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = 0;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 030;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 070;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(missing_mem)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = 0;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 034;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 071;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(access_fault)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = 0;
#if defined V_MODE || defined I_MODE
  cpu->fault.faddr  = addr;
#else
  cpu->fault.faddr  = (cpu->b << 16) | (addr & 0xffff);
#endif
  cpu->fault.vector = offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 044;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 073;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(arith_fault)(cpu_t *cpu, uint16_t code, uint32_t addr)
{
  cpu->fault.pc     = cpu->pb;
  cpu->fault.ring   = cpu->pb & ea_r;
#if defined V_MODE || defined I_MODE
  cpu->fault.faddr  = addr;
#else
  cpu->fault.faddr  = (cpu->b << 16) | (addr & 0xffff);
#endif
  cpu->fault.vector = (cpu->fault.ring >> 28) + offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 050;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = code;
  cpu->crs->fcodel  = 050;
  cpu->fault.offset = 074;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(stack_fault)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = cpu->po & ea_r;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = (cpu->fault.ring >> 28) + offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 054;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 075;
  longjmp(cpu->endop, endop_fault);
}

#define segment_fault_dtar (1)
#define segment_fault_sdw  (2)
static inline void __attribute__ ((noreturn)) E50X(segment_fault)(cpu_t *cpu, uint16_t type, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = 0;
#if defined V_MODE || defined I_MODE
  cpu->fault.faddr  = addr;
#else
  cpu->fault.faddr  = (cpu->b << 16) | (addr & 0xffff);
#endif
  cpu->fault.vector = offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 060;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = type;
  cpu->fault.offset = 076;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(pointer_fault)(cpu_t *cpu, uint16_t code, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = cpu->po & ea_r;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = (cpu->fault.ring >> 28) + offsetin(pcb_t, fault[0]);
  cpu->fault.vecoff = 064;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = code;
  cpu->fault.offset = 077;
  longjmp(cpu->endop, endop_fault);
}

static inline void __attribute__ ((noreturn)) E50X(power_check)(cpu_t *cpu)
{
  cpu->fault.pc     = cpu->pb;
  cpu->fault.ring   = cpu->pb & ea_r;
  cpu->fault.faddr  = 0;
  cpu->fault.vector = 0200;
//cpu->fault.vecoff = 0;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 0070;
  longjmp(cpu->endop, endop_check);
}

static inline void __attribute__ ((noreturn)) E50X(environment_check)(cpu_t *cpu, uint16_t code)
{
  cpu->fault.pc     = cpu->pb;
  cpu->fault.ring   = cpu->pb & ea_r;
  cpu->fault.faddr  = 0;
  cpu->fault.vector = 0200;
//cpu->fault.vecoff = 0;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = code;
  cpu->fault.offset = 070;
  longjmp(cpu->endop, endop_check);
}

static inline void __attribute__ ((noreturn)) E50X(parity_check)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = cpu->po & ea_r;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = 0270;
//cpu->fault.vecoff = 0;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 071;
  longjmp(cpu->endop, endop_check);
}

static inline void __attribute__ ((noreturn)) E50X(machine_check)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = cpu->po & ea_r;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = 0300;
//cpu->fault.vecoff = 0;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 071;
  longjmp(cpu->endop, endop_check);
}

static inline void __attribute__ ((noreturn)) E50X(memory_check)(cpu_t *cpu, uint32_t addr)
{
  cpu->fault.pc     = cpu->po;
  cpu->fault.ring   = cpu->po & ea_r;
  cpu->fault.faddr  = addr;
  cpu->fault.vector = 0310;
//cpu->fault.vecoff = 0;
  cpu->fault.km     = cpu->crs->km;
  cpu->fault.fcode  = 0;
  cpu->fault.offset = 077;
  longjmp(cpu->endop, endop_check);
}


static inline void E50X(rxm_check)(cpu_t *cpu)
{
  if(ea_ring(cpu->pb) != 0)
    E50X(rxm_fault)(cpu);
}

static inline void E50X(int_ovf)(cpu_t *cpu)
{
  if(cpu->crs->km.iex && cpu->crs->km.cbit)
    E50X(arith_fault)(cpu, 0x300, 0);
}

static inline void E50X(int_div0)(cpu_t *cpu)
{
  if(cpu->crs->km.iex && cpu->crs->km.cbit)
    E50X(arith_fault)(cpu, 0x301, cpu->po);
}
