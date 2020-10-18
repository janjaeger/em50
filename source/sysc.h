/* System Console
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


#ifndef _sysc_h
#define _sysc_h

typedef struct sc_t {
  cpu_t *cpu;
  uint16_t ctrl;
//struct {
//  pthread_t tid;
//  pthread_attr_t attr;
//} pthread;
  uint16_t id;  
  uint16_t rv;  /* Receive Interrupt Vector */
  uint16_t tv;  /* Transmit Interrupt Vector */
  uint16_t rc;  /* Receive Interrupt DMA Channel */
  uint16_t tc;  /* Transmit Interrupt DMA Channel */
  uint16_t rm;  /* Receive Interrupt Mask */
  uint16_t tm;  /* Transmit Interrupt Mask */
  uint16_t r1;  /* Receive Control Register 1 */
  uint16_t t1;  /* Transmit Control Register 1 */
  uint16_t r2;  /* Receive Control Register 2 */
  uint16_t t2;  /* Transmit Control Register 2 */
  int im; /* Interrupt mask */
  intr_t ri;
  intr_t ti;
  int echo;
  int kb[2];
  int pr[2];
} sc_t;

int sysc_io(cpu_t *cpu, int, int, int, int, void **, int, char *[]);
void sysc_input(cpu_t *, char *);
int sysc_term(cpu_t *);

#endif
