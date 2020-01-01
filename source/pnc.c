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


#include "emu.h"

#include "io.h"

#include "pnc.h"

#if 0
#undef logall
#define logall(...) PRINTF(__VA_ARGS__)
#endif

#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


#define PNC_RXRDY 'R'
#define PNC_TXRDY 'T'
#define PNC_XCONN 'C'
#define PNC_XDISC 'D'
#define PNC_TOKEN 'K'
#define PNC_SIMTK 'S'
#define PNC_INITP 'I'

static const uint8_t pnc_rxrdy = PNC_RXRDY;
static const uint8_t pnc_txrdy = PNC_TXRDY;
static const uint8_t pnc_xconn = PNC_XCONN;
static const uint8_t pnc_xdisc = PNC_XDISC;
static const uint8_t pnc_token = PNC_TOKEN;
static const uint8_t pnc_simtk = PNC_SIMTK;
static const uint8_t pnc_initp = PNC_INITP;


static int pnc_port = 0;

static inline size_t pnc_dmx_copy(cpu_t *cpu, int ca, uint8_t *buffer, ssize_t len, int wr)
{
  if((ca & PNC_DMC))
    return io_dmc_copy(cpu, ca & PNC_DMC_MASK, buffer, len, wr);
  else
    return io_dma_copy(cpu, ca & PNC_DMA_MASK, 0, buffer, len, wr);
}


static inline int pnc_inet_port(pnc_t *pnc, const char *serv)
{
#ifdef DEBUG
cpu_t *cpu = pnc->cpu;
#endif
struct servent *servent;

  if(isdigit(*serv))
    return atoi(serv);
  else
    if((servent = getservbyname(serv, "tcp")))
      return htons(servent->s_port);

  logmsg("pnc %03o getservbyname(%s) failed", pnc->ctrl, serv);
  return 0;
}


static inline in_addr_t pnc_inet_host(pnc_t *pnc, const char *host)
{
#ifdef DEBUG
cpu_t *cpu = pnc->cpu;
#endif
struct hostent *hostent;
struct in_addr in_addr;

  if(isdigit(*host))
    return inet_addr(host);
  else
    if((hostent = gethostbyname(host)))
    {
      memcpy(&in_addr, *hostent->h_addr_list, sizeof(in_addr));
      return in_addr.s_addr;
    }

  logmsg("pnc %03o gethostbyname(%s) failed", pnc->ctrl, host);
  return INADDR_NONE;
}


static inline ssize_t pnc_recv(int fd, buff_t *buff)
{
  if(buff->len < sizeof(buff->data.len))
  {
    ssize_t l = recv(fd, buff->raw + buff->len, sizeof(buff->data.len) - buff->len, 0);
    if(l <= 0)
      return -1;

    buff->len += l;
  }
  else if(buff->len < ntohs(buff->data.len))
  {
    if(ntohs(buff->data.len) > sizeof(buff->data))
      return -1;

    ssize_t l = recv(fd, buff->raw + buff->len, ntohs(buff->data.len) - buff->len, 0);
    if(l <= 0)
      return -1;

    buff->len += l;
  }

  if(buff->len < sizeof(buff->data.len) || buff->len < ntohs(buff->data.len))
    return 0;

  buff->len = 0;
  return ntohs(buff->data.len) - sizeof(buff->data.len);
}


static inline ssize_t pnc_send(int fd, buff_t *buff, size_t len)
{
  size_t n = len + sizeof(buff->data.len);
  buff->data.len = htons(n);

  ssize_t l = send(fd, buff->raw, n, 0);

  return l == n ? l : -1;
}


static inline link_t *pnc_connect(pnc_t *pnc, int fd)
{
#ifdef DEBUG
cpu_t *cpu = pnc->cpu;
#endif
link_t *l;

  for(l = pnc->link; l; l = l->next)
    if(l->fd == -1)
      break;

  if(!l)
  {
    l = calloc(1, sizeof(link_t));
    l->next = pnc->link;
    pnc->link = l;
  }

  l->fd = fd;
  l->nn = 255;
  l->rb.len = 0;

  ioctl(fd, FIOCLEX, NULL);

  logmsg("pnc %03o line %d connected\n", pnc->ctrl, fd);

  return l;
}


static inline void pnc_disconnect(pnc_t *pnc, link_t *link)
{
#ifdef DEBUG
cpu_t *cpu = pnc->cpu;
#endif

  if(link->fd < 0)
    return;

  logmsg("pnc %03o line %d disconnected\n", pnc->ctrl, link->fd);

  close(link->fd);
  link->fd = -1;
}


static void *pnc_thread(void *parm)
{
pnc_t *pnc = parm;
cpu_t *cpu = pnc->cpu;

  char tname[16];
  snprintf(tname, sizeof(tname), "pnc %03o", pnc->ctrl);
  pthread_setname_np(pthread_self(), tname);

  int sock;
  do {
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
      logmsg("pnc %03o Failed to obtain socket errno=%d: %s\n", pnc->ctrl, errno, strerror(errno));
      break;
    }

    ioctl(sock, FIOCLEX, NULL);

    const int optval = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
      logmsg("pnc %03o setsockopt(SO_REUSEADDR) failed errno=%d: %s\n", pnc->ctrl, errno, strerror(errno));

    struct sockaddr_in bindsock = {.sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY };

    const char *iface = cpu->sys->pncbind;
    const char *port  = cpu->sys->pncport;

    if(iface)
    {
      if(INADDR_NONE == (bindsock.sin_addr.s_addr = pnc_inet_host(pnc, iface)))
      {
        logmsg("pnc %03o No such interface: %s\n", pnc->ctrl, iface);
        close(sock); sock = -1;
        break;
      }
    }

    if(pnc_port)
      bindsock.sin_port = htons(++pnc_port);
    else
      if(port)
        bindsock.sin_port = htons(pnc_inet_port(pnc, port));
      else
        bindsock.sin_port = htons(PNC_DFLTPORT);

    if(bind(sock, (struct sockaddr *)&bindsock, sizeof(bindsock)))
    {
      logmsg("pnc %03o Failed to bind to socket errno=%d: %s\n", pnc->ctrl, errno, strerror(errno));
      close(sock); sock = -1;
      break;
    }

    if(!pnc_port)
      pnc_port = ntohs(bindsock.sin_port);

    if(listen(sock, PNC_BACKLOG))
    {
      logmsg("pnc %03o Failed to listen on socket errno=%d: %s\n", pnc->ctrl, errno, strerror(errno));
      close(sock); sock = -1;
      break;
    }
  } while (0);

  do {
    fd_set fdset;
    FD_ZERO(&fdset);

    if(sock >= 0)
      FD_SET(sock, &fdset);
    int maxfd = sock;

    if(pnc->pipe[0] >= 0)
    {
      FD_SET(pnc->pipe[0], &fdset);
      if(pnc->pipe[0] > maxfd)
        maxfd = pnc->pipe[0];
    }

    if(pnc->rxrdy || !(pnc->ns & PNC_NS_CONN))
    {
      for(link_t *l = pnc->link; l; l = l->next)
      {
        if(l->fd >= 0)
        {
          FD_SET(l->fd, &fdset);
          if(l->fd > maxfd)
            maxfd = l->fd;
        }
      }
    }

    int rc;
    if((rc = select(maxfd+1, &fdset, NULL, NULL, NULL)) <= 0)
    {
      if(rc != EINTR && rc < 0)
      {
        logmsg("pnc %03o Select failed rc=%d: %s\n", pnc->ctrl, rc, strerror(rc));
        continue;
      }
    }

    if(sock >= 0 && FD_ISSET(sock, &fdset))
    {
      int fd;
      if((fd = accept(sock, NULL, NULL)) >= 0)
        pnc_connect(pnc, fd);
      else
        logmsg("pnc %03o Accept failed errno=%d: %s\n", pnc->ctrl, errno, strerror(errno));
    }

    if(!(pnc->ns & PNC_NS_CONN))
    {
      for(link_t *l = pnc->link; l; l = l->next)
        if(l->fd >= 0 && FD_ISSET(l->fd, &fdset))
        {
          if(pnc_recv(l->fd, &l->rb) < 0)
            pnc_disconnect(pnc, l);
          FD_CLR(l->fd, &fdset);
        }
    }

    if(pnc->pipe[0] >= 0 && FD_ISSET(pnc->pipe[0], &fdset))
    {
    uint8_t c;

      if(read(pnc->pipe[0], &c, 1) == 1)
        switch(c) {
          case PNC_RXRDY:
            pnc->rxrdy = 1;
            break;
          case PNC_TXRDY:
            pnc->txrdy = 1;
            break;
          case PNC_XCONN:
            pnc->ns |= PNC_NS_CONN|PNC_NS_TD;
            break;
          case PNC_XDISC:
            pnc->ns &= ~(PNC_NS_CONN|PNC_NS_TD);
            break;
          case PNC_TOKEN:
            pnc->ns |= PNC_NS_TD;
            break;
          case PNC_SIMTK:
            pnc->ns |= PNC_NS_TD;
            break;
          case PNC_INITP:
            pnc->ns = 0;
            pnc->txrdy = 0;
            pnc->rxrdy = 0;
            pnc->im = 0;
            io_clrint(cpu, &pnc->inttx);
            io_clrint(cpu, &pnc->intrx);
            break;
        }
    }

    if(!(pnc->ns & PNC_NS_CONN))
      continue;

    if(pnc->txrdy)
    {
    int n = pnc_dmx_copy(cpu, pnc->tx, pnc->tb.data.buff, sizeof(pnc->tb.data.buff), 1);
      pnc->txrdy = 0;
      logmsg("pnc %03o xmit %d %02x > %02x\n", pnc->ctrl, n, pnc->tb.data.buff[1], pnc->tb.data.buff[0]);

      if(pnc->rxrdy
        && n
        && (pnc->tb.data.buff[0] == pnc->nn))
      {
        pnc->ts |= PNC_XS_ACK;
        pnc_dmx_copy(cpu, pnc->rx, pnc->tb.data.buff, n, 0);
        pnc->rxrdy = 0;
        pnc->rs |= 0; // PNC_RS_BUSY;
        pnc->ns |= PNC_NS_RXI;
        if(pnc->im)
          io_setintv(cpu, &pnc->intrx, PNC_RX_VEC(pnc->iv));
      }

      for(link_t *l = pnc->link; l; l = l->next)
      {
        if(l->fd >= 0
          && n
          && l->nn != 255
          && (pnc->tb.data.buff[0] == 255 || pnc->tb.data.buff[0] == l->nn))
        {
          logmsg("pnc %03o send line %d %d %02x > %02x\n", pnc->ctrl, l->fd, n, pnc->tb.data.buff[1], pnc->tb.data.buff[0]);
          if(pnc_send(l->fd, &pnc->tb, n) < 0)
            pnc_disconnect(pnc, l);
          else
            pnc->ts |= PNC_XS_ACK;
        }
      }

      if(!(pnc->ts & PNC_XS_ACK) || pnc->tb.data.buff[0] == 255)
      {
        for(link_t *l = pnc->link; l; l = l->next)
        {
          if(l->fd >= 0
            && n
            && l->nn == 255)
          {
            logmsg("pnc %03o snd2 line %d %d %02x > %02x\n", pnc->ctrl, l->fd, n, pnc->tb.data.buff[1], pnc->tb.data.buff[0]);
            if(pnc_send(l->fd, &pnc->tb, n) < 0)
              pnc_disconnect(pnc, l);
            else
              if(pnc->tb.data.buff[0] == 255)
                pnc->ts |= PNC_XS_ACK;
          }
        }
      }

      pnc->ns |= PNC_NS_TXI;
      pnc->ts |= PNC_TS_BUSY;
      if(pnc->im && (pnc->ns & PNC_NS_TXI))
        io_setintv(cpu, &pnc->inttx, PNC_TX_VEC(pnc->iv));
    }

    if(pnc->rxrdy)
    {
      for(link_t *l = pnc->link; l; l = l->next)
      {
        if(pnc->rxrdy && l->fd >= 0 && FD_ISSET(l->fd, &fdset))
        {
          ssize_t n = pnc_recv(l->fd, &l->rb);
          if(n <= 0)
          {
            if(n < 0)
              pnc_disconnect(pnc, l);
            continue;
          }

          logmsg("pnc %03o recv line %d %d %02x > %02x\n", pnc->ctrl, l->fd, (int)n, l->rb.data.buff[1], l->rb.data.buff[0]);

          if(l->nn != l->rb.data.buff[1])
          {
            for(link_t *t = pnc->link; t; t = t->next)
              if(t->fd >= 0 && t->nn != 255 && t->nn == l->rb.data.buff[1])
              {
                pnc_disconnect(pnc, l);
                goto continu;
              }

            if(l->nn != l->rb.data.buff[1])
            {
              logmsg("pnc %03o recv line %d node %u changed to %u\n", pnc->ctrl, l->fd, l->nn, l->rb.data.buff[1]);
              l->nn = l->rb.data.buff[1];
            }
          }

          pnc_dmx_copy(cpu, pnc->rx, l->rb.data.buff, n, 0);
          pnc->rxrdy = 0;
          pnc->rs |= 0; // PNC_RS_BUSY;
          pnc->ns |= PNC_NS_RXI;
          if(pnc->im)
            io_setintv(cpu, &pnc->intrx, PNC_RX_VEC(pnc->iv));

          break;
        }
        continu:;
      }
    }

  } while(1);

  return NULL;
}


static void pnc_init(cpu_t *cpu, int type, int ext, int func, int ctrl, pnc_t **pnc, int argc, char *argv[])
{
  (*pnc) = calloc(1, sizeof(pnc_t));
  (*pnc)->id = io_id(061, ctrl);
  (*pnc)->ctrl = ctrl;

  pthread_cond_init(&(*pnc)->pthread.cond, NULL);
  pthread_mutex_init(&(*pnc)->pthread.mutex, NULL);
  pthread_attr_init(&(*pnc)->pthread.attr);
  pthread_attr_setdetachstate(&(*pnc)->pthread.attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setinheritsched(&(*pnc)->pthread.attr, PTHREAD_EXPLICIT_SCHED);
  if(cpu->sys->cap_sys_nice)
  {
    pthread_attr_setschedpolicy(&(*pnc)->pthread.attr, SCHED_FIFO);
    const struct sched_param param = { .sched_priority = sched_get_priority_max(SCHED_FIFO) };
    pthread_attr_setschedparam(&(*pnc)->pthread.attr, &param);
  }

  (*pnc)->intrx.i = -1;
  (*pnc)->inttx.i = -1;

  (*pnc)->cpu = cpu;

  if(!pipe((*pnc)->pipe))
  {
    ioctl((*pnc)->pipe[0], FIOCLEX, NULL);
    ioctl((*pnc)->pipe[1], FIOCLEX, NULL);
  }

  pthread_create(&(*pnc)->pthread.tid, &(*pnc)->pthread.attr, pnc_thread, (*pnc));
}

int pnc_io(cpu_t *cpu, int type, int ext, int func, int ctrl, void **devparm, int argc, char *argv[])
{
pnc_t *pnc = *devparm;

  switch(type) {
    case IO_TYPE_INA:
      switch(func) {
        case 011:
          logmsg("pnc %03o Input ID\n", ctrl);
          S_A(cpu, pnc->id);
          break;
        case 012:
          logmsg("pnc %03o Receive Status (%4.4x)\n", ctrl, pnc->rs);
          S_A(cpu, pnc->rs);
          break;
        case 013:
          logmsg("pnc %03o Transmit Status (%4.4x)\n", ctrl, pnc->ts);
          S_A(cpu, pnc->ts);
          break;
        case 014:
          logmsg("pnc %03o Receive DMx Channel (%4.4x)\n", ctrl, pnc->rx);
          S_A(cpu, pnc->rx);
          break;
        case 015:
          logmsg("pnc %03o Transmit DMx Channel (%4.4x)\n", ctrl, pnc->tx);
          S_A(cpu, pnc->tx);
          break;
        case 016:
          logmsg("pnc %03o Diagnostic Reg\n", ctrl);
          S_A(cpu, 0);
          break;
        case 017:
          logmsg("pnc %03o Network Status (%4.4x)\n", ctrl, (pnc->ns & 0xff00) | pnc->nn);
          S_A(cpu, (pnc->ns & 0xff00) | pnc->nn);
          break;
        default:
          logall("pnc %03o unsupported INA order %03o\n", ctrl, func);
      }
      break;
    case IO_TYPE_OTA:
      switch(func) {
        case 000:
          logmsg("pnc %03o Special Function %4.4x\n", ctrl, G_A(cpu));
          break;
        case 001:
          logmsg("pnc %03o Buffer Address %4.4x\n", ctrl, G_A(cpu));
          break;
        case 002:
          logmsg("pnc %03o Word Count %4.4x\n", ctrl, G_A(cpu));
          break;
        case 011:
          logmsg("pnc %03o Diagnostic Output %4.4x\n", ctrl, G_A(cpu));
          break;
        case 014:
          logmsg("pnc %03o Receive DMx Channel %4.4x\n", ctrl, G_A(cpu));
          pnc->rx = G_A(cpu);
          write(pnc->pipe[1], &pnc_rxrdy, 1);
          break;
        case 015:
          logmsg("pnc %03o Transmit DMx Channel %4.4x\n", ctrl, G_A(cpu));
          pnc->tx = G_A(cpu);
          write(pnc->pipe[1], &pnc_txrdy, 1);
          break;
        case 016:
          logmsg("pnc %03o Int Vector %4.4x\n", ctrl, G_A(cpu));
          pnc->iv = G_A(cpu);
          break;
        case 017:
          logmsg("pnc %03o Node Number %4.4x\n", ctrl, G_A(cpu));
          pnc->nn = G_A(cpu) & 0x00ff;
          break;
        default:
          logall("pnc %03o unsupported OTA Order %03o %4.4x\n", ctrl, func, G_A(cpu));
      }
      break;
    case IO_TYPE_OCP:
      switch(func) {
        case 000:
          logmsg("pnc %03o Disconnect\n", ctrl);
          write(pnc->pipe[1], &pnc_xdisc, 1);
          break;
        case 001:
          logmsg("pnc %03o Connect\n", ctrl);
          write(pnc->pipe[1], &pnc_xconn, 1);
          break;
        case 002:
          logmsg("pnc %03o Transmit Token\n", ctrl);
          write(pnc->pipe[1], &pnc_token, 1);
          break;
        case 003:
          logmsg("pnc %03o Simulate Token\n", ctrl);
          write(pnc->pipe[1], &pnc_simtk, 1);
          break;
        case 004:
          logmsg("pnc %03o Ackn Transmit Int\n", ctrl);
          pnc->ts = 0;
          pnc->ns &= ~PNC_NS_TXI;
          io_clrint(cpu, &pnc->inttx);
          break;
        case 005:
          logmsg("pnc %03o Add Delay\n", ctrl);
          break;
        case 006:
          logmsg("pnc %03o Set New Mode\n", ctrl);
          break;
        case 007:
          logmsg("pnc %03o Retransmit Pkt\n", ctrl);
          break;
        case 010:
          logmsg("pnc %03o Stop Transmission\n", ctrl);
          break;
        case 011:
          logmsg("pnc %03o Stop Receive\n", ctrl);
          break;
        case 012:
          logmsg("pnc %03o Set Normal Mode\n", ctrl);
          break;
        case 013:
          logmsg("pnc %03o Set Diagnostic Mode\n", ctrl);
          break;
        case 014:
          logmsg("pnc %03o Ackn Receive Int\n", ctrl);
          pnc->rs = 0;
          pnc->ns &= ~PNC_NS_RXI;
          io_clrint(cpu, &pnc->intrx);
          break;
        case 015:
          logmsg("pnc %03o Set Int Mask\n", ctrl);
          pnc->im = 1;
          if((pnc->ns & PNC_NS_TXI))
            io_setintv(cpu, &pnc->inttx, PNC_TX_VEC(pnc->iv));
          if((pnc->ns & PNC_NS_RXI))
            io_setintv(cpu, &pnc->intrx, PNC_RX_VEC(pnc->iv));
          break;
        case 016:
          logmsg("pnc %03o Clear Int Mask\n", ctrl);
          pnc->im = 0;
          io_clrint(cpu, &pnc->inttx);
          io_clrint(cpu, &pnc->intrx);
          break;
        case 017:
          logmsg("pnc %03o Initialize\n", ctrl);
          write(pnc->pipe[1], &pnc_initp, 1);
          break;
        default:
          logall("pnc %03o unsupported OCP order %03o\n", ctrl, func);
      }
      return 0;
      break;
    case IO_TYPE_SKS:
      switch(func) {
        default:
          logall("pnc %03o unsupported SKS Order %03o\n", ctrl, func);
          return 0;
      }
      break;
    case IO_TYPE_INI:
      pnc_init(cpu, type, ext, func, ctrl, (pnc_t **)devparm, argc, argv);
      break;
    case IO_TYPE_ASN:
      {
      char *ahost, *aport;
        switch(argc) {
          case 1:
            ahost = "localhost";
            aport = argv[0];
            break;
          case 2:
            ahost = argv[0];
            aport = argv[1];
            break;
          default:
            printf("pnc %03o invalid number of parameters\n", ctrl);
            return 1;
        }

        int port = pnc_inet_port(pnc, aport);
        if(!port)
        {
          printf("pnc %03o port %s invalid\n", ctrl, aport);
          break;
        }

        in_addr_t host = pnc_inet_host(pnc, ahost);
        if(host == INADDR_NONE)
        {
          printf("pnc %03o host %s invalid\n", ctrl, ahost);
          break;
        }

        int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(fd < 0)
        {
          printf("pnc %03o socket: %s\n", ctrl, strerror(errno));
          break;
        }

        ioctl(fd, FIOCLEX, NULL);

        struct sockaddr_in sockaddr = {.sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = host};
        if(connect(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)))
        {
          printf("pnc %03o connect %s:%s failed: %s\n", ctrl, ahost, aport, strerror(errno));
          close(fd);
          break;
        }

#ifdef DEBUG
        printf("pnc %03o connecting to %s:%s\n", ctrl, ahost, aport);
#endif

        pthread_mutex_lock(&pnc->pthread.mutex);
        pnc_connect(pnc, fd);
        pthread_mutex_unlock(&pnc->pthread.mutex);
      }
      break;
    default:
      logall("pnc %03o Invalid Type %d\n", ctrl, type);
  }
  return 1;
}
