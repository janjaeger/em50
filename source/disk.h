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


#ifndef _disk_h
#define _disk_h

int disk_io(cpu_t *cpu, int, int, int, int, void **, int, char *[]);

#define DK_UNITS 8
#define DK_BUFFS 4096

typedef struct {
  char id[4];
#define dskhdr_id "EM50"
  uint16_t ver;
  uint16_t hdrsz;
  uint32_t heads;
  uint32_t tracks;
  uint32_t records;
  uint32_t size;
} __attribute__ ((packed)) dskhdr_t;

#define DK_ADDR_IPL (0760)

struct dk_t;
typedef struct {
  struct dk_t *dk;
  char *fn;
  int fd;
  int      formatting;
  uint32_t start;
  uint32_t heads;
  uint32_t tracks;
  uint32_t records;
  uint32_t size;
  uint32_t seek;
} dm_t;

typedef struct dk_t {
  cpu_t *cpu;
int busy;
  struct {
    pthread_t tid;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
  } pthread;
  uint16_t ctrl;
  uint16_t oar;
  uint16_t stat;
#define DK_STAT_OK      0x8000
#define DK_STAT_DMAOVR  0x4000
#define DK_STAT_CRCERR  0x1000
#define DK_STAT_PARERR  0x0800
#define DK_STAT_HDRERR  0x0400
#define DK_STAT_DPBUSY  0x0020
#define DK_STAT_WRPROT  0x0010
#define DK_STAT_SEEKING 0x0008
#define DK_STAT_SEEKERR 0x0004
#define DK_STAT_SELERR  0x0002
#define DK_STAT_UNAVAIL 0x0001
  uint16_t id;
  uint16_t cn; // Chain number
  uint16_t ca; // Channel Address
  int mhd;
  intr_t intr;
  dm_t dm[DK_UNITS];
  uint16_t bp;
  uint16_t bf[DK_BUFFS];
} dk_t;

#define DK_DHLT   0x0
#define DK_SFORM  0x2
#define DK_SSEEK  0x3
#define DK_DSEL   0x4
#define DK_SREAD  0x5
#define DK_SWRITE 0x6
#define DK_DSTALL 0x7
#define DK_DSTAT  0x9
#define DK_SSTOR  0xA
#define DK_DOAR   0xB
#define DK_SLOAD  0xC
#define DK_SDMA   0xD
#define DK_DINT   0xE
#define DK_DTRAN  0xF

#define DK_MEXIF   0b100000
#define DK_MPROT   0b010000
#define DK_MCERR   0b001000
#define DK_MSEEK   0b000100
#define DK_MDERR   0b000010
#define DK_MBUSY   0b000001

static const int recsize[16] = { 1040, 448, 512, 64, 128, 0, 0, 0, 2048, 0, 0, 0, 0, 0, 0, 0};

static const struct {
  const char *name;
  const int  heads;
  const int  tracks;
  const int  records;
} disk_type[] = {
/*  60 */  { "MODEL_4711",  4, 1020    ,   7 },
/*  68 */  { "68MB",        3, 1119 + 2,   9 },
/*  84 */  { "MODEL_4714",  5, 1015 + 2,   8 },
/*  96 */  { "CMD",         6,  823    ,   9 },
/* 120 */  { "MODEL_4715",  8, 1020    ,   7 },
/* 158 */  { "158MB",       7, 1119 + 2,   9 },
/* 160 */  { "160MB",      10,  821 + 2,   9 },
/* 258 */  { "MODEL_4719", 17, 1220 + 2,   6 },
/* 315 */  { "MODEL_4475", 19,  823    ,   9 },
/* 328 */  { "MODEL_4721", 12, 1641    ,   8 },
/* 496 */  { "MODEL_4735", 24,  711 + 2,  14 },
/* 675 */  { "600MB",      40,  841 + 2,   9 },
/* 770 */  { "MODEL_4845", 23,  848 + 2,  19 },
/* 817 */  { "MODEL_4860", 15, 1379 + 2,  19 },
/* 300 */  { "SMD",        19,  823    ,   9 },
/* 213 */  { "MODEL_4730", 31,   13 + 1, 254 },
/* 328 */  { "MODEL_472I", 31,   20    , 254 },
/* 421 */  { "MODEL_4731", 31,   25 + 1, 254 },
/* 637 */  { "MODEL_4729", 31,   41 + 1, 254 },
/* 1.0 */  { "MODEL_4734", 31,   64 + 1, 254 },
/* 1.3 */  { "MODEL_4732", 31,   81 + 1, 254 },
/* 2.0 */  { "MODEL_4736", 31,  121 + 1, 254 }
};

static const int disk_ntypes = sizeof(disk_type)/sizeof(*disk_type);

static inline int dk_unit(int mask)
{
static const int unit[] = { -1,  0,  1, -1,
                             2, -1, -1, -1,
                             3, -1, -1, -1,
                            -1, -1, -1, -1 };
  if((mask & 0b1111))
    return unit[mask & 0b1111];
  else
    return unit[(mask >> 4) & 0b1111] + 4;
}

static inline int dk_rdhdr(dm_t *dm)
{
dskhdr_t hdr;

  if((lseek(dm->fd, 0, SEEK_SET) != 0)
    || (read(dm->fd, &hdr, sizeof(dskhdr_t)) != sizeof(dskhdr_t)))
  {
    close(dm->fd);
    return (dm->fd = -1);
  }

  if(memcmp(hdr.id, dskhdr_id, sizeof(hdr.id)))
  {
    close(dm->fd);
    return (dm->fd = -1);
  }

  dm->start = from_be_16(hdr.hdrsz);

  dm->heads = from_be_32(hdr.heads);
  dm->tracks = from_be_32(hdr.tracks);
  dm->records = from_be_32(hdr.records);
  dm->size = from_be_32(hdr.size);

  if(dm->heads == 0
    || dm->tracks == 0
    || dm->records == 0
    || dm->size == 0)
    dm->formatting = 1;

  return dm->fd;
}

static inline int dk_wrhdr(dm_t *dm)
{
dskhdr_t hdr;

  memcpy(hdr.id, dskhdr_id, sizeof(hdr.id));
  hdr.ver = to_be_16(1);
  hdr.hdrsz = to_be_16(sizeof(dskhdr_t));

  hdr.heads = to_be_32(dm->heads);
  hdr.tracks = to_be_32(dm->tracks);
  hdr.records = to_be_32(dm->records);
  hdr.size = to_be_32(dm->size);

  if((lseek(dm->fd, 0, SEEK_SET) != 0)
    || (write(dm->fd, &hdr, sizeof(dskhdr_t)) != sizeof(dskhdr_t)))
  {
    close(dm->fd);
    dm->fd = -1;
  }

  dm->start = from_be_16(hdr.hdrsz);

  return dm->fd;
}

static inline const char *dk_fixup(dm_t *dm)
{
  if(dm->size == 0)
    dm->size = recsize[0];

  for(int n = 0; n < disk_ntypes; ++n)
  {
    if(dm->records == disk_type[n].records
     && dm->tracks == disk_type[n].tracks
     && dm->heads == disk_type[n].heads)
      return disk_type[n].name;

    if((dm->records == disk_type[n].records
     && dm->tracks == disk_type[n].tracks)
    || (dm->records == disk_type[n].records
     && dm->heads == disk_type[n].heads)
    || (dm->tracks == disk_type[n].tracks
     && dm->heads == disk_type[n].heads))
    {
      if(dm->records < disk_type[n].records)
        dm->records = disk_type[n].records;

      if(dm->tracks < disk_type[n].tracks)
        dm->tracks = disk_type[n].tracks;

      if(dm->heads < disk_type[n].heads)
        dm->heads = disk_type[n].heads;

      dk_wrhdr(dm);
      return disk_type[n].name;
    }
  }

  return NULL;
}

static inline int dk_creat(dm_t *dm)
{
  if((dm->fd = open(dm->fn, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR)) == -1)
  {
    perror(dm->fn);
    return -1;
  }

  return dk_wrhdr(dm);;
}

static inline int dk_open(dm_t *dm)
{
  if((dm->fd = open(dm->fn, O_RDWR | O_CLOEXEC)) == -1)
    if((dm->fd = dk_creat(dm)) == -1)
      return -1;

  return dk_rdhdr(dm);
}

static inline int dk_close(dm_t *dm)
{
  if(dm->fd < 0)
    return 0;

  close(dm->fd);
  dm->fd = -1;

  return -1;
}

static inline int dk_crnew(dm_t *dm, const char *type, int size)
{
  for(int n = 0; n < disk_ntypes; ++n)
    if(!strcasecmp(disk_type[n].name, type))
    {
      dm->heads = disk_type[n].heads;
      dm->tracks = disk_type[n].tracks;
      dm->records = disk_type[n].records;
      dm->size = recsize[size & 0xf];
      if(dk_creat(dm) < 0)
        return -1;
      dk_close(dm);
      return 0;
    }

  fprintf(stderr, "disk type invalid: %s\n", type);
  return -1;
}

static inline off_t dk_off(dm_t *dm, uint32_t size, uint32_t head, uint32_t track, uint32_t record)
{
off_t o = dm->start;

  if(size != dm->size)
    return 0;

  if(head >= dm->heads)
    return 0;

  if(track >= dm->tracks)
    return 0;

  if(((record + 1) * size) > (dm->records * size))
    return 0;

  o += head * dm->tracks * dm->records * (size << 1);
  o += track * dm->records * (size << 1);
  o += record * (size << 1);

  return o;
}

static inline uint16_t dk_read(dm_t *dm, uint8_t *buf, uint32_t size, uint32_t head, uint32_t track, uint32_t record)
{
off_t offset = dk_off(dm, size, head, track, record);

  if(offset == 0)
    return DK_STAT_SEEKERR;

  if(lseek(dm->fd, offset, SEEK_SET) != offset)
    return DK_STAT_HDRERR;

  size_t rd = read(dm->fd, buf, (size << 1));

  if(rd != 0 && rd != (size << 1))
    return DK_STAT_HDRERR;

  if(rd == 0)
    memset(buf, 0x00, (size << 1));

  return DK_STAT_OK;
}

static inline uint16_t dk_write(dm_t *dm, uint8_t *buf, uint32_t size, uint32_t head, uint32_t track, uint32_t record)
{
off_t offset = dk_off(dm, size, head, track, record);

  if(offset == 0)
    return DK_STAT_SEEKERR;

  if(dm->formatting)
  {
    dm->formatting = 0;
    dk_fixup(dm);
  }

  if(lseek(dm->fd, offset, SEEK_SET) != offset)
    return DK_STAT_HDRERR;

  if(write(dm->fd, buf, (size << 1)) != (size << 1))
    return DK_STAT_HDRERR;

  return DK_STAT_OK;
}

static inline uint16_t dk_format(dm_t *dm, uint32_t size, uint32_t head, uint32_t track, uint32_t records)
{
  if(dm->formatting)
  {
    if(size > dm->size)
      dm->size = size;

    if(head >= dm->heads)
      dm->heads = head + 1;

    if(track >= dm->tracks)
      dm->tracks = track + 1;

    if(records > dm->records)
      dm->records = records;

    dk_wrhdr(dm);
  }
  else
  {
    if(size > dm->size 
      || track >= dm->tracks
      || records > dm->records)
      return DK_STAT_SEEKERR;

    if(head >= dm->heads)
      dm->heads = head + 1;

    dk_wrhdr(dm);
    dk_fixup(dm);
  }

  off_t offset = dk_off(dm, size, head, track, records - 1) + (size << 1);

  if(lseek(dm->fd, offset, SEEK_SET) != offset)
    return DK_STAT_SEEKERR;

  return DK_STAT_OK;
}

#endif
