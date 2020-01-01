/* Disk Operations
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

#include "io.h"

#include "cntl.h"

#include "disk.h"


#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


#ifdef DEBUG
static const char *dk_order_name[16] = { "dhlt", "1", "sform", "sseek", "dsel", "sread", "swrite", "dstall", "8", "dstat", "sstor", "doar", "sload", "sdma", "dint", "dtran" };
#endif


static void *ex_chp(void *arg)
{
dk_t *dk = arg;
cpu_t *cpu = dk->cpu;

  dk->stat = DK_STAT_OK;

  while(1) {

    uint32_t order = ifetch_d(cpu, dk->oar);
    int opcde = order >> 28;
    int mask = (order >> 22) & 0b111111;

    logmsg("disk %03o:%d opcode[%04x] = %08x mask %2.2x %s\n", dk->ctrl, dk->mhd, dk->oar, order, mask, dk_order_name[opcde]);

    if(((mask & DK_MEXIF) != 0) ^ (((mask & DK_MDERR) && (dk->stat & (DK_STAT_SEEKERR | DK_STAT_SELERR | DK_STAT_UNAVAIL))) != 0))
    {
logmsg("disk %03o:%d skip\n", dk->ctrl, dk->mhd);
      switch(opcde) {
        case DK_SFORM:
        case DK_SREAD:
        case DK_SWRITE:
          dk->oar += 3;
          break;
        default:
          dk->oar += 2;
      }
    }
    else
    switch(opcde) {

      uint32_t track;
      uint32_t record;
      uint32_t head;
      uint16_t ext;
      uint32_t size;

      case DK_DHLT:
        dk->oar += 2;
logmsg("disk %03o:%d stat %4.4x\n", dk->ctrl, dk->mhd, dk->stat);
        return NULL;
        break;

      case DK_SFORM:
        if(dk->mhd >= 0)
        {
          ext = ifetch_w(cpu, dk->oar+2);
          size = recsize[(order >> 16) & 0xf];
//        off_t off = order >> 28;
//        sr = (order >> 27) & 1;
          track = order & 0x7ff;
          record = ext >> 8;
          head = ext & 0xff; // TODO MAYBE 1F
          logmsg("disk %03o:%d sform %4.4x %4.4x size %u head %u track %u records %u\n",
            dk->ctrl, dk->mhd, order & 0xffff, ext, size, head, track, record);
          dk->stat |= dk_format(&dk->dm[dk->mhd], size, head, track, record);
        }
        else
          dk->stat |= DK_STAT_SELERR;
        dk->oar += 3;
        break;

      case DK_SSEEK:
        dk->oar += 2;
        break;

      case DK_DSEL:
        dk->oar += 2;
        dk->mhd = dk_unit(order);
        if(dk->mhd >= 0)
        {
          if(dk->dm[dk->mhd].fd == -1)
            dk_open(&dk->dm[dk->mhd]);
          dk->stat |= DK_STAT_OK;
        }
        else
          dk->stat |= DK_STAT_SELERR;
        break;

      case DK_SREAD:
        if(dk->mhd >= 0)
        {
          ext = ifetch_w(cpu, dk->oar+2);
          size = recsize[(order >> 16) & 0xf];
//        off_t off = order >> 28;
//        sr = (order >> 27) & 1;
          track = order & 0x7ff;
          record = ext >> 8;
          head = ext & 0xff; // TODO MAYBE 1F
#ifdef DEBUG
          uint16_t addr = G_DMA_A(cpu, dk->ca);
          logmsg("disk %03o:%d sread %4.4x %4.4x addr %4.4x size %u head %u track %u record %u\n",
            dk->ctrl, dk->mhd, order & 0xffff, ext, addr, size, head, track, record);
#endif
          uint8_t buffer[size<<1];
          dk->stat |= dk_read(&dk->dm[dk->mhd], buffer, size, head, track, record);
          if(dk->stat == DK_STAT_OK)
            io_dma_copy(cpu, dk->ca, dk->cn, buffer, size<<1, 0);
        }
        else
          dk->stat |= DK_STAT_SELERR;
        dk->oar += 3;
        break;

      case DK_SWRITE:
        if(dk->mhd >= 0)
        {
          ext = ifetch_w(cpu, dk->oar+2);
          size = recsize[(order >> 16) & 0xf];
//        off_t off = order >> 28;
//        sr = (order >> 27) & 1;
          track = order & 0x7ff;
          record = ext >> 8;
          head = ext & 0xff; // TODO MAYBE 1F
#ifdef DEBUG
          uint16_t addr = G_DMA_A(cpu, dk->ca);
          logmsg("disk %03o:%d swrit %4.4x %4.4x addr %4.4x size %u head %u track %u record %u\n",
            dk->ctrl, dk->mhd, order & 0xffff, ext, addr, size, head, track, record);
#endif
          uint8_t buffer[size<<1];
          io_dma_copy(cpu, dk->ca, dk->cn, buffer, size<<1, 1);
          dk->stat |= dk_write(&dk->dm[dk->mhd], buffer, size, head, track, record);
        }
        else
          dk->stat |= DK_STAT_SELERR;
        dk->oar += 3;
        break;

      case DK_DSTALL:
        dk->oar += 2;
        usleep(210ULL);
        break;

      case DK_DSTAT:
        dk->oar += 2;
logmsg("disk %03o:%d stat %4.4x\n", dk->ctrl, dk->mhd, dk->stat);
        istore_w(cpu, order & 0xffff, dk->stat);
        break;

      case DK_SSTOR:
        dk->oar += 2;
        break;

      case DK_DOAR:
        dk->oar += 2;
        istore_w(cpu, order & 0xffff, dk->oar);
        break;

      case DK_SLOAD:
        dk->oar += 2;
        break;

      case DK_SDMA:
        dk->oar += 2;
        dk->cn = (order >> 16) & 0xf;
        dk->ca = order & 0xffff;
        break;

      case DK_DINT:
        dk->oar += 2;
        io_setintv(cpu, &dk->intr, order & 0xffff);
        return NULL; // suspend
        break;

      case DK_DTRAN:
        dk->oar = order & 0xffff;
        break;

      default:
        dk->oar += 2;
    }
  }
}


static void *dk_thread(void *parm)
{
dk_t *dk = parm;

  char tname[16];
  snprintf(tname, sizeof(tname), "disk %03o", dk->ctrl);
  pthread_setname_np(pthread_self(), tname);

  pthread_mutex_lock(&dk->pthread.mutex);
  while(dk->pthread.tid)
  {
    pthread_cond_wait(&dk->pthread.cond, &dk->pthread.mutex);
    dk->busy = 0;
    ex_chp(dk);
  }
  pthread_mutex_unlock(&dk->pthread.mutex);
  return NULL;
}


static void dk_init(cpu_t *cpu, int type, int ext, int func, int ctrl, dk_t **dk, int argc, char *argv[])
{
  (*dk) = calloc(1, sizeof(dk_t));
  (*dk)->id = io_id(026, ctrl);
  (*dk)->ctrl = ctrl;

  for(int mhd = 0; mhd < (sizeof((*dk)->dm)/sizeof(*(*dk)->dm)); ++mhd)
  {
    char genname[80];
    snprintf(genname, sizeof(genname), "%s/dk0%o%o", cpu->sys->hdir, ctrl, mhd);

    (*dk)->dm[mhd].fn = strdup(c_fname(genname));
    (*dk)->dm[mhd].fd = -1;
    (*dk)->dm[mhd].dk = (*dk);
  }

  (*dk)->intr.i = -1;
  (*dk)->cpu = cpu;
  pthread_attr_init(&(*dk)->pthread.attr);
  pthread_attr_setdetachstate(&(*dk)->pthread.attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setinheritsched(&(*dk)->pthread.attr, PTHREAD_EXPLICIT_SCHED);
  if(cpu->sys->cap_sys_nice)
  {
    pthread_attr_setschedpolicy(&(*dk)->pthread.attr, SCHED_RR);
//  const struct sched_param param = { .sched_priority = sched_get_priority_max(SCHED_RR) };
//  pthread_attr_setschedparam(&(*dk)->pthread.attr, &param);
  }
  pthread_mutex_init(&(*dk)->pthread.mutex, NULL);
  pthread_cond_init(&(*dk)->pthread.cond, NULL);
  pthread_create(&(*dk)->pthread.tid, &(*dk)->pthread.attr, dk_thread, *dk);
}


int disk_io(cpu_t *cpu, int type, int ext, int func, int ctrl, void **devparm, int argc, char *argv[])
{
dk_t *dk = *devparm;

  switch(type) {
    case IO_TYPE_OCP:
      switch(func) {
        case 016: // Clear Interrupt
          logmsg("disk %03o Clear Interrupt\n", ctrl);
          if(io_clrint(cpu, &dk->intr))
            pthread_cond_signal(&dk->pthread.cond);
          break;
        case 017: // Initialize
          break;
        default:
          logall("disk %03o unsupported OCP order %03o\n", ctrl, func);
      }
      return 0;
      break;
    case IO_TYPE_SKS:
      switch(func) {
        case 04: // Skip if not interrupting
          logmsg("disk %03o Skip if not Interrupting\n", ctrl);
          if(io_tstint(cpu, &dk->intr))
            return 0;
          break;
        default:
          logall("disk %03o unsupported SKS order %03o\n", ctrl, func);
      }
      break;
    case IO_TYPE_INA:
#if 1
      if(dk->busy || pthread_mutex_trylock(&dk->pthread.mutex))
      {
        pthread_yield();
        return 0;
      }
#else
      pthread_mutex_lock(&dk->pthread.mutex);
#endif
      switch(func) {
        case 011: // Input ID
          S_A(cpu, dk->id);
          break;
        case 017: // Input Order Address Register
          logmsg("disk %03o Input Order Address Register %4.4x\n", ctrl, dk->oar);
          S_A(cpu, dk->oar);
          break;
        default:
          logall("disk %03o unsupported INA order %03o\n", ctrl, func);
      }
      pthread_mutex_unlock(&dk->pthread.mutex);
      break;
    case IO_TYPE_OTA:
#if 0 // returning busy can result in disk io failures
      if(dk->busy || pthread_mutex_trylock(&dk->pthread.mutex))
      {
        pthread_yield();
        return 0;
      }
#else
      pthread_mutex_lock(&dk->pthread.mutex);
#endif
      switch(func) {
        case 017: // Load Order Address Start
          dk->oar = G_A(cpu);
          logmsg("disk %03o Load Order Address Start %4.4x\n", ctrl, dk->oar);
          dk->busy = 1;
          pthread_cond_signal(&dk->pthread.cond);
          break;
        default:
          logall("disk %03o unsupported OTA order %03o %4.4x\n", ctrl, func, G_A(cpu));
      }
      pthread_mutex_unlock(&dk->pthread.mutex);
      break;
    case IO_TYPE_IPL:
      pthread_mutex_lock(&dk->pthread.mutex);
      dk_open(&dk->dm[ext]);
      if(dk_read(&dk->dm[ext], physad(cpu, DK_ADDR_IPL), recsize[0], 0, 0, 0) != DK_STAT_OK)
        logmsg("disk %03o:%d boot failed\n", ctrl, ext);
      cpu->srf.drf.dma_h[040] = DK_ADDR_IPL + recsize[0];
      pthread_mutex_unlock(&dk->pthread.mutex);
      break;
    case IO_TYPE_INI:
      dk_init(cpu, type, ext, func, ctrl, (dk_t **)devparm, argc, argv);
      break;
    case IO_TYPE_CLS:
      break;
    case IO_TYPE_ASN:
      if(argc > 0)
      {
        pthread_mutex_lock(&dk->pthread.mutex);
        if(dk->dm[ext].fn)
          free(dk->dm[ext].fn);
        dk->dm[ext].fn =  strdup(argv[0]);
        dk_close(&dk->dm[ext]);
        isfile(dk->dm[ext].fn);
        pthread_mutex_unlock(&dk->pthread.mutex);
      }
      else
        if(dk->dm[ext].fn)
          printf("ASSIGN %03o:%1o %s\n", ctrl, ext, dk->dm[ext].fn);
      break;
    default:
      abort();
  }

  return 1;
}
