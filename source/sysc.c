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


#include "emu.h"

#include "io.h"

#include "sysc.h"

#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif
 

static void sc_init(cpu_t *cpu, int type, int ext, int func, int ctrl, sc_t **sc, int argc, char *argv[])
{
  (*sc) = calloc(1, sizeof(sc_t));
  (*sc)->id = io_id(0104, ctrl);
  (*sc)->ctrl = ctrl;

  pipe((*sc)->kb);
  pipe((*sc)->pr);
  int opt = 1;
  ioctl((*sc)->kb[0], FIONBIO, &opt);
  ioctl((*sc)->pr[0], FIONBIO, &opt);
  ioctl((*sc)->kb[0], FIOCLEX, NULL);
  ioctl((*sc)->pr[0], FIOCLEX, NULL);
  ioctl((*sc)->kb[1], FIOCLEX, NULL);
  ioctl((*sc)->pr[1], FIOCLEX, NULL);

  (*sc)->ri.i = -1;
  (*sc)->ti.i = -1;

  (*sc)->cpu = cpu;
  cpu->sc = (*sc);
//pthread_attr_init(&(*sc)->pthread.attr);
//pthread_attr_setdetachstate(&(*sc)->pthread.attr, PTHREAD_CREATE_DETACHED);
//char tname[16];
//snprintf(tname, sizeof(tname), "sysc %03o", (*sc)->ctrl);
//pthread_setname_np(cpu->sys->tid, tname);
//sigset_t set;
//sigemptyset(&set);
//sigaddset(&set, SIGINT);
//sigaddset(&set, SIGTSTP);
//pthread_sigmask(SIG_BLOCK, &set, NULL);
}


static uint16_t sysc_bclock[8] = { 0 };
#define sysc_bclock_size (sizeof(sysc_bclock)/sizeof(*sysc_bclock))
static unsigned int sysc_bclock_index = sysc_bclock_size;

static inline uint16_t sysc_bcd(int i)
{
  return (((i / 10) % 10) << 4) | (i % 10);
}

static inline void sysc_bclock_fetch(cpu_t *cpu)
{
  struct tm tm;
  const time_t t = time(NULL);
  localtime_r(&t, &tm);
  sysc_bclock[0] = sysc_bcd(tm.tm_year) + ((tm.tm_year > 99 && tm.tm_year < 128) ? 0240 : 0);
  sysc_bclock[1] = sysc_bcd(tm.tm_mon + 1);
  sysc_bclock[2] = sysc_bcd(tm.tm_mday);
  sysc_bclock[3] = sysc_bcd(tm.tm_wday + 1);
  sysc_bclock[4] = sysc_bcd(tm.tm_hour);
  sysc_bclock[5] = sysc_bcd(tm.tm_min);
  sysc_bclock[6] = sysc_bcd(tm.tm_sec > 59 ? 59 : tm.tm_sec);
  sysc_bclock[7] = 0;
  sysc_bclock_index = 0;
}

static inline void sysc_bclock_nofetch(cpu_t *cpu)
{
  sysc_bclock_index = sysc_bclock_size;
}

static inline int sysc_bclock_isfetch(cpu_t *cpu)
{
  return sysc_bclock_index < sysc_bclock_size;
}

static inline uint16_t sysc_bclock_read(cpu_t *cpu)
{
  return sysc_bclock[7 & sysc_bclock_index++];
}


static int sysc_poll(sc_t *sc)
{
  struct pollfd pfd = {.fd = sc->kb[0], .events = POLLIN};
  return poll(&pfd, 1, 0);
}


static int sysc_read(sc_t *sc)
{
  char c;
  return read(sc->kb[0], &c, 1) == 1 ? c : EOF;
}


static int sysc_write(sc_t *sc, char c)
{
  return write(sc->pr[1], &c, 1) == 1 ? c : EOF;
}


static int sysc_out(sc_t *sc)
{
  char c;
  return read(sc->pr[0], &c, 1) == 1 ? c : EOF;
}


static int sysc_in(sc_t *sc, char c)
{
  if(sc->im)
    io_setintv(sc->cpu, &sc->ri, sc->rv);
  return write(sc->kb[1], &c, 1) == 1 ? c : EOF;
}


void sysc_input(cpu_t *cpu, char *s)
{
sc_t *sc = cpu->sc;
char c = '\0';

  while(s && *s)
  {
    if(*s != '\\' && c != '\\')
      sysc_in(sc, *s);
    else
    {
      if(c != '\\')
        c = *s;
      else
        switch(*s)
        {
          case 'n':
            sysc_in(sc, '\n');
            break;
          case '\\':
            sysc_in(sc, '\\');
            break;
          default:
            sysc_in(sc, c);
            sysc_in(sc, *s);
        }
    }
    ++s;
  }
}


int sysc_term(cpu_t *cpu)
{
sc_t *sc = cpu->sc;

  struct termios orig, nonblock;
 
  tcgetattr(0,&orig);
  nonblock = orig;
  nonblock.c_lflag &= ~ICANON;
  nonblock.c_lflag &= ~ECHO;
  nonblock.c_lflag &= ~ISIG;
  nonblock.c_cc[VMIN] = 0;
  nonblock.c_cc[VTIME] = 0;
  nonblock.c_iflag &= ~(ICRNL | IGNCR);
  nonblock.c_iflag |= ISTRIP;
  tcsetattr(0, TCSANOW, &nonblock);

  fd_set fs;
  FD_ZERO(&fs);

  char c, esc = '\0';
  int count;

  do {

    FD_SET(STDIN_FILENO, &fs);
    FD_SET(sc->pr[0], &fs);

    struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
    if((count = select(sc->pr[0] + 1, &fs, NULL, NULL, &tv)) <= 0)
    {
      if(esc)
      {
        sysc_in(sc, esc);
        esc = '\0';
      }
    }
    else
    {
      if(FD_ISSET(STDIN_FILENO, &fs))
      {
        read(STDIN_FILENO, &c, 1);
        if(c == '\033')
        {
          if(esc)
            break;
          else 
            esc = c;
        }
        else
        {
          if(esc)
            sysc_in(sc, esc);
          esc = '\0';
          sysc_in(sc, c);
        }
      }
      if(FD_ISSET(sc->pr[0], &fs))
      {
        c = sysc_out(sc);
        write(STDOUT_FILENO, &c, 1);
      }
    }
  } while(cpu->halt.status == started || count > 0);

  while((c = sysc_out(sc) > 0))
    write(STDOUT_FILENO, &c, 1);

  tcsetattr(0, TCSANOW, &orig);
  return 0;
}


int sysc_io(cpu_t *cpu, int type, int ext, int func, int ctrl, void **devparm, int argc, char *argv[])
{
sc_t *sc = *devparm;

  switch(type) {
    case IO_TYPE_OCP:
      switch(func) {
//      case 000: // Initialize Echoplex Input
        case 001: // Initialize Echoplex Output
          logmsg("sysc %03o Initialize Echoplex Output\n", sc->ctrl);
          break;
//      case 002: // Set Receive Interrupt Mask
//      case 003: // Enable Receive DMA/C
//      case 004: // Reset Receive Interrupt Mask and Transmit DMA/C
//      case 005: // Set Transmit Interrupt Mask
//      case 006: // Enable Transmit DMA/C
//      case 007: // Reset Transmit Interrupt Mask and Transmit DMA/C
        case 010: // Enable full duplex I/O
          sc->echo = 0;
          logmsg("sysc %03o Enable full duplex I/O\n", sc->ctrl);
          break;
//      case 011: // Output Sync Pulse
        case 012: // Enable full duplex and echo
          sc->echo = 1;
          logmsg("sysc %03o Enable Echoplex I/O\n", sc->ctrl);
          break;
//      case 013: // Set Diagnostic Mode
//      case 014: // SLC only: Set Search for Sync Mode
        case 015: // Set Interrupt Mask
          logmsg("sysc %03o Set Interrupt Mask\n", sc->ctrl);
	  sc->im = 1;
	  if(sysc_poll(sc))
            io_setintv(cpu, &sc->ri, sc->rv);
          break;
        case 016: // Reset Interrupt Mask
          logmsg("sysc %03o Reset Interrupt Mask\n", sc->ctrl);
	  sc->im = 0;
          io_clrint(cpu, &sc->ri);
          io_clrint(cpu, &sc->ti);
          break;
        case 017: // Reset
          logmsg("sysc %03o Reset\n", sc->ctrl);
	  sc->im = 0;
          io_clrint(cpu, &sc->ri);
          io_clrint(cpu, &sc->ti);
          break;
        default:
          logall("sysc %03o unsupported OCP order %03o\n", sc->ctrl, func);
      }
      return 0;
      break;
    case IO_TYPE_SKS:
      switch(func) {
//      case 000: // Skip If Ready
        case 001: // Skip If Not Busy
          logmsg("sysc %03o Skip If Not Busy\n", sc->ctrl);
          break;
//      case 002: // Skip If Receiver not Interrupting
//      case 003: // Skip If Control Registers Valid
        case 004: // Skip If Neither Receiver nor Transmitter Interrupting
          logmsg("sysc %03o Skip If Neither Receiver nor Transmitter Interrupting\n", sc->ctrl);
	  if(sysc_poll(sc))
            return 0;
          break;
//      case 005: // Skip If Transmitter not Interrupting
        case 006: // Skip Transmit Ready
          logmsg("sysc %03o Skip Transmit Ready\n", sc->ctrl);
          break;
//      case 007: // Skip If Receive Ready
//      case 010: // SLC only: Skip it not in Search for Sync Mode
//      case 011: // Skip If Input bit 1 is Marking (open line)
//      case 012: // Skip If Input bit 2 is Marking
//      case 013: // Skip If Input bit 3 is Marking
//      case 014: // Skip If Input bit 4 is Marking
//      case 015: // Skip If Parity Error
//      case 016: // Skip If Overrun
//      case 017: // Skip If Framing Error
        default:
          logall("sysc %03o unsupported SKS order %03o\n", sc->ctrl, func);
      }
      break;
    case IO_TYPE_INA:
      switch(func) {
        case 000: // Input Character
        case 010: // Clear A and Input Character
          {
          int c = sysc_read(sc);
          if(c == EOF)
            return 0;
          if(sc->echo)
            sysc_write(sc, c);
          if(func)
            S_A(cpu, (c | 0x80));
          else
            S_A(cpu, G_A(cpu) | (c | 0x80));
          }
          logmsg("sysc %03o in '%c'\n", sc->ctrl, isprint(G_A(cpu) & 0x7f) ? G_A(cpu) & 0x7f : '.');
          break;
        case 004: // Input Receive Control Register 1
          S_A(cpu, sc->r1);
          logmsg("sysc %03o Input Receive Control Register 1 %4.4x\n", sc->ctrl, sc->r1);
          break;
        case 005: // Input Receive Control Register 2
          S_A(cpu, sc->r2);
          logmsg("sysc %03o Input Receive Control Register 2 %4.4x\n", sc->ctrl, sc->r2);
          break;
        case 006: // Input Transmit Control Register 1
          S_A(cpu, sc->t1);
          logmsg("sysc %03o Output Transmit Control Register 1 %4.4x\n", sc->ctrl, sc->t1);
          break;
        case 007: // Input Transmit Control Register 2
          S_A(cpu, sc->t2);
          logmsg("sysc %03o Input Transmit Control Register 2 %4.4x\n", sc->ctrl, sc->t2);
          break;
        case 011: // Input ID
          S_A(cpu, sc->id);
          break;
//      case 014: // Input Receive DMA/C channel address
//      case 015: // Input Transmit DMA/C channel address
//      case 016: // Input Receive Vector
        case 017: // Input Transmit Vector / Input BClock 
          if(sysc_bclock_isfetch(cpu))
          {
            uint16_t bc = sysc_bclock_read(cpu);
            logmsg("sysc %03o Input BClock %4.4x\n", sc->ctrl, bc);
            S_A(cpu, bc);
          }
          else
          {
            logmsg("sysc %03o Input Transmit Vector %4.4x\n", sc->ctrl, sc->tv);
            S_A(cpu, sc->tv);
          }
          break;
        default:
          logall("sysc %03o unsupported INA order %03o\n", sc->ctrl, func);
      }
      break;
    case IO_TYPE_OTA:
      switch(func) {
        case 000: // Output Character
          if(sysc_write(sc, G_A(cpu) & 0x7f) == EOF)
            return 0;
          logmsg("sysc %03o out '%c'\n", sc->ctrl, isprint(G_A(cpu) & 0x7f) ? G_A(cpu) & 0x7f : '.');
          break;
        case 004: // Output Receive Control Register 1
          sc->r1 = G_A(cpu);
          logmsg("sysc %03o Output Receive Control Register 1 %4.4x\n", sc->ctrl, sc->r1);
          break;
        case 005: // Output Receive Control Register 2
          sc->r2 = G_A(cpu);
          logmsg("sysc %03o Output Receive Control Register 2 %4.4x\n", sc->ctrl, sc->r2);
          break;
        case 006: // Output Transmit Control Register 1
          sc->t1 = G_A(cpu);
          logmsg("sysc %03o Output Transmit Control Register 1 %4.4x\n", sc->ctrl, sc->t1);
          break;
        case 007: // Output Transmit Control Register 2
          sc->t2 = G_A(cpu);
          logmsg("sysc %03o Output Transmit Control Register 2 %4.4x\n", sc->ctrl, sc->t2);
          break;
        case 013: // Set Clock
          logmsg("sysc %03o Set Clock %04x\n", sc->ctrl, G_A(cpu));
          break;
//      case 014: // Output Receive DMA/C Channel Address
//      case 015: // Output Transmit DMA/C Channel Address
        case 016: // Output Receive Interrupt Vector
          sc->rv = G_A(cpu);
          logmsg("sysc %03o Output Receive Interrupt Vector %4.4x\n", sc->ctrl, sc->t2);
          break;
        case 017: // Output Transmit Interrupt Vector
          if(G_A(cpu) == 0)
          {
            logmsg("sysc %03o Read BClock\n", sc->ctrl);
            sysc_bclock_fetch(cpu);
          }
          else
          {
            sysc_bclock_nofetch(cpu);
            sc->tv = G_A(cpu);
            logmsg("sysc %03o Output Transmit Vector %4.4x\n", sc->ctrl, sc->tv);
          }
          break;
        default:
          logall("sysc %03o unsupported OTA order %03o %4.4x\n", sc->ctrl, func, G_A(cpu));
      }
      break;
    case IO_TYPE_INI:
      sc_init(cpu, type, ext, func, ctrl, (sc_t **)devparm, argc, argv);
      break;
  }

  return 1;
}
