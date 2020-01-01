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


#ifndef _tape_h
#define _tape_h

#define _MT_EOM 0xffffffff
#define _MT_GAP 0xfffffffe
#define _MT_TMK 0x00000000
#define _MT_LNM 0x00ffffff
#define _MT_ERM 0x80000000

#define MT_BLN(_x) ((_x) & _MT_LNM)

#define IS_ERR(_x) ((_x) & _MT_ERM)
#define IS_EOM(_x) ((_x) == _MT_EOM)
#define IS_TMK(_x) ((_x) == _MT_TMK)

#define MT_TMK (0)
#define MT_ERR (-1)
#define MT_EOM (-2)
#define MT_BOF (-3)
#define MT_BOT (-4)
#define MT_OFL (-5)
#define MT_ONL (-6)
#define MT_WTM (-7)

#define MT_ADDR_IPL (0200)


struct mt_t;
typedef struct {
  struct mt_t *mt;
  char *fn;
  enum { cl, rd, wr, wa } md;
  int fd;
  uint16_t sw;
  size_t max;
} tm_t;

typedef struct mt_t {
  cpu_t *cpu;
int busy;
int pending;
  uint16_t ctrl;
  struct {
    pthread_t tid;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
  } pthread;
  enum { current_status_word=0, id_number=1, dmx_channel_number=2, vector_interrupt_address=3 } cdr;
  union {
    uint16_t dr[4];
    struct {
      uint16_t sw;
#define MT_SW_PAR 0x8000
#define MT_SW_RUN 0x4000
#define MT_SW_CRC 0x2000
#define MT_SW_LRC 0x1000
#define MT_SW_DMX 0x0800
#define MT_SW_UNC 0x0400
#define MT_SW_RAW 0x0200
#define MT_SW_TMK 0x0100
#define MT_SW_RDY 0x0080
#define MT_SW_ONL 0x0040
#define MT_SW_EOT 0x0020
#define MT_SW_REW 0x0010
#define MT_SW_BOT 0x0008
#define MT_SW_PRO 0x0004
#define MT_SW_OVR 0x0002
#define MT_SW_INT 0x0001
      uint16_t id;
      uint16_t dc;
#define MT_DC_DMC 0x800
#define MT_DC_CHA 0x7FF
      uint16_t va;
    };
  };
  uint16_t mo;
  int ff;
  int dv;
  intr_t intr;
  tm_t tm[4];
} mt_t;

ssize_t mt_load(char *, uint8_t *, size_t);

int tape_io(cpu_t *, int, int, int, int, void **, int, char *[]);


static inline int mt_unit(int mask)
{
static const int unit[] = { -1,  3,  2, -1,
                             1, -1, -1, -1,
                             0, -1, -1, -1,
                            -1, -1, -1, -1 };
  return unit[mask & 0b1111];
}

static inline ssize_t mt_rew(tm_t *tm)
{
off_t rc = lseek(tm->fd, 0, SEEK_SET);

  return rc ? rc : MT_BOT;
}

static inline ssize_t mt_read(tm_t *tm, uint8_t *rec, size_t len)
{
uint32_t meta;
ssize_t rc, bln, pln;
/* off_t pos; */

  rc = read(tm->fd, &meta, sizeof(meta));

  if(rc < sizeof(meta))
  {
  struct stat st;
    if(fstat(tm->fd, &st))
      return MT_EOM;
    else
      return st.st_size ? MT_EOM : MT_BOT;
  }

  if(IS_TMK(meta))
    return MT_TMK;

  if(IS_EOM(meta))
  {
    /* pos = */ lseek(tm->fd, -sizeof(meta), SEEK_CUR);
    return MT_EOM;
  }

  bln = MT_BLN(meta);
  pln = (bln + 1) & ~1;

  if(IS_ERR(meta))
  {
    /* pos = */ lseek(tm->fd, pln + sizeof(meta), SEEK_CUR);
    return MT_ERR;
  }

  if(len >= bln)
  {
    rc = read(tm->fd, rec, bln);
    if(pln > bln)
      /* pos = */ lseek(tm->fd, pln - bln, SEEK_CUR);
  }
  else
  {
    rc = read(tm->fd, rec, len);
    /* pos = */ lseek(tm->fd, pln - len, SEEK_CUR);
  }

  rc = read(tm->fd, &meta, sizeof(meta));

  if(MT_BLN(meta) != bln)
    return MT_ERR;

  return bln;
}

static inline ssize_t mt_rdbk(tm_t *tm, uint8_t *rec, size_t len)
{
uint32_t meta;
ssize_t /* rc, */ bln, pln;
off_t pos;

  pos = lseek(tm->fd, -sizeof(meta), SEEK_CUR);

  if(pos <= 0)
    return MT_BOF;

  /* rc = */ read(tm->fd, &meta, sizeof(meta));

  if(IS_TMK(meta))
  {
    pos = lseek(tm->fd, -sizeof(meta), SEEK_CUR);
    return MT_TMK;
  }

  bln = MT_BLN(meta);
  pln = (bln + 1) & ~1;

  if(IS_ERR(meta))
  {
    pos = lseek(tm->fd, -(pln + 2*sizeof(meta)), SEEK_CUR);
    return MT_ERR;
  }

  if(len >= bln)
  {
    pos = lseek(tm->fd, -(pln + sizeof(meta)), SEEK_CUR);

    /* rc = */ read(tm->fd, rec, bln);

    pos = lseek(tm->fd, -(bln + sizeof(meta)), SEEK_CUR);
  }
  else
  {
    pos = lseek(tm->fd, -((pln - (bln - len)) + sizeof(meta)), SEEK_CUR);

    /* rc = */ read(tm->fd, rec, len);

    pos = lseek(tm->fd, -(bln + sizeof(meta)), SEEK_CUR);
  }

  return bln;
}

static inline ssize_t mt_write(tm_t *tm, uint8_t *rec, size_t len)
{
uint32_t meta = len;
ssize_t pln = (len + 1) & ~1;
uint8_t pad = 0;
/* ssize_t rc; */

  if(tm->md == wa)
    tm->md = wr;

  if(tm->md != wr)
    return MT_ERR;

  /* rc = */ write(tm->fd, &meta, sizeof(meta));

  if(len == 0)
    return MT_WTM;

  /* rc = */ write(tm->fd, rec, len);

  if(pln > len)
    /* rc = */ write(tm->fd, &pad, sizeof(pad));

  /* rc = */ write(tm->fd, &meta, sizeof(meta));

  return len;
}

static inline ssize_t mt_fsf(tm_t *tm)
{
ssize_t rc;

  do {
    rc = mt_read(tm, NULL, 0);
  } while(rc > 0);

  return rc;
}

static inline ssize_t mt_bsf(tm_t *tm)
{
ssize_t rc;

  do {
    rc = mt_rdbk(tm, NULL, 0);
  } while(rc > 0);

  return rc;
}

static inline int mt_close(tm_t *tm)
{
//uint32_t meta = _MT_EOM;
//ssize_t rc;

  if(tm->fd < 0)
    return 0;

//if(tm->md == wr)
//{
//  rc = write(tm->fd, &meta, sizeof(meta));
//  off_t pos = lseek(tm->fd, 0, SEEK_CUR);
//  ftruncate(tm->fd, pos);
//}

  tm->md = cl;
  close(tm->fd);
  tm->fd = -1;

  return 0;
}

static inline int mt_open(tm_t *tm)
{
  mt_close(tm);
  
  tm->md = rd;
  tm->fd = open(tm->fn, O_RDWR | O_CLOEXEC);

  if(tm->fd == -1)
    tm->fd = open(tm->fn, O_RDONLY | O_CLOEXEC);
  else
    tm->md = wa;

  if(tm->fd == -1)
    tm->md = cl;

  return tm->fd;
}

static inline int mt_stat(tm_t *tm)
{
  if(tm->fd == -1)
    return MT_OFL;

  off_t pos = lseek(tm->fd, 0, SEEK_CUR);

  if(pos <= sizeof(uint32_t))
    return MT_BOT;

  struct stat st;
  fstat(tm->fd, &st);

  if(pos == st.st_size && tm->max > 0 && pos >= tm->max)
    return MT_EOM;

  return MT_ONL;
}

#endif
