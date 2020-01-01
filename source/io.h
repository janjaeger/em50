/* I/O Instructions
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


#ifndef _io_h
#define _io_h

void io_reset(cpu_t *);
int  io_assign(cpu_t *, int, int, int, char *[]);
int  io_load(cpu_t *, int, int);

uint16_t ifetch_w(cpu_t *, uint32_t);
uint32_t ifetch_d(cpu_t *, uint32_t);
void istore_w(cpu_t *, uint32_t, uint16_t);
void istore_d(cpu_t *, uint32_t, uint32_t);
int32_t i2r(cpu_t *, uint32_t);


typedef struct intr_t {
  volatile int      i;
  volatile uint16_t v;
} intr_t;

static inline void io_intr_init(cpu_t *cpu)
{
  pthread_mutex_init(&cpu->intr.mutex, NULL);
  cpu->intr.a = cpu->intr.n = 0;
  for(int n = 0; n < INTR_QSIZE; ++n)
    cpu->intr.v[n] = -1;
  cpu->intr.c = -1;
}

static inline uint16_t io_devslot(uint16_t devid, uint16_t slot)
{
  return ((slot << 8) & 0x1f00) | (devid & ~0x1f00);
}

static inline int io_getslot(uint16_t devid)
{
  return (devid >> 8) & 0x1f;
}

static inline uint16_t io_id(uint16_t devid, int ctrl)
{
static const uint16_t fixed_slot[] = { 054, 053, 052, 035, 015, 016, 017, 032 };
#define num_fixed_slots (sizeof(fixed_slot)/sizeof(*fixed_slot))
static uint16_t slot = num_fixed_slots;

  for(int n = 0; n < num_fixed_slots; ++n)
    if(fixed_slot[n] == ctrl)
      return io_devslot(devid, n);

  return io_devslot(devid, slot++);
}

static inline int io_pending(cpu_t *cpu)
{
  return cpu->intr.a != cpu->intr.n;
}

static inline int io_incrnxt(int i)
{
  return (i + 1) & INTR_QMASK;
}

static inline int io_incrint(volatile int *i)
{
  return (*i = io_incrnxt(*i));
}

static inline int io_tstint(cpu_t *cpu, intr_t *intr)
{
  return intr->i >= 0 && intr->v == cpu->intr.v[intr->i];
}

static inline void io_setv(intr_t *intr, uint16_t v)
{
  intr->v = v;
}

static inline void io_setint(cpu_t *cpu, intr_t *intr)
{
//cpu_post(cpu);

  if(io_tstint(cpu, intr))
    return;

  pthread_mutex_lock(&cpu->intr.mutex);
  int n = io_incrint(&cpu->intr.n);
  cpu->intr.v[n] = intr->v;
  pthread_mutex_unlock(&cpu->intr.mutex);
  intr->i = n;
logall("\nsetint v %4.4x i %d (%d)\n", intr->v, n, intr->i);
}

static inline void io_setintv(cpu_t *cpu, intr_t *intr, uint16_t v)
{
  io_setv(intr, v);

  io_setint(cpu, intr);
}

static inline int32_t io_intvec(cpu_t *cpu)
{
int32_t r = -1;
int a;

  if(pthread_mutex_trylock(&cpu->intr.mutex))
    return -1;

  while(io_pending(cpu))
  {
        a = io_incrint(&cpu->intr.a);

    if(cpu->intr.v[a] >= 0)
    {
      r = cpu->intr.c = cpu->intr.v[a];
      break;
    }
  }

  pthread_mutex_unlock(&cpu->intr.mutex);

logall("\nintvec v %4.4x [%d]\n", r, a);

  return r;
}

static inline int io_clrint(cpu_t *cpu, intr_t *intr)
{
int rc;
logall("\nclrint v %4.4x i %d\n", intr->v, intr->i);
  if(io_tstint(cpu, intr))
    rc = cpu->intr.v[intr->i] = -1;
  else
    rc = 0;
  
  intr->i = -1;

  return rc;
}

static inline void io_clrai(cpu_t *cpu)
{
logall("\nclrai v %4.4x i %d\n", cpu->intr.v[cpu->intr.a], cpu->intr.a);
  if(cpu->intr.c >= 0)
  {
    cpu->intr.c = cpu->intr.v[cpu->intr.a] = -1;
//  io_incrint(&cpu->intr.a);
  }
}


static inline void io_mem_copy(cpu_t *cpu, uint32_t addr, uint8_t *buffer, ssize_t len, int wr)
{
  for(int n = 0; n < len; n += 2)
    if(wr)
    {
      uint16_t w = ifetch_w(cpu, addr + (n >> 1));
      buffer[n] = w >> 8;
      buffer[n + 1] = w & 0xff;
    }
    else
      istore_w(cpu, addr + (n >> 1), (buffer[n] << 8) | buffer[n + 1]);
}


static inline size_t io_dma_copy(cpu_t *cpu, int ca, int cn, uint8_t *buffer, ssize_t len, int wr)
{
size_t bytes = 0;

  for(int n = ca; n <= ca + (cn<<1) && n < 040; n += 2) // FIXME TODO
  {
  int dma_channel = n & 037;
  ssize_t xfer = G_DMA_L(cpu, dma_channel) << 1;
  uint32_t addr = G_DMA_A(cpu, dma_channel);

    if(!addr)
      break;

    if(len < xfer)
      xfer = len;

    if(xfer && len)
    {
      logmsg("dma: copying %zd bytes %s %4.4x\n", xfer, wr ? "from" : "to", addr);
      io_mem_copy(cpu, G_DMA_A(cpu, dma_channel), buffer, xfer, wr);
      buffer += xfer;
      bytes += xfer;
      len -= xfer;
      logmsg("dma: copy sta addr %4.4x xfer %4.4x\n", G_DMA_A(cpu, dma_channel), G_DMA_L(cpu, dma_channel));
      S_DMA_A(cpu, dma_channel, G_DMA_A(cpu, dma_channel) + (xfer >> 1));
      S_DMA_L(cpu, dma_channel, G_DMA_L(cpu, dma_channel) - (xfer >> 1));
      logmsg("dma: copy end addr %4.4x xfer %4.4x\n", G_DMA_A(cpu, dma_channel), G_DMA_L(cpu, dma_channel));
    }
  }

  logmsg("dma: copied %zu bytes\n", bytes);
  return bytes;
}


static inline size_t io_dmc_copy(cpu_t *cpu, uint16_t channel, uint8_t *buffer, size_t len, int wr)
{
size_t bytes = 0;
int dc = channel & 0x7ff /* MT_DC_CHA */;  // TODO FIXME

  while(len > 0)
  {
  uint32_t dmc_sta = ifetch_w(cpu, dc);
  uint32_t dmc_end = ifetch_w(cpu, dc+1);

  if(!dmc_sta || dmc_end < dmc_sta)
    break;

  size_t dmc_len = (1 + dmc_end - dmc_sta) << 1;
  size_t xfer = len > dmc_len ? dmc_len : len;

    logmsg("dmc: sta %4x end %4x len %4zx(%zu)\n", dmc_sta, dmc_end, xfer >> 1, xfer);

    if(xfer)
    {
      logmsg("dmc: copying %zd bytes to %4.4x\n", xfer, dmc_sta);
      io_mem_copy(cpu, dmc_sta, buffer, xfer, wr);
      buffer += xfer;
      bytes += xfer;
      len -= xfer;
      dmc_sta += (xfer >> 1);
      istore_w(cpu, dc, dmc_sta);
    }

    logmsg("dmc: sta %4x end %4x len %4zx(%zu)\n", dmc_sta, dmc_end, xfer >> 1, xfer);

    dc += 2;
  }

  logmsg("dmc: copied %zu bytes\n", bytes);
  return bytes;
}


static inline int cpboot(cpu_t *cpu)
{
  if(!(cpu->sys->cpboot && *cpu->sys->cpboot) || isfilex(cpu->sys->cpboot))
    return 0;

  int fd = open(cpu->sys->cpboot, O_RDONLY);

  if(fd == -1)
    return 0;

  struct {
    uint16_t sa;
    uint16_t ea;
    uint16_t pc;
    uint16_t a;
    uint16_t b;
    uint16_t x;
    uint16_t k;
    uint16_t u[2];
  } __attribute__ ((packed)) cphdr;

  read(fd, &cphdr, sizeof(cphdr));
  uint8_t *cpboot = physad(cpu, from_be_16(cphdr.sa));

  read(fd, cpboot, (1+from_be_16(cphdr.ea)-from_be_16(cphdr.sa))<<1);

  close(fd);

  S_RB(cpu, from_be_16(cphdr.pc));
  S_A(cpu, from_be_16(cphdr.a));
  S_B(cpu, from_be_16(cphdr.b));
  S_X(cpu, from_be_16(cphdr.x));
  S_KEYS(cpu, from_be_16(cphdr.k));

  return 1;
}


#define IO_TYPE_OCP 0
#define IO_TYPE_SKS 1
#define IO_TYPE_INA 2
#define IO_TYPE_OTA 3

#define IO_TYPE_INI 4
#define IO_TYPE_CLS 5
#define IO_TYPE_IPL 6
#define IO_TYPE_ASN 7

#endif
