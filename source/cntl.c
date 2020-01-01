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


#include "emu.h"

#include "io.h"

#include "cntl.h"


#if 0
#undef logall
#define logall(...) PRINTF(__VA_ARGS__)
#endif


#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


#ifdef __APPLE__
static inline void clock_delta(struct timespec *r, const struct timespec *s, const struct timespec *e)
{
  if(s->tv_nsec > e->tv_nsec)
  {
    r->tv_sec = e->tv_sec - s->tv_sec - 1;
    r->tv_nsec = 1000000000ULL + e->tv_nsec - s->tv_nsec;
  }
  else
  {
    r->tv_sec = e->tv_sec - s->tv_sec;
    r->tv_nsec = e->tv_nsec - s->tv_nsec;
  }
}

#define TIMER_ABSTIME (999)
static inline int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *request, struct timespec *remain)
{
  if(flags == TIMER_ABSTIME)
  {
    struct timespec tn, tw;
    clock_gettime(CLOCK_MONOTONIC, &tn);
    clock_delta(&tw, &tn, request);
    return nanosleep(&tw, remain);
  }
  else
    return nanosleep(request, remain);
}
#endif

static inline void cp_clkinit(cp_t *vcp)
{
  clock_gettime(CLOCK_MONOTONIC, &vcp->pthread.ts);
}

static void *cp_thread(void *parm)
{
cpu_t *cpu = parm;
cp_t *vcp = cpu->vcp;

  char tname[16];
  snprintf(tname, sizeof(tname), "vcp %03o", vcp->ctrl);
  pthread_setname_np(pthread_self(), tname);

  cp_clkinit(vcp);

  do {

    uint64_t nsec = (0x10000 - vcp->ir) * 4 * 33 * 32; // 330 * 3.2

    if((vcp->cr & SOC_CR_PICR))
      nsec <<= 5; // 102.4 / 3.2

    vcp->pthread.ts.tv_nsec += nsec;
    vcp->pthread.ts.tv_sec  += vcp->pthread.ts.tv_nsec / 1000000000ULL;
    vcp->pthread.ts.tv_nsec %= 1000000000ULL;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &vcp->pthread.ts, NULL);

    if((vcp->cr & SOC_CR_ACT) && (vcp->cr & SOC_CR_EMI))
    {
      if(vcp->mi)
      {
        uint16_t t = ifetch_w(cpu, vcp->mi);

        if(++t == 0 && vcp->im)
          io_setintv(cpu, &vcp->intr, vcp->iv);
     
        istore_w(cpu, vcp->mi, t);
      }
    }
    else
      if((vcp->cr & SOC_CR_ACT) && vcp->im)
        io_setintv(cpu, &vcp->intr, vcp->iv);

  } while(vcp->pthread.tid);

  return NULL;
}

static void vcp_init(cpu_t *cpu, int type, int ext, int func, int ctrl, cp_t **vcp, int argc, char *argv[])
{
  (*vcp) = calloc(1, sizeof(cp_t));
  (*vcp)->id = io_id(0120, ctrl);
  (*vcp)->ctrl = ctrl;

  pthread_cond_init(&(*vcp)->pthread.cond, NULL);
  pthread_mutex_init(&(*vcp)->pthread.mutex, NULL);
  pthread_attr_init(&(*vcp)->pthread.attr);
  pthread_attr_setdetachstate(&(*vcp)->pthread.attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setinheritsched(&(*vcp)->pthread.attr, PTHREAD_EXPLICIT_SCHED);
  if(cpu->sys->cap_sys_nice)
  {
    pthread_attr_setschedpolicy(&(*vcp)->pthread.attr, SCHED_FIFO);
    const struct sched_param param = { .sched_priority = sched_get_priority_max(SCHED_FIFO) };
    pthread_attr_setschedparam(&(*vcp)->pthread.attr, &param);
  }

  (*vcp)->intr.i = -1;

  (*vcp)->cpu = cpu;
  cpu->vcp = (*vcp);

  pthread_create(&(*vcp)->pthread.tid, &(*vcp)->pthread.attr, cp_thread, cpu);
}

int cntl_io(cpu_t *cpu, int type, int ext, int func, int ctrl, void **devparm, int argc, char *argv[])
{
cp_t *vcp = *devparm;

  switch(type) {
    case IO_TYPE_INA:
      switch(func) {
        case 002: // Input PIC interval register
          logmsg("soc %03o Input PIC Interval Register %4.4x\n", ctrl, vcp->ir);
          S_A(cpu, vcp->ir);
          break;
        case 011: // Input ID
          S_A(cpu, vcp->id);
          break;
        case 012: // Input Memory Increment Cell Address
          logmsg("soc %03o Input Memory Cell Address %4.4x\n", ctrl, vcp->mi);
          S_A(cpu, vcp->mi);
          break;
        case 013: // Input interrupt vector
          logmsg("soc %03o Input interrupt vector %4.4x\n", ctrl, vcp->iv);
          S_A(cpu, vcp->iv);
          break;
        case 016: // Read Sense Switches
          S_A(cpu, cpu->sys->sswitches);
          break;
        case 017: // Read Data Switches
          S_A(cpu, cpu->sys->dswitches);
          break;
        default:
          logall("soc %03o unsupported INA order %03o\n", ctrl, func);
      }
      return 0;
      break;
    case IO_TYPE_OTA:
      switch(func) {
        case 002: // Set interval register
          vcp->ir = G_A(cpu);
          logmsg("soc %03o Set PIC Interval Register %4.4x\n", ctrl, vcp->ir);
          break;
        case 007: // Load control register
          vcp->cr = G_A(cpu);
          logmsg("soc %03o Load Control Register %4.4x\n", ctrl, vcp->cr);
          break;
        case 012: // Load Memory Increment Cell Address
          vcp->mi = G_A(cpu);
          logmsg("soc %03o Load Memory Cell Address %4.4x\n", ctrl, vcp->mi);
          break;
        case 013: // Load interrupt vector
          vcp->iv = G_A(cpu);
          vcp->im = 1;
          logmsg("soc %03o load interrupt vector %4.4x\n", ctrl, vcp->iv);
          break;
        case 017: // Load Lights
          vcp->li = G_A(cpu);
          break;
        default:
          logall("soc %03o unsupported OTA order %03o %4.4x\n", ctrl, func, G_A(cpu));
      }
      return 0;
      break;
    case IO_TYPE_OCP:
      switch(func) {
        case 000: // Start LFC and Enable Memory Increment
          logmsg("soc %03o Start LFC and Enable Memory Increment\n", ctrl);
          io_clrint(cpu, &vcp->intr);
          vcp->cr |= SOC_CR_EMI;
          break;
        case 001: // Ackknowledge PIC interrupt
          logmsg("soc %03o Acknowledge PIC interrupt\n", ctrl);
          io_clrint(cpu, &vcp->intr);
          break;
        case 002: // Stop LFC and Enable Memory Increment
          logmsg("soc %03o Stop LFC and Enable Memory Increment\n", ctrl);
          io_clrint(cpu, &vcp->intr);
          vcp->cr |= SOC_CR_EMI;
          break;
        case 004: // Select Line Freguancy for Memory Increment
          logmsg("soc %03o Select Line Freguancy for Memory Increment\n", ctrl);
          break;
        case 005: // Select External Clock for Memory Increment
          logmsg("soc %03o Select External Clock for Memory Increment\n", ctrl);
          break;
        case 006: // Trigger WDT
          logmsg("soc %03o Trigger WDT\n", ctrl);
          break;
        case 007: // Stop WDT
          logmsg("soc %03o Stop WDT\n", ctrl);
          break;
        case 015: // Set interrupt mask
          logmsg("soc %03o Set Interrupt Mask\n", ctrl);
          vcp->im = 1;
          break;
        case 016: // Reset interrupt mask
          logmsg("soc %03o Reset Interrupt Mask\n", ctrl);
          vcp->im = 0;
          break;
        case 017: // Stop WDT
          logmsg("soc %03o Reset\n", ctrl);
          vcp->im = 0;
          vcp->cr = 0;
          break;
        default:
          logall("soc %03o unsupported OCP order %03o\n", ctrl, func);
      }
      return 0;
      break;
    case IO_TYPE_SKS:
      switch(func) {
        case 000: // Skip if not interrupting
          return io_tstint(cpu, &vcp->intr) ? 0 : 1;
        default:
          logall("soc %03o unsupported SKS order %03o\n", ctrl, func);
          return 0;
      }
      break;
    case IO_TYPE_INI:
      vcp_init(cpu, type, ext, func, ctrl, (cp_t **)devparm, argc, argv);
      {
      char genname[PATH_MAX];

        if(!(cpu->sys->cpboot && *cpu->sys->cpboot))
        {
          snprintf(genname, sizeof(genname), "%s/cpboot.pma", cpu->sys->hdir);
          if(!isfilex(c_fname(genname)))
            cpu->sys->cpboot = strdup(genname);
        }
      }
      break;
    case IO_TYPE_ASN:
      if(argc > 0)
      {
        if(cpu->sys->cpboot)
        {
          free(cpu->sys->cpboot);
          cpu->sys->cpboot = NULL;
        }

        if(strcasecmp(argv[0], "none") && !isfile(argv[0]))
          cpu->sys->cpboot = strdup(argv[0]);
      }
      else
        if(cpu->sys->cpboot && *cpu->sys->cpboot)
          printf("ASSIGN %03o:%1o %s\n", ctrl, ext, cpu->sys->cpboot);
      break;
    default:
      logall("soc %03o Invalid Type %d\n", ctrl, type);
  }
  return 1;
}
