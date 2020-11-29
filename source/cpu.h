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


#ifndef _cpu_h
#define _cpu_h

typedef void (*run_cpu_t)(cpu_t *);

extern void e16s_run_cpu(cpu_t *);
extern void e32s_run_cpu(cpu_t *);
extern void e64r_run_cpu(cpu_t *);
extern void e32r_run_cpu(cpu_t *);
extern void e32i_run_cpu(cpu_t *);
extern void e64v_run_cpu(cpu_t *);


static inline void cpu_reset(cpu_t *cpu)
{
  memset(&(cpu->srf), 0, sizeof(cpu->srf));
  mm_ptlb(cpu);
  mm_piotlb(cpu);
  cpu->crn = 0;
  cpu->crs = &cpu->srf.urs[0];
  cpu->crs->timer = 0;
  S_DMA_0(cpu, 6, 01000);
  S_RB(cpu, RESET_PC);
}

#endif
