/* Emulator Thread
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

#include "cpu.h"

#include "cmd.h"

#include "io.h"

#define SIGHANDLER

#ifdef SIGHANDLER
static pthread_once_t once = PTHREAD_ONCE_INIT;

static pthread_key_t cput;

static void key_create(void)
{
  pthread_key_create(&cput, NULL);
}

void mach_chk(cpu_t *, uint32_t);
static void handler(int signo)
{
cpu_t *cpu = pthread_getspecific(cput);

  if(cpu)
    mach_chk(cpu, signo);
  else
    raise(signo);
}

static inline void setsig(cpu_t *cpu)
{
  signal(SIGSEGV, handler);
}
#endif

static void mode_error(cpu_t *cpu)
{
  longjmp(cpu->smode, smode_halt);
}


static void run_cpu(cpu_t *cpu)
{
//static const char *mode[8] = { "e16s", "e32s", "e64r", "e32r", "e32i", "err5", "e64v", "err7" };
static const run_cpu_t run[8] = {
  e16s_run_cpu,
  e32s_run_cpu,
  e64r_run_cpu,
  e32r_run_cpu,
  e32i_run_cpu,
  mode_error,
  e64v_run_cpu,
  mode_error };
//logall("-> mode %s\n", mode[cpu->crs->km.mode]);
  return run[cpu->crs->km.mode](cpu);
}


static void *cpu_thread(void *arg)
{
cpu_t *cpu = arg;

#ifdef SIGHANDLER
  pthread_once(&once, key_create);
  pthread_setspecific(cput, cpu);
#endif

  pthread_setname_np(pthread_self(), "cpu");
  
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTSTP);
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  smode_t mode = setjmp(cpu->smode);

  do {

    switch(mode) {
      case smode_halt:
        cpu->halt.status = stopped;
#ifdef DEBUG
        dump_physstor(cpu, 0, 0);
#endif
        break;

      case smode_terminate:
        cpu->pthread.tid = 0;
        return NULL;

      default:
        ;
    }

    run_cpu(cpu);

  } while (1);

}


int em50_init(cpu_t *cpu)
{

  cpu->sys->tid = pthread_self();
  const struct sched_param sparam = { .sched_priority = sched_get_priority_max(SCHED_FIFO) };
  cpu->sys->cap_sys_nice = pthread_setschedparam(pthread_self(), SCHED_FIFO, &sparam) ? false : true;

  logall("physstor = %p\nphyssize = %zu (%zu pages)\n", cpu->sys->physstor, cpu->sys->physsize, cpu->sys->physsize / em50_pgoc_size);

#if !defined(MODEL)
  cpu->model = *default_cpumodel();
#endif

  io_intr_init(cpu);
  cpu_halt_init(cpu);

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, SIG_IGN);

#ifdef SIGHANDLER
  setsig(cpu);
#endif

  cpu_reset(cpu);

  io_reset(cpu);

  pthread_attr_init(&cpu->pthread.attr);
  pthread_attr_setdetachstate(&cpu->pthread.attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setinheritsched(&cpu->pthread.attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&cpu->pthread.attr, SCHED_OTHER);
  const struct sched_param param = { .sched_priority = sched_get_priority_min(SCHED_OTHER) };
  pthread_attr_setschedparam(&cpu->pthread.attr, &param);

  pthread_create(&cpu->pthread.tid, &cpu->pthread.attr, cpu_thread, cpu);

  return 0;
}
