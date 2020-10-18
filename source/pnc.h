/* Primenet Node Controller
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


#ifndef _pnc_h
#define _pnc_h

#define PNC_BACKLOG 5
#define PNC_DFLTPORT 2324
#define PNC_BUFSZ 4096

typedef struct buff_t {
  union {
  struct {
  uint16_t len;
  uint8_t  buff[PNC_BUFSZ];
  } __attribute__ ((packed)) data;
  uint8_t raw[0];
  };
  int len;
} buff_t;

typedef struct link_t {
  struct link_t *next;
  int fd;
  uint8_t nn;
  buff_t rb;
} link_t;

typedef struct pnc_t {
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
  uint16_t nn; // Node Number
  uint16_t ns; // Network Status
#define PNC_NS_RXI   0x8000
#define PNC_NS_TXI   0x4000
#define PNC_NS_BST   0x2000
#define PNC_NS_NEW   0x1000
#define PNC_NS_BAD   0x0800
#define PNC_NS_CONN  0x0400
#define PNC_NS_MTD   0x0200
#define PNC_NS_TD    0x0100
  uint16_t iv; // Interrupt Vector
#define PNC_TX_VEC(_v) ((_v) | 1)
#define PNC_RX_VEC(_v) ((_v) & ~1)
  int      im; // Interrupt Mask
  uint16_t rx; // Receive DMA Channel
  uint16_t tx; // Transmit DMA Channel
  uint16_t rs; // Receive Status
#define PNC_XS_ACK   0x8000
#define PNC_XS_MACK  0x4000
#define PNC_XS_WACK  0x2000
#define PNC_XS_NACK  0x0400
#define PNC_XS_PERR  0x0200
#define PNC_XS_CERR  0x0100
#define PNC_RS_EOR   0x0080
#define PNC_RS_BUSY  0x0040
#define PNC_RS_RBER  0x0020
  uint16_t ts; // Transmit Status
#define PNC_TS_TBER  0x0080
#define PNC_TS_BUSY  0x0040
#define PNC_TS_NRET  0x0020
#define PNC_TS_CERR  0x0010
#define PNC_TS_RERR  0x0008
#define PNC_TS_RMASK 0x0007
  uint16_t dr; // Diagnostic Register

  struct intr_t intrx;
  struct intr_t inttx;

  int pipe[2];

  int rxrdy;
  int txrdy;

  link_t *link;

  buff_t tb;
} pnc_t;

int pnc_io(cpu_t *cpu, int, int, int, int, void **, int, char *[]);

#endif
