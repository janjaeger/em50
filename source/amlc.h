/* Asynchronous Multi Line Controller
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


#ifndef _amlc_h
#define _amlc_h

#define AMLC_LINES 16

#define AMLC_TX_VAL  0x8000
#define AMLC_TX_SPC  0x4000
#define AMLC_TX_IDLE 0x2000
#define AMLC_TX_CHAR 0x00FF

#define AMLC_RX_LINE 0xF000
#define AMLC_RX_VAL  0x0200
#define AMLC_RX_PARE 0x0100
#define AMLC_RX_CHAR 0x00FF
#define AMLC_RX_ENS  0x0004
#define AMLC_RX_EOVR 0x0002
#define AMLC_RX_EBRK 0x0001

struct amlc_t;
typedef struct {
  struct amlc_t *amlc;
  int no;
  uint16_t cf; // Configuration
#define AMLC_CF_DSC  0x0400
#define AMLC_CF_LOOP 0x0200
#define AMLC_CF_RATE 0x01C0
#define AMLC_CF_STBT 0x0010
#define AMLC_CF_PDIS 0x0008
#define AMLC_CF_PAR  0x0004
#define AMLC_CF_CLEN 0x0003
  uint16_t cn; // Control
#define AMLC_CN_TIME 0x0020
#define AMLC_CN_XMIT 0x0008
#define AMLC_CN_ECHO 0x0004
#define AMLC_CN_RBRK 0x0002
#define AMLC_CN_RECV 0x0001
  uint16_t ds; // Dataset
#define AMLC_DS_DSC4 0x0008   // LOCAL / BUSY
#define AMLC_DS_DSC3 0x0004   // STD
#define AMLC_DS_DSC2 0x0002   // DTR
#define AMLC_DS_DSC1 0x0001   // RTS
  int fds, fdr;
  enum { offl = 0, pend, onln, loop, conn } ls;
  struct {
    int amlc; // amlc device number to connect to
    int ln;   // line number to connect to
  } conn;
#ifdef LIBTELNET
  telnet_t *telnet;
  void **tnparm;
#endif
} line_t;

typedef struct amlc_t {
  cpu_t *cpu;
  intr_t intr;
  uint16_t ctrl;
  uint16_t id;  
  uint16_t va;
  uint16_t ca;
  uint16_t c1;
  uint16_t da;
  uint16_t cl;
  uint16_t st;
#define AMLC_ST_EOR  0x8000   // End of Range
#define AMLC_ST_CLK  0x4000   // Clock Running
#define AMLC_ST_DSC  0x0200   // Dataset Set Status Changed Interrupt
#define AMLC_ST_BUF  0x0100   // Receiving into buffer 0 / 1
#define AMLC_ST_CTI1 0x0080   // Character Time Interrupt 1
#define AMLC_ST_CTI2 0x0040   // Character Time Interrupt 2
#define AMLC_ST_IENA 0x0020   // Interrupts Enabled
#define AMLC_ST_DMQ  0x0010   // DMQ Mode
#define AMLC_ST_LINE 0x000F   // CTI Line number
  int ra;
  int im;
  int dm;
  int dv;
  line_t ln[AMLC_LINES];
  struct {
    pthread_t tid;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
  } pthread;
  int in;
} amlc_t;

int amlc_io(cpu_t *cpu, int, int, int, int, void **, int, char *[]);

#endif
