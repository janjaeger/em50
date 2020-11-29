/* Asynchronous Multi Line Controller
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


#define LIBTELNET

#include "emu.h"

#include "io.h"

#include "amlc.h"

#include "queue.h"

#if 0
#undef logall
#define logall(...) PRINTF(__VA_ARGS__)
#endif

#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif

#ifdef LIBTELNET
static const telnet_telopt_t telopts[] = {
  { TELNET_TELOPT_BINARY,    TELNET_WILL, TELNET_DO   },
  { TELNET_TELOPT_SGA,       TELNET_WILL, TELNET_DO   },
  { TELNET_TELOPT_ECHO,      TELNET_WILL, TELNET_DO   },
  { TELNET_TELOPT_ENVIRON,   TELNET_WILL, TELNET_DONT },
  { TELNET_TELOPT_COMPRESS2, TELNET_WILL, TELNET_DONT },
  { -1, 0, 0 }
};
#endif


static amlc_t *amlc_device_table[AMLC_MAXDEV] = { NULL };
static inline line_t *amlc_getfreeline(cpu_t *cpu)
{
  for(int n = 0; n < AMLC_MAXDEV; n++)
  {
  amlc_t *amlc = amlc_device_table[n];

    if(amlc)
    {
      pthread_mutex_lock(&(amlc->pthread.mutex));

      for(int l = 0; l < AMLC_LINES; ++l)
      {
      line_t *line = &amlc->ln[l];

        if(line->fds == -1 && line->fdr == -1 && line->ls == offl)
          return line;
      }

      pthread_mutex_unlock(&(amlc->pthread.mutex));
    }
  }

  return NULL;
}


static inline line_t *amlc_getline(cpu_t *cpu, int ctrl, int ln)
{
  for(int n = 0; n < AMLC_MAXDEV; n++)
  {
  amlc_t *amlc = amlc_device_table[n];

    if(amlc && amlc->ctrl == ctrl)
    {
    line_t *line = &amlc->ln[ln];

      if(line->fds == -1 && line->fdr == -1 && line->ls == offl)
        return line;
    }
  }

  return NULL;
}


static inline void amlc_reassign(line_t *line, int amlc, int ln)
{
  if(line->amlc->ctrl == amlc && line->no == ln)
    return;

  line_t *newline = amlc_getline(line->amlc->cpu, amlc, ln);
  if(newline)
  {
#ifdef DEBUG
cpu_t *cpu = line->amlc->cpu;
logmsg("amlc %03o reassign line %d -> %03o:%d\n", line->amlc->ctrl, line->no, newline->amlc->ctrl, newline->no);
#endif
    if(newline->amlc != line->amlc)
      pthread_mutex_lock(&(newline->amlc->pthread.mutex));
#ifdef LIBTELNET
    newline->telnet = line->telnet; line->telnet = NULL;
    newline->tnparm = line->tnparm; line->tnparm = NULL;
    *(newline->tnparm) = newline;
#endif
    newline->fds = line->fds; line->fds = -1;
    newline->fdr = line->fdr; line->fdr = -1;
    newline->conn.inbinary = line->conn.inbinary; line->conn.inbinary = 0;
    newline->conn.outbinary = line->conn.outbinary; line->conn.outbinary = 0;
    newline->conn.amlc = newline->conn.ln = line->conn.amlc = line->conn.ln = 0;
    newline->ls = line->ls;   line->ls = offl;
    newline->ds = (ln << 12) | AMLC_DS_DSC3 | AMLC_DS_DSC2 | AMLC_DS_DSC1;

    if(newline->amlc != line->amlc)
    {
      newline->amlc->st |= (newline->amlc->st & ~AMLC_ST_LINE) | ln | AMLC_ST_DSC;
      pthread_mutex_unlock(&(newline->amlc->pthread.mutex));
    }
  }
}


static inline uint16_t amlc_dss(amlc_t *amlc)
{
uint16_t dss = 0;

  for(int n = 0, mask = 0x8000; n < AMLC_LINES; n++, mask >>= 1)
    if(amlc->ln[n].ls == offl)
      dss |= mask;

  return dss;
}


static inline int amlc_canrx(amlc_t *amlc)
{
cpu_t *cpu = amlc->cpu;
uint32_t sta, end;

  if(IO_IS_DMA(amlc->ca))
  {
    int nb = (amlc->ca & 0x02) ? -2 : 2;
    uint32_t dma = IO_DMA_CH(amlc->ca + (amlc->ra ? nb : 0));
    sta = G_DMA_A(cpu, dma);
    end = (sta + G_DMA_L(cpu, dma)) - 1;
  }
  else
  {
    uint32_t dmc = IO_DMC_CH(amlc->ca) + (amlc->ra ? 2 : 0);
    sta = ifetch_w(cpu, dmc);
    end = ifetch_w(cpu, dmc + 1);
  }

  int rc = (!sta || !end || sta > end) ? 0 : 1;

  return rc;
}


static inline void amlc_rxword(line_t *line, uint16_t word)
{
amlc_t *amlc = line->amlc;
cpu_t *cpu = amlc->cpu;
uint32_t dmx, sta, end;

  if(IO_IS_DMA(amlc->ca))
  {
    int nb = (amlc->ca & 0x02) ? -2 : 2;
    dmx = IO_DMA_CH(amlc->ca + (amlc->ra ? nb : 0));
    sta = G_DMA_A(cpu, dmx);
    end = (sta + G_DMA_L(cpu, dmx)) - 1;
  }
  else
  {
    dmx = IO_DMC_CH(amlc->ca) + (amlc->ra ? 2 : 0);
    sta = ifetch_w(cpu, dmx);
    end = ifetch_w(cpu, dmx + 1);
  }
logmsg("amlc %03o %8.8x %8.8x %8.8x c'%c'\n", amlc->ctrl, dmx, sta, end, word & 0x7f);

  if(sta && sta <= end)
  {
  uint16_t lc = (line->no << 12) | (word & 0x0fff);

    istore_w(cpu, sta++, lc);

    if(IO_IS_DMA(amlc->ca))
    {
      S_DMA_A(cpu, dmx, sta);
      S_DMA_L(cpu, dmx, (end - sta) + 1);
    }
    else
      istore_w(cpu, dmx, sta);

logmsg("amlc %03o stored %8.8x %8.8x %8.8x %4.4x c'%c'\n", amlc->ctrl, dmx, sta, end, word, word & 0x7f);
    if(sta > end)
    {
      amlc->st |= AMLC_ST_EOR;
      amlc->ra = !amlc->ra;
logmsg("amlc %03o eor ra %d stat %4.4x int %4.4x ena %d\n", amlc->ctrl, amlc->ra, amlc->st, amlc->va, amlc->im);
    }
  }
#if 1
  else
    logmsg("amlc %03o byte lost %d\n", amlc->ctrl, amlc->ra);
#endif
}


static inline void amlc_rxchar(line_t *line, uint8_t ch)
{
  amlc_rxword(line, AMLC_RX_VAL | (ch & amlc_cl_mask[line->cf & AMLC_CF_CLEN] & (line->conn.inbinary ? 0xff : 0x7f)));
}


static inline void amlc_rxbrk(line_t *line)
{
  amlc_rxword(line, AMLC_RX_EBRK);
}


#ifdef LIBTELNET
static const char *amlcvar = "EM50AMLC";
static void amlc_setenviron(telnet_t *telnet, line_t *line, int req)
{
char amlcval[8];
  snprintf(amlcval, sizeof(amlcval), "%03o:%d", line->conn.amlc, line->conn.ln);

  telnet_begin_sb(telnet, TELNET_TELOPT_ENVIRON);
  static const char cmd = TELNET_ENVIRON_INFO;
  telnet_send(telnet, &cmd, sizeof(cmd));
  telnet_newenviron_value(telnet, TELNET_ENVIRON_VAR, amlcvar);
  if(!req)
    telnet_newenviron_value(telnet, TELNET_ENVIRON_VALUE, amlcval);
  telnet_finish_sb(telnet);
}

static void amlc_getenviron(telnet_t *telnet, line_t *line, unsigned char type, char *var, char *value)
{
  if(type == TELNET_ENVIRON_VAR)
  {
    if(!strcmp(amlcvar, var))
    {
    int amlc, ln; char c;
      if(sscanf(value, "%o%c%d%c", &amlc, &c, &ln, &c) == 3 && amlc)
        amlc_reassign(line, amlc, ln);
    }
  }
}


static void amlc_telnet_event(telnet_t *telnet, telnet_event_t *ev, void *parm)
{
line_t *line = *(line_t **)parm;
#ifdef DEBUG
cpu_t *cpu = line->amlc->cpu;
#endif

  switch (ev->type) {
    case TELNET_EV_DATA: // data received
      for(int n = 0; n < ev->data.size; n++)
        amlc_rxchar(line, *(ev->data.buffer + n));
      break;
    case TELNET_EV_SEND: // data sent
      send(line->fds, ev->data.buffer, ev->data.size, 0);
      break;
    case TELNET_EV_IAC: // generic IAC
      switch(ev->iac.cmd) {
        case TELNET_BREAK:
        case TELNET_ABORT:
        case TELNET_IP:
          amlc_rxbrk(line);
          break;
        default:
          break;
      }
      break;  
    case TELNET_EV_WILL: // request to enable remote feature (or receipt)
      switch(ev->neg.telopt) {
        case TELNET_TELOPT_BINARY:
          line->conn.inbinary = 1;
          break;
        case TELNET_TELOPT_ENVIRON:
          amlc_setenviron(telnet, line, 0);
          break;
        default:
          break;
      }
      break;
    case TELNET_EV_WONT: // notification of disabling remote feature (or receipt)
      switch(ev->neg.telopt) {
        case TELNET_TELOPT_BINARY:
          line->conn.inbinary = 0;
          break;
        default:
          break;
      }
      break;
    case TELNET_EV_DO: // request to enable local feature (or receipt)
      switch(ev->neg.telopt) {
        case TELNET_TELOPT_BINARY:
          line->conn.outbinary = 1;
          break;
        case TELNET_TELOPT_COMPRESS2:
          telnet_begin_compress2(telnet);
          break;
        case TELNET_TELOPT_ENVIRON:
          amlc_setenviron(telnet, line, 1);
          break;
        default:
          break;
      }
      break;
    case TELNET_EV_DONT: // demand to disable local feature (or receipt)
      switch(ev->neg.telopt) {
        case TELNET_TELOPT_BINARY:
          line->conn.outbinary = 0;
          break;
        default:
          break;
      }
      break;
    case TELNET_EV_ENVIRON:
      amlc_getenviron(telnet, line, ev->environ.values->type, ev->environ.values->var, ev->environ.values->value);
      break;
    case TELNET_EV_TTYPE: // respond to TTYPE commands
      break;
    case TELNET_EV_SUBNEGOTIATION: // respond to particular subnegotiations
      break;
    case TELNET_EV_ERROR: // error
      logall("amlc %03o telnet error\n", line->amlc->ctrl);
      break;
    default:
      break;
  }
}
#endif


static inline int amlc_attach(cpu_t *cpu, int fd)
{
  line_t *line = amlc_getfreeline(cpu);

  if(!line)
    return -1;

  line->conn.amlc = line->conn.ln = line->conn.inbinary = line->conn.outbinary = 0;

  line->fds = line->fdr = fd;
  line->ls = conn;

  pthread_mutex_unlock(&(line->amlc->pthread.mutex));

  return 0;
}


static inline void amlc_detach(line_t *line)
{
  close(line->fdr);
  if(line->ls == loop)
    close(line->fds);
  line->fdr = line->fds = -1;
  line->ls = offl;
  line->conn.amlc = line->conn.ln = line->conn.inbinary = line->conn.outbinary = 0;

#ifdef LIBTELNET
  if(line->telnet)
  {
    telnet_free(line->telnet);
    line->telnet = NULL;
  }

  if(line->tnparm)
  {
    free(line->tnparm);
    line->tnparm = NULL;
  }
#endif
}


static inline void amlc_loop(line_t *line)
{
int pipefd[2];
#ifdef DEBUG
cpu_t *cpu = line->amlc->cpu;
#endif

  if(!pipe(pipefd))
  {
    line->fdr = pipefd[0];
    line->fds = pipefd[1];
    int opt = 1;
    ioctl(pipefd[0], FIONBIO, &opt);
    ioctl(pipefd[1], FIONBIO, &opt);
    ioctl(pipefd[0], FIOCLEX, NULL);
    ioctl(pipefd[1], FIOCLEX, NULL);
    line->ls = loop;
  }
  else
  {
    logall("amlc %03o pipe error\n", line->amlc->ctrl);
  }
}


static inline int amlc_inet_port(cpu_t *cpu, const char *serv)
{
struct servent *servent;

  if(isdigit(*serv))
    return atoi(serv);
  else
    if((servent = getservbyname(serv, "tcp")))
      return htons(servent->s_port);

  logmsg("amlc getservbyname(%s) failed", serv);
  return 0;
}


static inline in_addr_t amlc_inet_host(cpu_t *cpu, const char *host)
{
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

  logmsg("amlc gethostbyname(%s) failed",host);
  return INADDR_NONE;
}


static void *amlc_listener(void *parm)
{
cpu_t *cpu = parm;
struct sockaddr_in bindsock;
int sock;
int fd;
int optval = 1;
int rc;
fd_set fdset;

  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    logmsg("amlc-l Failed to obtain socket errno=%d: %s\n", errno, strerror(errno));
    return NULL;
  }

  ioctl(sock, FIOCLEX, NULL);

  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
    logmsg("amlc-l setsockopt(SO_REUSEADDR) failed errno=%d: %s\n", errno, strerror(errno));

  memset(&bindsock, 0x00, sizeof(bindsock));
  bindsock.sin_family = AF_INET;
  bindsock.sin_addr.s_addr = INADDR_ANY;

  const char *iface = cpu->sys->bind;
  const char *port  = cpu->sys->port;

  if(iface)
  {
    if(INADDR_NONE == (bindsock.sin_addr.s_addr = amlc_inet_host(cpu, iface)))
    {
      logmsg("amlc-l No such interface: %s\n", iface);
      return NULL;
    }
  }

  if(port)
    bindsock.sin_port = htons(amlc_inet_port(cpu, port));
  else
    bindsock.sin_port = htons(AMLC_DFLTPORT);

  char tname[16];
  snprintf(tname, sizeof(tname), "amlc-l %u", ntohs(bindsock.sin_port));
  pthread_setname_np(pthread_self(), tname);
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTSTP);
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  if(bind(sock, (struct sockaddr *)&bindsock, sizeof(bindsock)))
  {
    logmsg("amlc-l Failed to bind to socket errno=%d: %s\n", errno, strerror(errno));
    return NULL;
  }

  if(listen(sock, AMLC_BACKLOG))
  {
    logmsg("amlc-l Failed to listen on socket errno=%d: %s\n", errno, strerror(errno));
    return NULL;
  }

  do {

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);

    if((rc = select(sock+1, &fdset, NULL, NULL, NULL)) <= 0)
    {
      if(rc == EINTR || rc == 0)
        continue;

      logmsg("amlc-l Select failed rc=%d: %s\n", rc, strerror(rc));
      return NULL;
    }

    if(FD_ISSET(sock, &fdset))
    {
      if((fd = accept(sock, NULL, NULL)) < 0)
      {
        logmsg("amlc-l Accept failed errno=%d: %s\n", errno, strerror(errno));
        continue;
      }

      if(amlc_attach(cpu, fd))
      {
        logmsg("amlc-l No line available\n");
        close(fd);
      }
      else
        logmsg("amlc-l Line started socket=%d\n", fd);
    }

  } while(1);

}


static pthread_t amlc_listener_tid = 0;
static inline void amlc_connect(amlc_t *amlc)
{
  int n = io_getslot(amlc->id);
  if(n >= AMLC_MAXDEV)
    n = 0;
  for(; n < AMLC_MAXDEV; n++)
  {
    if(!amlc_device_table[n] || amlc_device_table[n] == amlc)
    {
      amlc_device_table[n] = amlc;
      break;
    }
  }
}


static inline int amlc_send(line_t *line, uint8_t ch)
{
  if(line->ls != loop)
  {
#ifdef LIBTELNET
    char c = ch & amlc_cl_mask[line->cf & AMLC_CF_CLEN] & (line->conn.outbinary ? 0xff : 0x7f);
    telnet_send(line->telnet, &c, 1);   
    return 1;
#else
    return send(line->fds, &ch, 1, 0);
#endif
  }
  else
    return write(line->fds, &ch, 1);
}


static int amlc_recv(line_t *line)
{
int rc;
char c;

  if(line->ls != loop)
  {
#ifdef LIBTELNET
    rc = recv(line->fdr, &c, 1, 0);
    if(rc == 1)
      telnet_recv(line->telnet, &c, 1);
#else
    rc = recv(line->fdr, &c, 1, 0);
    if(rc == 1)
      amlc_rxchar(line, c);
#endif
  }
  else
  {
    rc = read(line->fdr, &c, 1);
    if(rc == 1)
      amlc_rxchar(line, c);
  }

  return rc;
}


static void *amlc_thread(void *parm)
{
amlc_t *amlc = parm;
cpu_t *cpu = amlc->cpu;

  char tname[16];
  snprintf(tname, sizeof(tname), "amlc %03o", amlc->ctrl);
  pthread_setname_np(pthread_self(), tname);
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTSTP);
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  pthread_mutex_lock(&(amlc->pthread.mutex));

  while(amlc->pthread.tid)
  {
if(!amlc->in) amlc->in = 1;
    if(amlc->im
     && (amlc->st & (AMLC_ST_CTI1|AMLC_ST_DSC|AMLC_ST_EOR)))
{
logmsg("amlc %03o int stat %04x\n", amlc->ctrl, amlc->st);
      io_setintv(cpu, &(amlc->intr), amlc->va);
}

    pthread_mutex_unlock(&(amlc->pthread.mutex));
    int didsend = 0;

    for(int ln = 0; ln < AMLC_LINES; ++ln)
    {
      line_t *line = &amlc->ln[ln];

      if(amlc->da != 0 && (line->cn & AMLC_CN_XMIT))
      {
        uint32_t dmx = (amlc->da & 0xfff0) | (ln << (amlc->dm ? 2 : 0));

        uint16_t ch;

        if(amlc->dm)
        {
          while(!io_rtq(cpu, dmx, &ch))
          {
            didsend = 1;
logmsg("amlc %03o %4.4x '%c'\n", amlc->ctrl, ch, ch & 0x7f);
            if((line->ls == onln || line->ls == loop) && (ch & AMLC_TX_VAL))
            {
logmsg("amlc %03o line %d '%c'\n", amlc->ctrl, ln, ch & 0x7f);
              if(amlc_send(line, ch) != 1)
logmsg("amlc %03o line %d send failed\n", amlc->ctrl, ln);
            }
          }
        }
        else
        {
          ch = ifetch_w(cpu, dmx);
          if(ch != 0)
          {
            didsend = 1;
            istore_w(cpu, dmx, 0);
logmsg("amlc %03o line %d fetch %4.4X %4.4x\n", amlc->ctrl, ln, dmx, ch);
            if((line->ls == onln || line->ls == loop) && (ch & AMLC_TX_VAL))
            {
logmsg("amlc %03o line %d '%c'\n", amlc->ctrl, ln, ch & 0x7f);
              if(amlc_send(line, ch) != 1)
logmsg("amlc %03o line %d send failed\n", amlc->ctrl, ln);
            }
          }
        }
      }
    }
    if(didsend)
      io_idle_post(cpu);

    fd_set fdrset;
    int fdmax = -1;
    FD_ZERO(&fdrset);

    pthread_mutex_lock(&(amlc->pthread.mutex));
    for(int ln = 0; ln < AMLC_LINES; ++ln)
    {
    line_t *line = &amlc->ln[ln];

      if(line->fdr >= 0)
      {
        FD_SET(line->fdr, &fdrset);
        if(line->fdr > fdmax)
          fdmax = line->fdr;
      }

      if(line->ls == conn)
      {
logmsg("amlc %03o line %d connected\n",amlc->ctrl, ln);

        line->ls = onln;
#ifdef LIBTELNET
        void **parm = malloc(sizeof(void*));
        *parm = line;
        line->tnparm = parm;
        line->telnet = telnet_init(telopts, amlc_telnet_event, 0, parm);
        telnet_negotiate(line->telnet, TELNET_WILL, TELNET_TELOPT_ECHO);
        telnet_negotiate(line->telnet, TELNET_DO,   TELNET_TELOPT_ECHO);
        telnet_negotiate(line->telnet, TELNET_WILL, TELNET_TELOPT_SGA);
        telnet_negotiate(line->telnet, TELNET_DO,   TELNET_TELOPT_SGA);

        if(line->conn.outbinary)
          telnet_negotiate(line->telnet, TELNET_WILL, TELNET_TELOPT_BINARY);
        telnet_negotiate(line->telnet, TELNET_DO,   TELNET_TELOPT_BINARY);

        if(line->conn.amlc)
        {
          telnet_negotiate(line->telnet, TELNET_WILL, TELNET_TELOPT_ENVIRON);
          telnet_negotiate(line->telnet, TELNET_DO,   TELNET_TELOPT_ENVIRON);
        }
#endif

        amlc->st |= (amlc->st & ~AMLC_ST_LINE) | ln | AMLC_ST_DSC;
        line->ds = (ln << 12) | AMLC_DS_DSC3 | AMLC_DS_DSC2 | AMLC_DS_DSC1;

        continue;
      }
    }

    struct timeval tv = { .tv_sec = 0, .tv_usec = didsend ? 1000 : 10000 };
    pthread_mutex_unlock(&(amlc->pthread.mutex));
    int rc = select(amlc_canrx(amlc) ? fdmax+1 : 0, &fdrset, NULL, NULL, &tv);
    pthread_mutex_lock(&(amlc->pthread.mutex));

    int didrecv = 0;
    if(rc > 0)
    {
      for(int ln = 0; ln < AMLC_LINES; ++ln)
      {
      line_t *line = &amlc->ln[ln];
  
        if(!(line->cn & AMLC_CN_RECV))
          continue;

        if(line->fdr < 0 || !FD_ISSET(line->fdr, &fdrset))
          continue;

        if(!amlc_canrx(amlc))
          continue;

        FD_CLR(line->fdr, &fdrset);

        if(amlc_recv(line) != 1)
        {
logmsg("amlc line %d closed\n", ln);
          amlc_detach(line);
// TODO SET STATUS
          amlc->st |= (amlc->st & ~AMLC_ST_LINE) | ln | AMLC_ST_DSC;
          line->ds = (ln << 12);
          continue;
        }
        else
          didrecv = 1;

        if((line->cn & AMLC_CN_TIME) && !(amlc->st & AMLC_ST_EOR))
          amlc->st |= (amlc->st & AMLC_ST_CTI1) ? (AMLC_ST_CTI1|AMLC_ST_CTI2) : AMLC_ST_CTI1;
        continue;
      }
    }
    else if(rc == 0)
      for(int ln = 0; ln < AMLC_LINES; ++ln)
      {
      line_t *line = &amlc->ln[ln];
        if((line->cn & AMLC_CN_TIME) && !(amlc->st & AMLC_ST_EOR))
        {
          if((amlc->st & AMLC_ST_CTI1) && (amlc->st & AMLC_ST_LINE) != ln)
          {
            amlc->st = (amlc->st & ~AMLC_ST_LINE) | ln;
            amlc->st |= AMLC_ST_CTI2 | ln;
          }
          else
            amlc->st |= AMLC_ST_CTI1 | ln;
        }
      }

    if(didrecv)
      io_idle_post(cpu);
  }

  pthread_mutex_unlock(&(amlc->pthread.mutex));

  return NULL;
}


static inline void amlc_init(cpu_t *cpu, int type, int ext, int func, int ctrl, amlc_t **amlc, int argc, char *argv[])
{
  if(cpu->sys->port && !strcasecmp(cpu->sys->port, "none"))
  {
    (*amlc) = NULL;
    return;
  }

  (*amlc) = calloc(1, sizeof(amlc_t));
  (*amlc)->id = io_id(020254, ctrl);
  (*amlc)->ctrl = ctrl;
  (*amlc)->va = 0154;

  for(int ln = 0; ln < AMLC_LINES; ++ln)
  {
    (*amlc)->ln[ln].no = ln;
    (*amlc)->ln[ln].fds = (*amlc)->ln[ln].fdr = -1;
    (*amlc)->ln[ln].amlc = (*amlc);
  }
  (*amlc)->dv = -1;

  (*amlc)->intr.i = -1;
  (*amlc)->cpu = cpu;
  pthread_attr_init(&(*amlc)->pthread.attr);
  pthread_attr_setdetachstate(&(*amlc)->pthread.attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setinheritsched(&(*amlc)->pthread.attr, PTHREAD_EXPLICIT_SCHED);
  if(cpu->sys->cap_sys_nice)
  {
    pthread_attr_setschedpolicy(&(*amlc)->pthread.attr, SCHED_RR);
//  const struct sched_param param = { .sched_priority = sched_get_priority_max(SCHED_RR) };
//  pthread_attr_setschedparam(&(*amlc)->pthread.attr, &param);
  }
  pthread_mutex_init(&(*amlc)->pthread.mutex, NULL);

  amlc_connect(*amlc);

  if(!amlc_listener_tid)
    pthread_create(&amlc_listener_tid, &(*amlc)->pthread.attr, amlc_listener, (*amlc)->cpu);

  pthread_create(&(*amlc)->pthread.tid, &(*amlc)->pthread.attr, amlc_thread, *amlc);
}


int amlc_io(cpu_t *cpu, int type, int ext, int func, int ctrl, void **devparm, int argc, char *argv[])
{
amlc_t *amlc = *devparm;

  switch(type) {
    case IO_TYPE_INA:
#if 1
      pthread_mutex_lock(&(amlc->pthread.mutex));
#else
      if(pthread_mutex_trylock(&(amlc->pthread.mutex)))
      {
        pthread_yield();
        return 0;
      }
#endif
      switch(func) {
        case 000:  // Input Data Set Status
          if(amlc->dv >= 0)
          {
            logmsg("amlc %03o Input Data Set Status line %d %4.4X\n", ctrl, amlc->dv, amlc->ln[amlc->dv].ds);
            S_A(cpu, amlc->ln[amlc->dv].ds);
            amlc->dv = -1;
          }
          else
          {
            logmsg("amlc %03o Input Data Set Status %4.4X\n", ctrl, amlc_dss(amlc));
            S_A(cpu, amlc_dss(amlc));
          }
          break;
        case 007:  // Input and Clear Status
          if(amlc->pthread.tid) amlc->st |= AMLC_ST_CLK;
          if(amlc->ra) amlc->st |= AMLC_ST_BUF;
          if(amlc->im) amlc->st |= AMLC_ST_IENA;
          if(amlc->dm) amlc->st |= AMLC_ST_DMQ;
          logmsg("amlc %03o Input and Clear Status %4.4X\n", ctrl, amlc->st);
          S_A(cpu, amlc->st);
#if 1
          if(io_clrint(cpu, &amlc->intr))
            logmsg("amlc %03o intr clear\n", amlc->ctrl);
          else
#endif
            amlc->st = 0;
          break;
        case 011:  // Input ID
          logmsg("amlc %03o Input ID %4.4x\n", ctrl, amlc->id);
          S_A(cpu, amlc->id);
          break;
        case 014:  // DMA/DMC Channel
          logmsg("amlc %03o Input DMA/DMC Channel %4.4x\n", ctrl, amlc->ca);
          S_A(cpu, amlc->ca);
          break;
        case 015:  // DMT base Address
          logmsg("amlc %03o Input DMT Base Address %4.4x\n", ctrl, amlc->da);
          S_A(cpu, amlc->da);
          break;
        case 016:  // Input Vector Address
          logmsg("amlc %03o Input Vector Address %4.4x\n", ctrl, amlc->va);
          S_A(cpu, amlc->va);
          break;
        default:
          logall("amlc %03o unsupported INA order %03o\n", ctrl, func);
      }
      pthread_mutex_unlock(&(amlc->pthread.mutex));
      break;
    case IO_TYPE_OTA:
      if(!amlc->in)
      {
        amlc->in = 1;
        return 0;
      }
#if 1
      pthread_mutex_lock(&(amlc->pthread.mutex));
#else
      if(pthread_mutex_trylock(&(amlc->pthread.mutex)))
      {
        pthread_yield();
        return 0;
      }
#endif

      {
        uint16_t a = G_A(cpu);
        switch(func) {
          case 000:  // Output Line no. to Read DSS
            logmsg("amlc %03o Output Line no. to Read DSS %4.4x\n", ctrl, a);
            amlc->dv = a >> 12;
            break;
          case 001:  // Output Line Configuration
            logmsg("amlc %03o Output Line Configuration %4.4x\n", ctrl, a);
            if((amlc->ln[a >> 12].cf & AMLC_CF_DSC) && !(a & AMLC_CF_DSC) && amlc->ln[a >> 12].ls != offl)
            {
              amlc->ln[a >> 12].cf = a;
              amlc_detach(&amlc->ln[a >> 12]);
            }
            else
              amlc->ln[a >> 12].cf = a;
            if((amlc->ln[a >> 12].cf & AMLC_CF_LOOP) && amlc->ln[a >> 12].ls != loop)
              amlc_loop(&amlc->ln[a >> 12]);
            if(!(amlc->ln[a >> 12].cf & AMLC_CF_LOOP) && amlc->ln[a >> 12].ls == loop)
              amlc_detach(&amlc->ln[a >> 12]);
            break;
          case 002:  // Output Line Control
            logmsg("amlc %03o Output Line Control %4.4x\n", ctrl, a);
            amlc->ln[a >> 12].cn = a;
            break;
          case 003:  // Output DSC
            logmsg("amlc %03o Output DSC %4.4x\n", ctrl, a);
            amlc->ln[a >> 12].ds = a;
            break;
          case 014:  // DMA/DMC Channel
            logmsg("amlc %03o Output DMA/DMC Channel %4.4x\n", ctrl, a);
            amlc->ca = a;
            amlc->ra = 0;
            io_clrint(cpu, &(amlc->intr));
            break;
          case 015:  // DMT base Address
            logmsg("amlc %03o Output DMT Base Address %4.4x\n", ctrl, a);
            amlc->da = a;
            break;
          case 016:  // Interrupt Vector Address
            logmsg("amlc %03o Output Interrupt Vector Address %4.4x\n", ctrl, a);
            amlc->va = a;
            break;
          case 017:  // Programmable Async Clock
            logmsg("amlc %03o Programmable Async Clock %4.4x\n", ctrl, a);
            amlc->cl = a;
            break;
          default:
            logall("amlc %03o unsupported OTA order %03o %4.4x\n", ctrl, func, a);
        }
      }
      pthread_mutex_unlock(&(amlc->pthread.mutex));
      break;
    case IO_TYPE_OCP:
      switch(func) {
        case 000:   // Stop Clock
          logmsg("amlc %03o Stop Clock\n", ctrl);
          break;
        case 001:   // Single Step Clock
          logmsg("amlc %03o Single Step Clock\n", ctrl);
          break;
        case 012:   // Set Normal Mode
          logmsg("amlc %03o Set Normal Mode\n", ctrl);
          amlc->dm = 0;
          break;
        case 013:   // Set DMQ Mode
          logmsg("amlc %03o Set DMQ Mode\n", ctrl);
          amlc->dm = 1;
          break;
        case 015:   // Set Interrupt Mask
          logmsg("amlc %03o Set Interrupt Mask\n", ctrl);
          amlc->im = 1;
          break;
        case 016:   // Clear Interrupt Mask
          logmsg("amlc %03o Clear Interrupt Mask\n", ctrl);
          amlc->im = 0;
          break;
        case 017:   // Initialise
          logmsg("amlc %03o Initialise\n", ctrl);
          if(amlc->in)
          {
            io_clrint(cpu, &amlc->intr);
            amlc->ra = 0;
            amlc->st = 0;
            amlc->in = 0;
            amlc->dm = 0;
            amlc->im = 0;
            amlc->ca = 0;
          }
          break;
        default:
          logall("amlc %03o unsupported OCP order %03o\n", ctrl, func);
      }
      break;
    case IO_TYPE_SKS:
      switch(func) {
        case 004:  // Skip not interrupting
          logmsg("amlc %03o Skip if not interrupting\n", ctrl);
          if(io_tstint(cpu, &amlc->intr))
            return 0;
          break;
        default:
          logall("amlc %03o unsupported SKS order %03o\n", ctrl, func);
      }
      break;
    case IO_TYPE_INI:
      amlc_init(cpu, type, ext, func, ctrl, (amlc_t **)devparm, argc, argv);
      break;
    case IO_TYPE_ASN:
      {
      char *ahost, *aport, *aamlc = NULL, *aline = NULL;
        switch(argc) {
          case 2:
            ahost = "localhost";
            aport = argv[1];
            break;
          case 3:
            ahost = argv[1];
            aport = argv[2];
            break;
          case 4:
            ahost = "localhost";
            aport = argv[1];
            aamlc = argv[2];
            aline = argv[3];
            break;
          case 5:
            ahost = argv[1];
            aport = argv[2];
            aamlc = argv[3];
            aline = argv[4];
            break;
          default:
            printf("amlc %03o invalid number of parameters\n", ctrl);
            return 1;
        }

        int ln; char c;
        if(sscanf(argv[0],"%d%c", &ln, &c) != 1
          || ln < 0 || ln >= AMLC_LINES)
        {
          printf("amlc %03o line %d invalid\n", ctrl, ln);
          break;
        }

        int namlc, nline;
        if(aamlc && aline)
        {
          if(sscanf(aamlc,"%o%c", &namlc, &c) != 1
            || namlc < 0 || namlc >= 0100)
          {
            printf("amlc %03o line %d invalid target amlc device %s\n", ctrl, ln, aamlc);
            break;
          }

          if(sscanf(aline,"%d%c", &nline, &c) != 1
            || nline < 0 || nline >= AMLC_LINES)
          {
            printf("amlc %03o line %d invalid target line number %s\n", ctrl, ln, aline);
            break;
          }
        }
        else
          namlc = nline = 0;

        line_t *line = &amlc->ln[ln];

        if(line->ls != offl)
        {
          printf("amlc %03o line %d in use\n", ctrl, ln);
          break;
        }

        int port = amlc_inet_port(cpu, aport);
        if(!port)
        {
          printf("amlc %03o port %s invalid\n", ctrl, aport);
          break;
        }

        in_addr_t host = amlc_inet_host(cpu, ahost);
        if(host == INADDR_NONE)
        {
          printf("amlc %03o host %s invalid\n", ctrl, ahost);
          break;
        }

        int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(fd < 0)
        {
          printf("amlc %03o socket: %s\n", ctrl, strerror(errno));
          break;
        }
        ioctl(fd, FIOCLEX, NULL);

        pthread_mutex_lock(&amlc->pthread.mutex);
        if(line->ls == offl)
        {
          line->fds = line->fdr = fd;
          line->ls = conn;
          line->conn.amlc = namlc;
          line->conn.ln = nline;
          line->conn.outbinary = 1;
        }
        else
        {
          printf("amlc %03o line %d in use\n", ctrl, ln);
          close(fd);
        }
        pthread_mutex_unlock(&amlc->pthread.mutex);

        struct sockaddr_in sockaddr = {.sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = host};
        if(connect(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)))
        {
          printf("amlc %03o line %d connect %s:%s failed: %s\n", ctrl, ln, ahost, aport, strerror(errno));

          pthread_mutex_lock(&amlc->pthread.mutex);
          line->ls = offl;
          line->fds = line->fdr = -1;
          line->conn.amlc = line->conn.ln = line->conn.outbinary = 0;
          pthread_mutex_unlock(&amlc->pthread.mutex);

          close(fd);
          break;
        }

        if(aamlc && aline)
          printf("amlc %03o line %d connecting to %s:%s amlc %03o line %d\n", ctrl, ln, ahost, aport, namlc, nline);
        else
          printf("amlc %03o line %d connecting to %s:%s\n", ctrl, ln, ahost, aport);

      }
      break;
    default:
      logmsg("amlc %03o: invalid type %d\n", ctrl, type);
  }
  return 1;
}
