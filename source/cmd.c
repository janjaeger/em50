/* Control Panel Commands
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

#include "cmd.h"

#include "copyright.h"

#include "cpu.h"

#include "io.h"

#include "cntl.h"

#include "sysc.h"

#include "help.h"

static const char *prompt = "CP> ";
#ifndef __APPLE__
const
#endif
char *rl_readline_name = "em50";

#define HISTORY "%s/history"


static inline int cmd_issq(const char c)
{
    return c == '\'';
}


static inline int cmd_isdq(const char c)
{
    return c == '\"';
}


static inline int cmd_eval(char *cmd, char *argv[])
{
int argc = 0;

  size_t len = strlen(cmd);

  if(len > 0)
  {
    size_t n;

    for(n = 0; n < len; ++n)
      if(!isspace(cmd[n]))
          break;

#define bracket_no  0b000
#define bracket_cmd 0b001
#define bracket_sq  0b010
#define bracket_dq  0b100
    int bracketed = bracket_no;

    for(; n < len; ++n)
    {

      if(!bracketed && isspace(cmd[n]))
      {
        cmd[n] = '\0';
        continue;
      }

      if(!bracketed)
      {
        if(cmd_issq(cmd[n]) || cmd_isdq(cmd[n]))
          argv[argc++] = cmd + n + 1;
        else
          argv[argc++] = cmd + n;
        bracketed ^= bracket_cmd;
      }

      if(cmd_isdq(cmd[n]))
        bracketed ^= bracket_dq;

      if(cmd_issq(cmd[n]))
        bracketed ^= bracket_sq;

      if(bracketed == bracket_cmd && isspace(cmd[n]))
      {
        if(cmd_issq(cmd[n-1]) || cmd_isdq(cmd[n-1]))
          cmd[n-1] = '\0';
        else
          cmd[n] = '\0';
        bracketed = bracket_no;
      }
    }

    if(cmd_issq(cmd[n-1]) || cmd_isdq(cmd[n-1]))
      cmd[n-1] = '\0';
  }
  else
    argv[0] = cmd;

  argv[argc] = NULL;

  return argc;
}


static inline int cmd_i2r(cpu_t *cpu, uint32_t vaddr, uint32_t *raddr)
{
int32_t r = c2r(cpu, vaddr);

  if(r >= 0)
  {
    *raddr = r;
    return 0;
  }
  else
  {
    *raddr = vaddr;
    return -1;
  }
}


static inline int cmd_termx(cpu_t *cpu)
{
  sysc_term(cpu);
  putchar('\n');
  if(cpu->halt.status == stopped)
    printf("HALTED AT %o/%o: %o\n", cpu->b, cpu->p, cpu->vcp->li);
  return 0;
}


static inline int cmd_sw2dev(const int sw, int *r_ctrl, int *r_unit, int legacy)
{
static const int dk_tab[4] = { 026, 027, 022, 023 };
static const int mt_tab[2] = { 014, 013 };

int ctrl, unit = 0;

  if(legacy)
    switch((sw & 7))
    {
      case 4: // MHD
        ctrl = dk_tab[(sw >> 4) & 3];
        break;
      case 5: // TAPE
        ctrl = mt_tab[0];
        break;
      case 3: // FHD
      case 6: // FLOPPY
      default:
        ctrl = 0;
    }
  else
    switch((sw & 7))
    {
      case 4: // MHD
        if((sw & 0110) == 0110)
        {
          ctrl = dk_tab[(sw >> 4) & 3];
          unit = (sw >> 7) & 3;
        }
        else
          ctrl = dk_tab[0];
        break;

      case 5: // TAPE
        if((sw & 0150) == 0000)
        {
          ctrl = mt_tab[(sw >> 4) & 1];
          unit = (sw >> 7) & 3;
        }
        else
          ctrl = mt_tab[0];
        break;

      case 3: // FHD
      case 6: // FLOPPY
      default:
        ctrl = 0;
    }

  if(r_ctrl)
    *r_ctrl = ctrl;
  if(r_unit)
    *r_unit = unit;

  return ctrl ? 0 : -1;
}


static inline int cmd_mt2sw(const char *mt)
{
static const struct {
  char *name;
  int  devno;
} tab[] = {
  { "MT0", 0005 },
  { "MT1", 0205 },
  { "MT2", 0405 },
  { "MT3", 0605 },
  { "MT4", 0025 },
  { "MT5", 0225 },
  { "MT6", 0425 },
  { "MT7", 0625 },
};

  for(int n = 0; n < sizeof(tab)/sizeof(*tab); ++n)
    if(!strcasecmp(mt, tab[n].name))
      return tab[n].devno;

  return -1;
}


static inline int cmd_altrstor(int argc, char *argv[], cpu_t *cpu)
{
uint32_t addr = 0; uint16_t val;

  if(argc <= 2)
    return 0;

  int i; char c;
  if(sscanf(argv[2], "%i%c", &i, &c) != 1)
    return 0;
  else
    addr = i;

  for(int n = 3; n < argc; ++n)
  {
    if(sscanf(argv[n], "%i%c", &i, &c) != 1)
      return 0;
    else
      val = i;

    uint32_t raddr;
    if(tolower(*argv[1]) == 'v')
    {
      if(cmd_i2r(cpu, addr + n - 3, &raddr))
      {
        printf("V%8.8X Storage inaccessible\n", raddr);
        return 0;
      }
    }
    else
      raddr = addr + n - 3;

    if(raddr < (cpu->sys->physsize>>1))
      store_w(physad(cpu, raddr), val);
    else
    {
      printf("R%6.6X Storage not available\n", raddr);
      return 0;
    }
  }

  return 0;
}


static int cmd_alter(int argc, char *argv[], cpu_t *cpu)
{
  if(argc > 1)
  {
    switch(tolower(*argv[1])) {
      case 'v':
      case 'r':
          cmd_altrstor(argc, argv, cpu);
        break;
    }
  }
  return 0;
}


static inline int cmd_dispreg(int argc, char *argv[], cpu_t *cpu)
{
urs_t *reg = cpu->crs;

  printf("CRS   %9u\n", cpu->crn);
  printf("OWNER %o/%4.4x\n", reg->ownerh, reg->ownerl);
  printf("PPA   %4.4x:%4.4x\n", cpu->srf.mrf.ppa, cpu->srf.mrf.pcba);
  printf("PPB   %4.4x:%4.4x\n", cpu->srf.mrf.ppb, cpu->srf.mrf.pcbb);
  printf("TIMER %9.8x\n", reg->timer + em50_timer());
  printf("A/B   %4.4x/%4.4x\n", G_A(cpu), G_B(cpu));
  printf("X/Y   %4.4x/%4.4x\n", G_X(cpu), G_Y(cpu));
  for(int far = 0; far < 2; ++far)
  printf("FAR%d %8.8x/%6.6x:%4.4x\n", far, G_FAR(cpu, far), G_FLR(cpu, far), G_FBR(cpu, far));
  printf("PC    %o(%o)/%o\n", ea_seg(cpu->pb), ea_ring(cpu->pb), ea_word(cpu->pb));

  for(int r = 0; r < 16; r +=2)
  {
    int e, f; // endianess corrected r
    switch(r) {
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
        e = r ^ 1;
        f = (r + 1) ^ 1;
        break;
      default:
        e = r;
        f = r + 1;
    }
    printf("R%02d %8.8x   R%02d %8.8x\n", r, cpu->crs->r[e],
                                          r+1, cpu->crs->r[f]);
  }
  return 0;
}

static inline int cmd_dispstor(int argc, char *argv[], cpu_t *cpu)
{
uint32_t addr = 0; int len = 8;

  if(argc <= 2)
    return 0;

  int i; char c;
  if(sscanf(argv[2], "%i%c", &i, &c) != 1)
    return 0;
  else
    addr = i;

  if(argc > 3)
  {
    if(sscanf(argv[3], "%i%c", &i, &c) != 1)
      return 0;
    else
      len = i;
  }

  if(len > 128)
    len = 128;

  for(int n = 0; n < len;)
  {
    for(; n < len; ++n)
    {
      if(!(n & 7))
        printf("%s%c%8.8X", n ? "\n" : "", toupper(*argv[1]), addr + n);

      uint32_t raddr;
      if(tolower(*argv[1]) == 'v')
      {
        if(cmd_i2r(cpu, addr + n, &raddr))
        {
          printf(" Storage inaccessible\n");
          return 0;
        }
        if(!(n & 7))
          printf(" R%6.6X", raddr);
      }
      else
        raddr = addr + n;

      if(raddr < (cpu->sys->physsize>>1))
        printf(" %4.4X", fetch_w(physad(cpu, raddr)));
      else
      {
        printf(" Storage not available\n");
        return 0;
      }
    }
    printf("\n");
  }
  return 0;
}


static int cmd_lights(int argc, char *argv[], cpu_t *cpu)
{
  printf("LIGHTS  %06o\n", cpu->vcp->li);
  return 0;
}


static int cmd_lightsc(int argc, char *argv[], cpu_t *cpu)
{
  int i = 0; char c;
  if(argc > 1 && sscanf(argv[1], "%i%c", &i, &c) != 1)
    i = 1;
  if(i == 0)
    i = 1;

  while (1)
  {
    printf("LIGHTS  %06o", cpu->vcp->li); fflush(stdout);
    usleep(1000000 / i); putchar('\r');
  }
  return 0;
}


static inline int cmd_dispintr(int argc, char *argv[], cpu_t *cpu)
{
  int p = io_pending(cpu);
  int a = cpu->intr.a;
  int n = a;
  printf("INT Q %d/%d\n", a, cpu->intr.n);
  if(cpu->intr.c >= 0)
    printf("INT V %4.4X ACTIVE\n", cpu->intr.c);
  if(p)
    do {
      if(cpu->intr.v[n] >= 0)
        printf("INT Q[%d] V %4.4X\n", n, cpu->intr.v[n]);
      n = io_incrnxt(n);
    } while (a != n);

  return 0;
}


static int cmd_display(int argc, char *argv[], cpu_t *cpu)
{
  if(argc > 1)
  {
    switch(tolower(*argv[1])) {
      case 'v':
      case 'r':
          cmd_dispstor(argc, argv, cpu);
        break;
      case 'i':
          cmd_dispintr(argc, argv, cpu);
        break;
      case 'c':
          cmd_dispreg(argc, argv, cpu);
        break;
      case 'l':
          cmd_lights(argc, argv, cpu);
        break;
    }
  }
  return 0;
}


static int cmd_assign(int argc, char *argv[], cpu_t *cpu)
{
int argd = 0;

  if(argc > 2 && tolower(*argv[1]) == 'd')
  {
    argd = 1;
    argc--;
    argv++;
  }

  if(argc > 1)
  {
  int sw, ctrl, unit; char c;
    if(sscanf(argv[1], "%o%c%o%c", &ctrl, &c, &unit, &c) != 3)
    if((sscanf(argv[1], "%o%c", &sw, &c) != 1
      && (sw = cmd_mt2sw(argv[1])) < 0)
      || argd || cmd_sw2dev(sw, &ctrl, &unit, 0))
      {
        if(sw > 0 && sw < 0100)
        {
          ctrl = sw;
          unit = 0;
        }
        else
        {
          printf("Invalid device (%s)\n", argv[1]);
          return 0;
        }
      }

    io_assign(cpu, ctrl, unit, argc - 2, argv + 2);
  }

  return 0;
}


static int cmd_sswitch(int argc, char *argv[], cpu_t *cpu)
{
  if(argc > 1)
  {
  int switches; char c;
    if(sscanf(argv[1], "%o%c", &switches, &c) != 1)
      printf("Invalid value (%s)\n", argv[1]);
    else
      cpu->sys->sswitches = switches;
  }
  else
    printf("SENSESW %06o\n", cpu->sys->sswitches);

  return 0;
}


static int cmd_sdatasw(int argc, char *argv[], cpu_t *cpu)
{
  if(argc > 1)
  {
  int switches; char c;
    if(sscanf(argv[1], "%o%c", &switches, &c) != 1)
      printf("Invalid value (%s)\n", argv[1]);
    else
      cpu->sys->dswitches = switches;
  }
  else
    printf("DATASW  %06o\n", cpu->sys->dswitches);

  return 0;
}


static int cmd_load(int argc, char *argv[], cpu_t *cpu)
{
  for(int n = 1; n < argc; ++n)
    if(!cpload(cpu, argv[n]))
      printf("Load of %s failed: %s\n", argv[1], strerror(errno));

  return 0;
}


static int cmd_boot(int argc, char *argv[], cpu_t *cpu)
{
int i; char c; int legacy = 0;

  if(argc > 1 && (isupper(*argv[1]) ? tolower(*argv[1]) : *argv[1]) == 'l')
  {
    ++argv;
    --argc;
    legacy = 1;
  }

  if(argc > 1 && sscanf(argv[1], "%o%c", &i, &c) == 1)
    cpu->vcp->iv = cpu->sys->sswitches = i;
  if(argc > 2 && sscanf(argv[2], "%o%c", &i, &c) == 1)
    cpu->sys->dswitches = i;
  if(argc > 3 && sscanf(argv[3], "%o%c", &i, &c) == 1)
    S_A(cpu, i);
  if(argc > 4 && sscanf(argv[4], "%o%c", &i, &c) == 1)
    S_B(cpu, i);
  if(argc > 5 && sscanf(argv[5], "%o%c", &i, &c) == 1)
    S_X(cpu, i);
  if(argc > 6 && sscanf(argv[6], "%o%c", &i, &c) == 1)
    S_KEYS(cpu, i);

  if(cpboot(cpu))
  {
    cpu_start(cpu);

    if(cpu->halt.status == started && cpu->sys->tmode == st)
      cmd_termx(cpu);
  }
  else
  {
  int ctrl, unit;

    if(!cmd_sw2dev(cpu->sys->sswitches, &ctrl, &unit, legacy)
      && io_load(cpu, ctrl, unit))
    {
      cpu_start(cpu);

      if(cpu->halt.status == started && cpu->sys->tmode == st)
        cmd_termx(cpu);
    }
    else
      printf("%s DEVICE %o INVALID\n", argv[0], cpu->sys->sswitches);
  }

  return 0;
}


static int cmd_sysclr(int argc, char *argv[], cpu_t *cpu)
{
  if(cpu->halt.status != stopped)
  {
    cpu_stop(cpu);
    usleep(10000ULL);
  }

  cpu_reset(cpu);

  return 0;
}


static int cmd_comment(int argc, char *argv[], cpu_t *cpu)
{
  return 0;
}


static int cmd_license(int argc, char *argv[], cpu_t *cpu)
{
  puts(license);
  return 0;
}


static int cmd_version(int argc, char *argv[], cpu_t *cpu)
{
  puts(package);
  puts(copyright);
  puts("EM50 version " VERSION " " __DATE__ " " __TIME__ " (" LICENSE ")");
  if(argc > 1)
    puts(license);
  return 0;
}


#ifdef DEBUG
static int cmd_trace(int argc, char *argv[], cpu_t *cpu)
{
  if(argc > 1)
  {
    if(!strcasecmp(argv[1], "on"))
      cpu->sys->verbose = 1;
    else
    if(!strcasecmp(argv[1], "off"))
      cpu->sys->verbose = 0;
    else
    if(strcasecmp(argv[1], "-"))
      printf("Invalid option (%s)\n", argv[1]);

    if(argc > 2)
    {
      if(!strcasecmp(argv[2], "close"))
      {
        if(cpu->sys->trace) 
        {
          fclose(cpu->sys->trace);
          cpu->sys->trace = NULL;
        }
      }
      else
      {
        char *mode = "w";
        if(argc > 3 && tolower(*argv[3]) == 'a')
          mode = "a";
        cpu->sys->trace = fopen(argv[2], mode);
        if(!cpu->sys->trace)
          printf("Open error: %s\n", strerror(errno));
      }
    }
  }
  else
    printf("%sctive, verbose o%s\n", cpu->sys->trace ? "A" : "Ina", cpu->sys->verbose ? "n" : "ff");
  return 0;
}


static int cmd_dump(int argc, char *argv[], cpu_t *cpu)
{
  dump_physstor(cpu, 0, 0);
  return 0;
}


static inline void test_lock(pthread_mutex_t *mutex, char *name)
{
  int rc = pthread_mutex_trylock(mutex);
  if(!rc)
    pthread_mutex_unlock(mutex);
  printf("Lock %-8s %sheld\n", name, rc ? "" : "not ");
}


static int cmd_locks(int argc, char *argv[], cpu_t *cpu)
{
  test_lock(&cpu->halt.mutex, "HALT");
  test_lock(&cpu->halt.mutex, "WAIT");
  return 0;
}
#endif


static int cmd_input(int argc, char *argv[], cpu_t *cpu)
{
  for(int n = 1; n < argc; ++n)
    sysc_input(cpu, argv[n]);
  return 0;
}


static int cmd_sleep(int argc, char *argv[], cpu_t *cpu)
{
  if(argc < 2)
    return 0;

  int i; char c;
  if(sscanf(argv[1], "%i%c", &i, &c) == 1)
    sleep(i);

  return 0;
}


static int cmd_terminal(int argc, char *argv[], cpu_t *cpu)
{
  if(argc > 1)
    switch(*argv[1]) {
      case 'c':
      case 'C':
        cpu->sys->tmode = cp;
        break;
      case 's':
      case 'S':
        cpu->sys->tmode = st;
        break;
      default:
        printf("Terminal mode is %s\n", cpu->sys->tmode == cp ? "CP" : "ST");
  }
  else
    cmd_termx(cpu);
  return 0;
}


static int cmd_run(int argc, char *argv[], cpu_t *cpu)
{
  if(argc > 1)
  {
    uint32_t i; char c;
    if(sscanf(argv[1], "%o%c", &i, &c) == 1)
      cpu->pb = i;
  }

  cpu_start(cpu);

  if(cpu->halt.status == started && cpu->sys->tmode == st)
    cmd_termx(cpu);

  return 0;
}


static int cmd_step(int argc, char *argv[], cpu_t *cpu)
{
  cpu_step(cpu);

  return 0;
}


static int cmd_stop(int argc, char *argv[], cpu_t *cpu)
{
  cpu_stop(cpu);

  return 0;
}


static int cmd_shell(int argc, char *argv[], cpu_t *cpu)
{
  char *sh = getenv("SHELL");
  if(!sh || !*sh)
    sh = "/bin/sh";

  argv[0] = sh;

  posix_spawnattr_t attr;
  posix_spawnattr_init(&attr);
  posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER);
  posix_spawnattr_setschedpolicy(&attr, SCHED_OTHER);
  const struct sched_param param = { .sched_priority = 0 };
  posix_spawnattr_setschedparam(&attr, &param);

  pid_t gpid;
  posix_spawn(&gpid, sh, NULL, &attr, argv, environ);

  posix_spawnattr_destroy(&attr);

  int rc, rs;
  while ((rc = waitpid(gpid, &rs, 0)) == -1 && (errno == EINTR));

  return 0;
}


static int cmd_serial(int argc, char *argv[], cpu_t *cpu)
{
char aserial[15];

  switch(argc) {
    case 1:
      printf("SERIAL %16.16s\n", cpu->sys->serial);
      break;
    case 2:
      switch(strlen(argv[1])) {
        case 12:
          memcpy(cpu->sys->serial + 2, argv[1], 12);
          memset(cpu->sys->serial + 14, ' ', 2);
          break;
        case 14:
          memcpy(cpu->sys->serial, argv[1], 14);
          memset(cpu->sys->serial + 14, ' ', 2);
          break;
        case 16:
          memcpy(cpu->sys->serial, argv[1], 16);
          break;
        default:
          snprintf(aserial, sizeof(aserial), "%12.12s  ", argv[1]);
          memcpy(cpu->sys->serial + 2, aserial, 14);
      }
      break;
    case 3:
      if(strlen(argv[1]) != 2)
      {
        printf("Invalid plant code (%s)\n", argv[1]);
        return 1;
      }
      else
      {
        memcpy(cpu->sys->serial, argv[1], 2);
      }
      snprintf(aserial, sizeof(aserial), "%12.12s  ", argv[2]);
      memcpy(cpu->sys->serial + 2, aserial, 14);
      break;
    default: 
      printf("Invalid parameters\n");
      return 1;
  }

  return 0;
}


#if !defined(MODEL)
static int cmd_model(int argc, char *argv[], cpu_t *cpu)
{
char c;

  if(argc == 1)
  {
    printf("MODEL %s (%hu) %hu %hu %hu %hu\n",
      cpu->model.name, cpu->model.number, 
      cpu->model.ucodeman, cpu->model.ucodeeng, 
      cpu->model.ucodepln, cpu->model.ucodeext);
    return 0;
  }
  if(argc > 1)
  {
    if(!strcasecmp(argv[1], "list"))
    {
    int n = 0;
    cpumodel_t *m;

      do {
        m = list_cpumodel(n++);
        if(!m)
          break;
        printf("%7.7s", m->name);
        if(!(n & 7))
          putchar('\n');
      } while (m);
      if(((n-1)& 7))
        putchar('\n');

      return 0;
    }

    if(cpu->halt.status != stopped)
    {
      printf("CPU must not be running\n");
      return 1;
    }

    cpumodel_t *newmodel = get_cpumodel(argv[1]);
    if(!newmodel)
    {
      printf("Invalid CPU model (%s)\n", argv[1]);
      return 1;
    }
    cpu->model = *newmodel;
  }
  if(argc > 2)
  {
    if(sscanf(argv[2],"%hu%c",&cpu->model.ucodeman, &c) != 1)
    {
      printf("Invalid ucode manufacturing level (%s)\n", argv[2]);
      return 1;
    }
  }
  if(argc > 3)
  {
    if(sscanf(argv[3],"%hu%c",&cpu->model.ucodeeng, &c) != 1)
    {
      printf("Invalid ucode engineering level (%s)\n", argv[3]);
      return 1;
    }
  }
  if(argc > 4)
  {
    if(sscanf(argv[4],"%hu%c",&cpu->model.ucodepln, &c) != 1)
    {
      printf("Invalid processor line (%s)\n", argv[4]);
      return 1;
    }
  }
  if(argc > 5)
  {
    if(sscanf(argv[5],"%hu%c",&cpu->model.ucodeext, &c) != 1)
    {
      printf("Invalid ucode extension (%s)\n", argv[5]);
      return 1;
    }
  }

  return 0;
}
#endif


static int cmd_quit(int argc, char *argv[], cpu_t *cpu)
{ return -1; }


static int cmd_enter(int argc, char *argv[], cpu_t *cpu)
{
  if(argc < 0)
    putchar('\n');
  if(cpu->halt.status == started && cpu->sys->tmode == st)
    cmd_termx(cpu);
  return 0;
}


static int cmd_error(int argc, char *argv[], cpu_t *cpu)
{
  printf("Invalid command (%s)\n", argv[0]);

  return 1;
}


static int cmd_help(int, char *[], cpu_t *cpu);
static const struct { 
  char *cmd;
  const int minlen;
  const enum { okrc, norc } valid;
  int (*func)(int, char *[], cpu_t *);
  const help_t *help;
} cmdtab[] = { 
  { "ASSIGN",   2, okrc, cmd_assign,   &help_assign },
  { "BOOT",     1, okrc, cmd_boot,     &help_boot },
  { "LOAD",     4, okrc, cmd_load,     &help_load },
  { "TERMINAL", 4, okrc, cmd_terminal, &help_terminal },
  { "SYSCLR",   4, okrc, cmd_sysclr,   &help_sysclr },
  { "STORE",    2, okrc, cmd_alter,    &help_alter },
  { "RUN",      1, okrc, cmd_run,      &help_run },
  { "STOP",     4, okrc, cmd_stop,     &help_stop },
  { "HALT",     4, okrc, cmd_stop,     &help_nohelp },
  { "STEP",     4, okrc, cmd_step,     &help_step },
#ifdef DEBUG
  { "LOCKS",    5, okrc, cmd_locks,    &help_locks },
  { "TRACE",    2, okrc, cmd_trace,    &help_trace },
  { "DUMP",     4, norc, cmd_dump,     &help_dump },
#endif
  { "SERIAL",   3, okrc, cmd_serial,   &help_serial },
#if !defined(MODEL)
  { "MODEL",    3, okrc, cmd_model,    &help_model },
#endif
  { "SSWITCH",  2, okrc, cmd_sswitch,  &help_sswitch },
  { "SDATASW",  2, okrc, cmd_sdatasw,  &help_sdatasw },
  { "LIGHTS",   2, okrc, cmd_lights,   &help_lights },
  { "LIGHTSC",  7, norc, cmd_lightsc,  &help_nohelp },
  { "DISPLAY",  1, okrc, cmd_display,  &help_display },
  { "ALTER",    2, okrc, cmd_alter,    &help_alter },
  { "VERSION",  3, okrc, cmd_version,  &help_version },
  { "LICENSE",  3, okrc, cmd_license,  &help_license },
  { "INPUT",    5, okrc, cmd_input,    &help_input },
  { "SHELL",    2, norc, cmd_shell,    &help_shell },
  { "SLEEP",    5, okrc, cmd_sleep,    &help_sleep },
  { "*",        1, okrc, cmd_comment,  &help_nohelp },
  { "QUIT",     4, norc, cmd_quit,     &help_quit },
  { "HELP",     4, norc, cmd_help,     &help_help }
};
static const int cmdtablen = sizeof(cmdtab)/sizeof(*cmdtab);


static int cmd_help(int argc, char *argv[], cpu_t *cpu)
{
  if(argc > 1)
  {
    int hf = 0;
    for(int o = 1; o < argc; ++o)
      for(int n = 0; n < cmdtablen; ++n)
      {
        if(strlen(argv[o]) >= cmdtab[n].minlen && !strncasecmp(argv[o], cmdtab[n].cmd, strlen(argv[o])) && cmdtab[n].help->h_short)
        {
          if(cmdtab[n].help->h_long)
            printf("Command:  %s  (%.*s)\n\n%s\n\n%s\n", cmdtab[n].cmd, cmdtab[n].minlen, cmdtab[n].cmd, cmdtab[n].help->h_short, cmdtab[n].help->h_long);
          else
            printf("Command:  %s  (%.*s)\n\n%s\n", cmdtab[n].cmd, cmdtab[n].minlen, cmdtab[n].cmd, cmdtab[n].help->h_short);
          hf = 1;
        }
      }
    if(!hf)
      printf("No help available for \"%s\"\n", argv[1]);
  }
  else
  {
    for(int n = 0; n < cmdtablen; ++n)
      if(cmdtab[n].help->h_short)
        printf("%-8s %s\n", cmdtab[n].cmd, cmdtab[n].help->h_short);
  }

  return 0;
}


static inline int cmd_exec(char *cmdline, cpu_t *cpu)
{
  if(!cmdline)
    return cmd_enter(-1, NULL, cpu);

  char *argv[2+strlen(cmdline)/2];
  int argc = cmd_eval(cmdline, argv);

  if(argc == 0)
    return cmd_enter(argc, argv, cpu);

  int cmdlen = strlen(argv[0]);

  for(int n = 0; n < cmdtablen; ++n)
    if(cmdlen >= cmdtab[n].minlen && !strncasecmp(argv[0], cmdtab[n].cmd, cmdlen) && !(cmdtab[n].valid == norc && cpu->sys->tmode == rc))
    {
      argv[0] = cmdtab[n].cmd;
      return cmdtab[n].func(argc, argv, cpu);
    }

  return cmd_error(argc, argv, cpu);
}


static sigjmp_buf cmd_control_c_jmpbuf;
static void cmd_control_c(int signo, siginfo_t *siginfo, void *context)
{
  siglongjmp(cmd_control_c_jmpbuf, 1);
}

int cmd_main(cpu_t *cpu)
{
  struct sigaction csav, cnew = {.sa_sigaction = cmd_control_c };
  struct sigaction zsav, znew = {.sa_handler = SIG_IGN };

#if 0
  char *line = NULL;
  size_t cap = 0;
  ssize_t len;
  int rc;

  if(sigsetjmp(cmd_control_c_jmpbuf, 1))
    putchar('\n');
  else
  {
    sigaction(SIGINT, &cnew, &csav);
    sigaction(SIGTSTP, &znew, &zsav);
  }

  do {
    fputs(prompt, stdout);
    len = getline(&line, &cap, stdin);
    if(len > 0)
      line[--len] = '\0';
    rc = cmd_exec(line, cpu);
  } while(rc >= 0);
#else
  char history[PATH_MAX];
  snprintf(history, sizeof(history), HISTORY, cpu->sys->hdir);
  read_history(history);
  char *cmdline;
  int rc;

  if(sigsetjmp(cmd_control_c_jmpbuf, 1))
    putchar('\n');
  else
  {
    sigaction(SIGINT, &cnew, &csav);
    sigaction(SIGTSTP, &znew, &zsav);
  }

  do {
    cmdline = readline(prompt);
    if(cmdline && *cmdline)
      add_history(cmdline);
    rc = cmd_exec(cmdline, cpu);
    if(cmdline && *cmdline && rc == 0)
      write_history(history);
  } while(rc >= 0);

#endif

  sigaction(SIGINT, &csav, NULL);
  sigaction(SIGTSTP, &zsav, NULL);

  return rc;
}


int cmd_mainrc(cpu_t *cpu, const char *rcfile)
{
FILE *fp;

  if(!(fp = fopen(rcfile, "r")))
  {
    if(errno == ENOENT)
      cmd_version(0, NULL, cpu); // Do not display version if a .em50/rc file exists
    else
      fprintf(stderr, "fopen(%s) failed rc=%d: %s\n", rcfile, errno, strerror(errno));
  }
  else
  {
    cpu->sys->tmode = rc;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp))
    {
    int ln = strlen(buffer);
    if(ln > 1 && buffer[ln - 1] == '\n')
      buffer[ln - 1] = '\0';
    cmd_exec(buffer, cpu);
    }
    if(cpu->sys->tmode == rc)
      cpu->sys->tmode = st;
    fclose(fp);
  }

  if(cpu->halt.status == started && cpu->sys->tmode == st)
    cmd_termx(cpu);

  return 0;
}
