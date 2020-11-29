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


#include "emu.h"

#include "mode.h"

#include "opcode.h"

#include "io.h"

#include "cntl.h"
#include "sysc.h"
#include "disk.h"
#include "tape.h"
#include "amlc.h"
#include "pnc.h"

#include "prcex.h"


#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


#ifndef _io_c
#define _io_c

static const ioop_t device[0100] = {
/*000*/ NULL,                    /*      Not Assigned */
/*001*/ NULL,                    /* 3000 Paper tape reader */
/*002*/ NULL,                    /* 3000 Paper tape punch*/
/*003*/ NULL,                    /* 3100 URC 1 */
/*004*/ sysc_io,                 /* 3000 System Console */
/*005*/ NULL,                    /* 3100 URC 2 */
/*006*/ NULL,                    /* 7000 IPC */
/*007*/ pnc_io,                  /* 7040 Primenet Node Controller 1 */
/*010*/ NULL,                    /*      ICS2 1 */
/*011*/ NULL,                    /*      ICS2 2 */
/*012*/ tape_io,                 /* 4020 MT 8-11 WAS: 4300 Floppy Disk */
/*013*/ tape_io,                 /* 4020 MT 4-7 */
/*014*/ tape_io,                 /* 4020 MT 0-4 */
/*015*/ NULL,                    /* 5000 AMLC 5 */
/*016*/ NULL,                    /* 5000 AMLC 6 */
/*017*/ NULL,                    /* 5000 AMLC 7 */
/*020*/ cntl_io,                 /* 3000 SOC */
/*021*/ NULL,                    /* 4002 Disk B2 */
/*022*/ disk_io,                 /* 400X Disk Controller 3 */
/*023*/ disk_io,                 /* 400X Disk Controller 4 */
/*024*/ disk_io,                 /* 2076 Writable Control Store */
/*025*/ disk_io,                 /* 4000 Disk B1 */
/*026*/ disk_io,                 /* 400X Disk Conntroller 1 */
/*027*/ disk_io,                 /* 400X Disk Conntroller 2 */
/*030*/ NULL,                    /* 3007 Parallel IO Channel 1 */
/*031*/ NULL,                    /* 3025 Parallel IO Channel 2 */
/*032*/ NULL,                    /* 5000 AMLC 8 */
/*033*/ NULL,                    /* 300X Printer */
/*034*/ NULL,                    /* 300X Printer */
/*035*/ amlc_io,                 /* 5000 AMLC 4 */
/*036*/ NULL,                    /*      ICS1 1 */
/*037*/ NULL,                    /*      ICS1 2 */
/*040*/ NULL,                    /* 600X PRIMAD */
/*041*/ NULL,                    /* 6020 Digital Input 1 */
/*042*/ NULL,                    /* 6020 Digital Input 2 */
/*043*/ NULL,                    /* 6040 Digital Output 1 */
/*044*/ NULL,                    /* 6040 Digital Output 2 */
/*045*/ disk_io,                 /* 6060 Digital to Analog Output */
/*046*/ disk_io,                 /* 6080 Computer Prod Interface */
/*047*/ NULL,                    /* 7040 Primenet Node Controller 2 */
/*050*/ NULL,                    /* 5300 MDLC 1 */
/*051*/ NULL,                    /* 5300 MDLC 2 */
/*052*/ amlc_io,                 /* 5000 AMLC 3 */
/*053*/ amlc_io,                 /* 5000 AMLC 2 */
/*054*/ amlc_io,                 /* 5000 AMLC 1 */
/*055*/ NULL,                    /* 5400 Multiple Autocall */
/*056*/ NULL,                    /* 5200 SMLC */
/*057*/ NULL,                    /*      Not Assigned */
/*060*/ NULL,                    /* 7000 GPIO */
/*061*/ NULL,                    /* 7000 GPIO */
/*062*/ NULL,                    /* 7000 GPIO */
/*063*/ NULL,                    /* 7000 GPIO */
/*064*/ NULL,                    /* 7000 GPIO */
/*065*/ NULL,                    /* 7000 GPIO */
/*066*/ NULL,                    /* 7000 GPIO */
/*067*/ NULL,                    /* 7000 GPIO */
/*070*/ NULL,                    /*      Reserved */
/*071*/ NULL,                    /*      Reserved */
/*072*/ NULL,                    /*      Reserved */
/*073*/ NULL,                    /*      Reserved */
/*074*/ NULL,                    /*      Reserved */
/*075*/ NULL,                    /*      Reserved */
/*076*/ NULL,                    /*      Reserved */
/*077*/ NULL                     /*      IO Bus tester */
};
static void *devparm[sizeof(device)/sizeof(*device)] = { NULL };

static const int ndevices = sizeof(device)/sizeof(*device);

#define io_type(_e) (((_e) >> 14) & 0b11)
#define io_ext(_e)  (((_e) >> 10) & 0b1111)
#define io_func(_e) (((_e) >> 6) & 0b1111)
#define io_ctrl(_e) ((_e)  & 0b111111)

#ifdef DEBUG
static const char *io_type_c[4] = { "ocp", "sks", "ina", "ota" };
#endif

void io_reset(cpu_t *cpu)
{
  for(int ctrl = 0; ctrl < ndevices; ++ctrl)
    if(device[ctrl])
      device[ctrl](cpu, IO_TYPE_INI, 0, 0, ctrl, &devparm[ctrl], 0, NULL);
}


static inline int io_execcmd(cpu_t *cpu, int cmd, int ctrl, int unit, int argc, char *argv[])
{
  if(devparm[ctrl])
    return device[ctrl](cpu, cmd, unit, 0, ctrl, &devparm[ctrl], argc, argv);
  else
    return 0;
}


int io_assign(cpu_t *cpu, int ctrl, int unit, int argc, char *argv[])
{
  return io_execcmd(cpu, IO_TYPE_ASN, ctrl, unit, argc, argv);
}


int io_load(cpu_t *cpu, int ctrl, int unit)
{
  S_RB(cpu, RESET_PC);
  return io_execcmd(cpu, IO_TYPE_IPL, ctrl, unit, 0, NULL);
}

#endif


E50I(eio)
{
uint32_t ea = E50X(ea)(cpu, op);
int type = io_type(ea);
int ext  = io_ext(ea);
int func = io_func(ea);
int ctrl = io_ctrl(ea);

  logop2o(op, "eio", ea);

  E50X(rxm_check)(cpu);

  logmsg("-> %s ext %o func %o ctrl %o\n", 
    io_type_c[io_type(ea)], io_ext(ea), io_func(ea), io_ctrl(ea));

  if(devparm[ctrl])
  {
    cpu->crs->km.eq = device[ctrl](cpu, type, ext, func, ctrl, &devparm[ctrl], 0, NULL);
  }
  else
  {
    logall("ctrl %03o invalid\n", ctrl);
    cpu->crs->km.eq = 0;
  }

}


E50I(pio)
{
int type = op[0] >> 6;
int func = ((op[0] & 0b11) << 2) | (op[1] >> 6);
int ctrl = op[1] & 0b111111;
int skip = 0;

  logop1oo(op, io_type_c[type], func, ctrl);

  E50X(rxm_check)(cpu);

logmsg("pio %s 0%o 0%o\n", io_type_c[type], func, ctrl);

  if(devparm[ctrl])
  {
    skip = device[ctrl](cpu, type, 0, func, ctrl, &devparm[ctrl], 0, NULL);
  }
  else
  {
    logall("ctrl %03o invalid\n", ctrl);
    skip = 0;
  }

  if(type != IO_TYPE_OCP && skip)
    ++cpu->p;
}


E50I(irtc)
{
  logop1(op, "irtc");

  E50X(rxm_check)(cpu);

  S_RB(cpu, cpu->srf.mrf.pswpb);
  S_KEYS(cpu, cpu->srf.mrf.pswkeys.keys);
#if defined S_MODE || defined R_MODE
  S_VSC(cpu, G_KEYS(cpu));
#endif

  io_clrai(cpu);

  cpu->crs->km.ie = 1;

  if(cpu->crs->km.pxm)
    E50X(pxm_disp)(cpu);

  set_cpu_mode(cpu, cpu->crs->km.mode);
}


E50I(irtn)
{
  logop1(op, "irtn");

  E50X(rxm_check)(cpu);

  S_RB(cpu, cpu->srf.mrf.pswpb);
  S_KEYS(cpu, cpu->srf.mrf.pswkeys.keys);
#if defined S_MODE || defined R_MODE
  S_VSC(cpu, G_KEYS(cpu));
#endif

  cpu->crs->km.ie = 1;

  if(cpu->crs->km.pxm)
    E50X(pxm_disp)(cpu);

  set_cpu_mode(cpu, cpu->crs->km.mode);
}


E50I(inh)
{
  logop1(op, "inh");

  E50X(rxm_check)(cpu);

  cpu->crs->km.ie = 0;
}


E50I(inhm)
{
  logop1(op, "inhm");

  E50X(rxm_check)(cpu);

  cpu->crs->km.ie = 0;
}


E50I(inhp)
{
  logop1(op, "inhp");

  E50X(rxm_check)(cpu);

  cpu->crs->km.ie = 0;
}


E50I(enb)
{
  logop1(op, "enb");

  E50X(rxm_check)(cpu);

  cpu->crs->km.ie = 1;

  longjmp(cpu->endop, endop_nointr1);
}


E50I(enbm)
{
  logop1(op, "enbm");

  E50X(rxm_check)(cpu);

  cpu->crs->km.ie = 1;

  longjmp(cpu->endop, endop_nointr1);
}


E50I(enbp)
{
  logop1(op, "enbp");

  E50X(rxm_check)(cpu);

  cpu->crs->km.ie = 1;

  longjmp(cpu->endop, endop_nointr1);
}


E50I(esim)
{
  logop1(op, "esim");

  E50X(rxm_check)(cpu);

  cpu->crs->km.vim = 0;

  longjmp(cpu->endop, endop_nointr1);
}


E50I(evim)
{
  logop1(op, "evim");

  E50X(rxm_check)(cpu);

  cpu->crs->km.vim = 1;

  longjmp(cpu->endop, endop_nointr1);
}


E50I(cai)
{
  logop1(op, "cai");

  E50X(rxm_check)(cpu);

  if(cpu->crs->km.vim)
  {
    io_clrai(cpu);
    longjmp(cpu->endop, endop_nointr1);
  }
}


#ifdef HMDE
int32_t c2r(cpu_t *cpu, uint32_t vaddr)
{
  return i2r(cpu, vaddr);
}

void istore_w(cpu_t *cpu, uint32_t vaddr, uint16_t val)
{
  int32_t raddr = i2r(cpu, vaddr);

  if(raddr >= 0)
    store_w(physad(cpu, raddr), val);

  logmsg("istore_w %8.8x %8.8x %4.4x\n", vaddr, raddr, val);
}


void istore_d(cpu_t *cpu, uint32_t vaddr, uint32_t val)
{
  if(page_cross_d(vaddr))
  {
      istore_w(cpu, vaddr, val >> 16);
      istore_w(cpu, vaddr + 1, val);
      return;
  }

  int32_t raddr = i2r(cpu, vaddr);

  if(raddr >= 0)
    store_d(physad(cpu, raddr), val);

  logmsg("istore_d %8.8x %8.8x %8.8x\n", vaddr, raddr, val);
}


uint16_t ifetch_w(cpu_t *cpu, uint32_t vaddr)
{
  int32_t raddr = i2r(cpu, vaddr);

  logmsg("ifetch_w %8.8x %8.8x %4.4x\n", vaddr, raddr, fetch_w(physad(cpu, vaddr)));

  return (raddr >= 0) ? fetch_w(physad(cpu, raddr)) : 0;
}


uint32_t ifetch_d(cpu_t *cpu, uint32_t vaddr)
{
  if(page_cross_d(vaddr))
  {
      return (ifetch_w(cpu, vaddr) << 16) | ifetch_w(cpu, vaddr + 1);
  }

  int32_t raddr = i2r(cpu, vaddr);

  logmsg("ifetch_d %8.8x %8.8x %8.8x\n", vaddr, raddr, fetch_w(physad(cpu, vaddr)));

  return (raddr >= 0) ? fetch_d(physad(cpu, raddr)) : 0;
}

#endif


#ifndef EMDE
 #include __FILE__
#endif
