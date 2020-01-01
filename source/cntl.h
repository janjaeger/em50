/* Control Panel / System Option Controller / Diagnostic Processor
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


#ifndef _cntl_h
#define _cntl_h

typedef struct cp_t {
  cpu_t *cpu;
  uint16_t ctrl;
  struct {
    struct timespec ts;
    pthread_t tid;
    pthread_attr_t attr;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
  } pthread;
  uint16_t id;
  uint16_t iv;
  uint16_t ir;
  uint16_t mi;
  uint16_t cr;
#define SOC_CR_EMI  040
#define SOC_CR_PICR 020
#define SOC_CR_ELFC 010
#define SOC_CR_EPIC 004
#define SOC_CR_SPIC 002
#define SOC_CR_WDT  001
#define SOC_CR_ACT  077
  uint16_t li;
  int im;
  struct intr_t intr;
} cp_t;

int cntl_io(cpu_t *cpu, int, int, int, int, void **, int, char *[]);

#endif
