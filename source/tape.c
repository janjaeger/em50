/* Tape Operations
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


#if 0
#undef logall
#define logall(...) PRINTF(__VA_ARGS__)
#endif


#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


#include "io.h"

#include "tape.h"

#ifdef DEBUG
static const char *mtstat[] = { "TMK", "ERR", "EOM", "BOF", "BOT", "OFL", "ONL", "WTM" };
static const char *drn[] = { "current status word", "id number", "dmx channel number", "vector interrupt addr" };
#endif

static inline size_t copy_io_buffer(cpu_t *cpu, mt_t *mt, uint8_t *buffer, ssize_t len, int write)
{
  if(!(mt->dc & MT_DC_DMC))
    return io_dma_copy(cpu, mt->dc & 037, 040, buffer, len, write);
  else // DMC
    return io_dmc_copy(cpu, mt->dc & MT_DC_CHA, buffer, len, write);
}


static inline void setsw(tm_t *tm, ssize_t rc)
{
  tm->sw = 0;

  int st = mt_stat(tm);

  switch(st) {
    case MT_BOT:
      tm->sw |= MT_SW_ONL | MT_SW_RDY | MT_SW_BOT;
      break;
    case MT_EOM:
      tm->sw |= MT_SW_ONL | MT_SW_RDY | MT_SW_EOT;
      break;
    case MT_ONL:
      tm->sw |= MT_SW_ONL | MT_SW_RDY;
      break;
  }

  switch(rc) {
    case MT_TMK:
      tm->sw |= MT_SW_TMK;
      break;
    case MT_ERR:
      tm->sw |= MT_SW_UNC;
      break;
    case MT_EOM:
      tm->sw |= MT_SW_EOT;
      break;
    case MT_BOF:
      tm->sw |= MT_SW_BOT;
      break;
    case MT_BOT:
      tm->sw |= MT_SW_BOT; // | MT_SW_INT;
      break;
  }
}


static inline void motion_setup(mt_t *mt)
{
cpu_t *cpu = mt->cpu;
uint16_t order = mt->mo & 0xe0f0;
int dev = mt_unit(mt->mo);
ssize_t rc = 0;
  
uint8_t buffer[65536];

uint8_t *addr = buffer;
ssize_t len = sizeof(buffer);

  mt->sw = MT_SW_ONL;

  if(!(mt->mo & 0x0100))
    logmsg("tape %03o not supported: one byte / word\n", mt->ctrl);

  if(dev < 0)
    dev = 0;

  tm_t *tm = &mt->tm[dev];

  if(tm->fd == -1)
  {
    mt_open(tm);
    setsw(tm, MT_ONL);
  }

  switch(order) {
    case 0x0020: // Rewind
      logmsg("tape %03o:%d rewind\n", mt->ctrl, dev);
      rc = mt_rew(tm);
      setsw(tm, rc);
      mt_close(tm);
      break;
    case 0x4080: // Read Record Forward
      logmsg("tape %03o:%d read\n", mt->ctrl, dev);
      rc = mt_read(tm, addr, len);
      logmsg("tape %zd bytes\n", rc);
      if(rc > 0)
        rc = copy_io_buffer(cpu, mt, buffer, rc, 0);
      setsw(tm, rc);
      break;
    case 0x6080: // Skip Record Forward
      logmsg("tape %03o:%d fsr\n", mt->ctrl, dev);
      rc = mt_read(tm, NULL, 0);
      setsw(tm, rc);
      break;
    case 0x2080: // Skip File Forward
      logmsg("tape %03o:%d fsf\n", mt->ctrl, dev);
      rc = mt_fsf(tm);
      setsw(tm, rc);
      break;
    case 0x4040: // Read Record Backward
      logmsg("tape %03o:%d rdbk\n", mt->ctrl, dev);
      rc = mt_rdbk(tm, addr, len);
//    if(rc > 0)
//      rc = copy_io_buffer(cpu, mt, buffer, rc, 0);
      setsw(tm, rc);
      break;
    case 0x6040: // Skip Record Backward
      logmsg("tape %03o:%d bsr\n", mt->ctrl, dev);
      rc = mt_rdbk(tm, NULL, 0);
      setsw(tm, rc);
      break;
    case 0x2040: // Skip File Backward
      logmsg("tape %03o:%d bsf\n", mt->ctrl, dev);
      rc = mt_bsf(tm);
      setsw(tm, rc);
      break;
    case 0x4090: // Write (forward)
      logmsg("tape %03o:%d write\n", mt->ctrl, dev);
      len = copy_io_buffer(cpu, mt, buffer, len, 1);
      rc = mt_write(tm, addr, len);
      setsw(tm, rc);
      break;
    case 0x2090: // Write tape mark
      logmsg("tape %03o:%d write tape mark\n", mt->ctrl, dev);
      rc = mt_write(tm, NULL, 0);
      setsw(tm, rc);
      break;
    case 0x8000: // Transport
      logmsg("tape %03o:%d transport\n", mt->ctrl, dev);
      rc = MT_ONL;
      break;
    default:
      logall("tape %03o invalid setup motion order %4.4x\n", mt->ctrl, order);
    }

  mt->sw = tm->sw;
  mt->pending = 1;

  if(rc > 0)
    logmsg("tape %03o:%d %zd bytes transferred\n", mt->ctrl, dev, rc);
  else
    logmsg("tape %03o:%d status %s\n", mt->ctrl, dev, mtstat[-rc]);

  if(mt->ff)
    io_setintv(cpu, &mt->intr, mt->va);
}


static void *mt_thread(void *parm)
{
mt_t *mt = parm;

  char tname[16];
  snprintf(tname, sizeof(tname), "tape %03o", mt->ctrl);
  pthread_setname_np(pthread_self(), tname);

  pthread_mutex_lock(&mt->pthread.mutex);
  while(mt->pthread.tid)
  {
    pthread_cond_wait(&mt->pthread.cond, &mt->pthread.mutex);
    mt->busy = 0;
    motion_setup(mt);
  }
  pthread_mutex_unlock(&mt->pthread.mutex);
  return NULL;
}


static void mt_init(cpu_t *cpu, int type, int ext, int func, int ctrl, mt_t **mt, int argc, char *argv[])
{
  (*mt) = calloc(1, sizeof(mt_t));
  (*mt)->id = io_id(0114, ctrl);
  (*mt)->ctrl = ctrl;
  (*mt)->va = 0114;
  (*mt)->cdr = current_status_word;
  (*mt)->sw = MT_SW_ONL;

  for(int dv = 0; dv < (sizeof((*mt)->tm)/sizeof(*(*mt)->tm)); ++dv)
  {
    char genname[100];
    snprintf(genname, sizeof(genname), "%s/mt0%o%o", cpu->sys->hdir, ctrl, dv);
  
    (*mt)->tm[dv].fn = strdup(c_fname(genname));
    (*mt)->tm[dv].fd = -1;
    (*mt)->tm[dv].mt = (*mt);
    (*mt)->tm[dv].sw = (*mt)->sw;
  }

  (*mt)->intr.i = -1;
  (*mt)->cpu = cpu;
  pthread_attr_init(&(*mt)->pthread.attr);
  pthread_attr_setdetachstate(&(*mt)->pthread.attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setinheritsched(&(*mt)->pthread.attr, PTHREAD_EXPLICIT_SCHED);
  if(cpu->sys->cap_sys_nice)
  {
    pthread_attr_setschedpolicy(&(*mt)->pthread.attr, SCHED_RR);
//  const struct sched_param param = { .sched_priority = sched_get_priority_max(SCHED_RR) };
//  pthread_attr_setschedparam(&(*mt)->pthread.attr, &param);
  }
  pthread_mutex_init(&(*mt)->pthread.mutex, NULL);
  pthread_cond_init(&(*mt)->pthread.cond, NULL);
  pthread_create(&(*mt)->pthread.tid, &(*mt)->pthread.attr, mt_thread, *mt);
}


int tape_io(cpu_t *cpu, int type, int ext, int func, int ctrl, void **devparm, int argc, char *argv[])
{
mt_t *mt = *devparm;

  switch(type) {
    case IO_TYPE_OCP:
      switch(func) {
        case 014: // Acknowledge Interrupt
          logmsg("tape %03o acknowledge interrupt\n", mt->ctrl);
          io_clrint(cpu, &mt->intr);
          break;
        case 015: // Set Interrupt Mask
          logmsg("tape %03o set interrupt mask\n", mt->ctrl);
          mt->ff = 1;
if(mt->pending) io_setintv(cpu, &mt->intr, mt->va);
          break;
        case 016: // Clear Interrupt Mask
          logmsg("tape %03o clear interrupt mask\n", mt->ctrl);
          mt->ff = 0;
          break;
        case 017: // Initialise
        default:
          logall("tape %03o unsupported OCP order %03o\n", mt->ctrl, func);
      }
      return 0;
      break;
    case IO_TYPE_SKS:
      switch(func) {
        case 000: // Skip if ready
          logmsg("tape %03o skip if ready\n", mt->ctrl);
          break;
        case 001: // Skip if not busy
          logmsg("tape %03o skip if not busy\n", mt->ctrl);
          return  mt->busy ? 0 : 1;
        case 004: // Skip if not Interrupting
          logmsg("tape %03o skip if not interrupting\n", mt->ctrl);
          if(io_tstint(cpu, &mt->intr))
            return 0;
          break;
        case 007: // Skip if Status Incorrect
          logmsg("tape %03o skip if status incorrect\n", mt->ctrl);
          return /* status is incorrect ? 1 : */ 0;
          break;
        default:
          logall("tape %03o unsupported SKS order %03o\n", mt->ctrl, func);
      }
      break;
    case IO_TYPE_INA:
      if(mt->busy || pthread_mutex_trylock(&mt->pthread.mutex))
      {
        pthread_yield();
        return 0;
      }
      switch(func) {
        case 000: // Input Data Register
          logmsg("tape %03o input data register: %s: %4.4x\n", mt->ctrl, drn[mt->cdr], mt->dr[mt->cdr]);
          S_A(cpu, mt->dr[mt->cdr]);
if(mt->cdr == current_status_word) mt->pending = 0;
          break;
        default:
          logall("tape %03o unsupported INA order %03o\n", mt->ctrl, func);
      }
      pthread_mutex_unlock(&mt->pthread.mutex);
      break;
    case IO_TYPE_OTA:
      if(mt->busy || pthread_mutex_trylock(&mt->pthread.mutex))
      {
        pthread_yield();
        return 0;
      }
      switch(func) {
        case 001:
          mt->mo = G_A(cpu);
          logmsg("tape %03o motion setup %4.4x\n", mt->ctrl, mt->mo);
          mt->busy = 1;
          pthread_cond_signal(&mt->pthread.cond);
          break;
        case 002: // Setup Data Register
          switch(G_A(cpu) >> 12) {
            case 0b1000:
              mt->cdr = current_status_word;
              break;
            case 0b0100:
              mt->cdr = id_number;
              break;
            case 0b0010:
              mt->cdr = dmx_channel_number;
              break;
            case 0b0001:
              mt->cdr = vector_interrupt_address;
              break;
          }
          logmsg("tape %03o setup data register: %s\n", mt->ctrl, drn[mt->cdr]);
          break;
        case 014:
          mt->dc = G_A(cpu);
          logmsg("tape %03o setup channel %s addr %04x (%4.4x)\n", mt->ctrl, (mt->dc & MT_DC_DMC) ? "dmc" : "dma", mt->dc & MT_DC_CHA, mt->dc);
          break;
        case 016: // Set interrupt vector
          mt->va = G_A(cpu);
//        mt->ff = 1;
          logmsg("tape %03o set interrupt vector %04x\n", mt->ctrl, mt->va);
          break;
        case 003: // Power On
        default:
          logall("tape %03o unsupported OTA order %03o %4.4x\n", mt->ctrl, func, G_A(cpu));
      }
      pthread_mutex_unlock(&mt->pthread.mutex);
      break;
    case IO_TYPE_IPL:
      pthread_mutex_lock(&mt->pthread.mutex);
      if(mt_open(&mt->tm[ext]) < 0)
        logall("open(%s) failed: %s\n", mt->tm[ext].fn, strerror(errno));
      else
      {
        uint16_t n = mt_read(&mt->tm[ext], physad(cpu, MT_ADDR_IPL), 8192);
        if(n > 0)
          cpu->srf.drf.dma_h[040] = (n>>1) + MT_ADDR_IPL;
        else
          logmsg("tape %03o boot failed\n", mt->ctrl);
      }
      pthread_mutex_unlock(&mt->pthread.mutex);
      break;
    case IO_TYPE_INI:
      mt_init(cpu, type, ext, func, ctrl, (mt_t **)devparm, argc, argv);
      break;
    case IO_TYPE_CLS:
      pthread_mutex_lock(&mt->pthread.mutex);
      mt_close(&mt->tm[ext]);
      pthread_mutex_unlock(&mt->pthread.mutex);
      break;
    case IO_TYPE_ASN:
      if(argc > 0)
      {
        pthread_mutex_lock(&mt->pthread.mutex);
        if(mt->tm[ext].fn)
          free(mt->tm[ext].fn);
        mt->tm[ext].fn =  strdup(argv[0]);
        if(argc > 1)
          mt->tm[ext].max = a2i(argv[1]);
        if(mt->ff)
          io_setintv(cpu, &mt->intr, mt->va);
        isfile(mt->tm[ext].fn);
        pthread_mutex_unlock(&mt->pthread.mutex);
      }
      else
        if(mt->tm[ext].fn)
          printf("ASSIGN %03o:%1o %s\n", ctrl, ext, mt->tm[ext].fn);
      break;
    default:
      abort();
  }

  return 1;
}
