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


#ifndef _emu_h
#define _emu_h

#include "common.h"
#include "hw.h"
#include "endian.h"

#ifdef DEBUG
 #define PRINTF(...) \
 do { \
   char __b[134];  \
   int __l = snprintf(__b, sizeof(__b), __VA_ARGS__); \
   write(STDOUT_FILENO, __b, __l); \
 } while (0)
#else
 #define PRINTF(...) do {} while (0)
#endif

typedef struct sys_t {
  size_t physsize;
  uint8_t *physstor;
#define physsize_default (0x01000000) // 16Mb
#define physsize_min     (0x00040000) // 256Kb
#define physsize_max     (0x20000000) // 512Mb
  int sswitches;
  int dswitches;

  union {
  char serial[16];
  uint32_t serd[4];
  };
  uint16_t ucodeman;
  uint16_t ucodeeng;
  uint16_t ucodepln;
  uint16_t ucodeext;

  char *bind;
  char *port;

  char *pncbind;
  char *pncport;

  char *hdir;
#define hdir_default ".em50"

  char *rcfile;

  char *cpboot;

  enum { st = 0, cp, rc } tmode;

#ifdef DEBUG
  bool verbose;
  FILE *trace;
#endif

  bool cap_sys_nice;
  pthread_t tid;
} sys_t;


#define CACHE_SIZE 2048
#define CACHE_MASK (CACHE_SIZE-1)
#define CACHE_INDEX(_v, _a) ((((_v) & em50_page_mask) >> (em50_page_shift - 1) | (((_a) == acc_wr || (_a) == acc_wx) ? 1 : 0)) & CACHE_MASK)

#define IOTLB_SIZE 256
#define IOTLB_MASK (IOTLB_SIZE-1)
#define IOTLB_INDEX(_a) (((_a) >> em50_page_shift) & IOTLB_MASK)

#define INTR_QSIZE 040
#define INTR_QMASK (INTR_QSIZE-1)

#define IDLE_WAIT 10000

typedef enum {
  endop_setjmp = 0,
  endop_run    = 1,
  endop_check,
  endop_fault,
  endop_intrchk,
  endop_nointr1,
  endop_inhibit
} endop_t;


typedef enum {
  smode_setjmp = 0,
  smode_run    = 1,
  smode_setmode,
  smode_halt,
  smode_terminate
} smode_t;


struct cp_t;
struct sc_t;

typedef struct cpu_t {
  int crn;    // Current register set number
  urs_t *crs; // Current User Register Set
  union {
    uint32_t  pb;
    struct {
      uint16_t p;
      uint16_t b;
    } __attribute__ ((packed));
  };
  union {
    uint32_t  exec;
    struct {
      uint16_t ep;
      uint16_t eb;
    } __attribute__ ((packed));
  };
  union {
    uint32_t  po;
    struct {
      uint16_t bp;
      uint16_t bb;
    } __attribute__ ((packed));
  };
  uint32_t maxmem;
  int atr;
  uint64_t c;
  jmp_buf endop;
  jmp_buf smode;
  sys_t *sys;
  struct cp_t *vcp;
  struct sc_t *sc;
  srf_t srf;
  union {
    struct {
      uint16_t inst;
      union {
        uint32_t arg_ap;
        struct {
          uint16_t arg;
          uint16_t opt;
        } __attribute__ ((packed));
      };
    } __attribute__ ((packed));
    uint8_t op[6];
  };
  struct {
    uint32_t e[CACHE_SIZE];
    uint32_t r[CACHE_SIZE];
    uint32_t s[CACHE_SIZE];
    int v[CACHE_SIZE];
  } tlb;
  struct {
    uint32_t i[IOTLB_SIZE];
    int v[IOTLB_SIZE];
  } iotlb;
  struct {
    pthread_mutex_t mutex;
#if defined(IDLE_WAIT)
    pthread_cond_t cond;
#endif
    volatile int32_t v[INTR_QSIZE];
    volatile int32_t c;
    volatile int     a;
    volatile int     n;
  } intr;
#if !defined(MODEL)
  struct cpumodel_t model;
#endif
  struct {
    uint32_t pc;
    uint32_t ring;
    uint32_t faddr;
    uint32_t vector;
    int      vecoff;
    km_t     km;
    uint16_t fcode;
    int      offset;
  } fault;
  struct {
    pthread_attr_t attr;
    pthread_t tid;
  } pthread;
  struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    volatile enum { stopped = 0, stepping = 1, started = 2 } status;
  } halt;
} cpu_t;

static inline void mm_piotlb(cpu_t *cpu)
{
  memset(cpu->iotlb.v, 0, sizeof(cpu->iotlb.v));
}

static inline void mm_ptlb(cpu_t *cpu)
{
  memset(cpu->tlb.v, 0, sizeof(cpu->tlb.v));
}

static inline uint32_t em50_timer(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  uint32_t timer = ((64ULL * ts.tv_sec * 1000000ULL) + (64ULL * ts.tv_nsec / 1000ULL));

  return timer & ~1;
}

void missing_mem(cpu_t *cpu, uint32_t addr);
static inline void *physad(cpu_t *cpu, uint32_t addr)
{
//addr &= 0x07ffffff;
  if(addr >= cpu->maxmem)
    missing_mem(cpu, addr);
  return cpu->sys->physstor + (addr << 1);
}

static inline void cpu_halt_init(cpu_t *cpu)
{
  pthread_cond_init(&cpu->halt.cond, NULL);
  pthread_mutex_init(&cpu->halt.mutex, NULL);
  cpu->halt.status = stopped;
}

static inline void cpu_halt(cpu_t *cpu)
{
  pthread_mutex_lock(&cpu->halt.mutex);
  do {
    pthread_cond_wait(&cpu->halt.cond, &cpu->halt.mutex);
  } while(cpu->halt.status == stopped);
  pthread_mutex_unlock(&cpu->halt.mutex);
}

static inline void cpu_start(cpu_t *cpu)
{
  pthread_mutex_lock(&cpu->halt.mutex);
  cpu->halt.status = started;
  pthread_cond_broadcast(&cpu->halt.cond);
  pthread_mutex_unlock(&cpu->halt.mutex);
}

static inline void cpu_stop(cpu_t *cpu)
{
  pthread_mutex_lock(&cpu->halt.mutex);
  cpu->halt.status = stopped;
  pthread_cond_broadcast(&cpu->halt.cond);
  pthread_mutex_unlock(&cpu->halt.mutex);
}

static inline void cpu_step(cpu_t *cpu)
{
  pthread_mutex_lock(&cpu->halt.mutex);
  cpu->halt.status = stepping;
  pthread_cond_broadcast(&cpu->halt.cond);
  pthread_mutex_unlock(&cpu->halt.mutex);
}

static inline int cpu_started(cpu_t *cpu)
{
  return cpu->halt.status == started;
}

#ifdef DEBUG
 static pthread_mutex_t _loglock = PTHREAD_MUTEX_INITIALIZER;
 #define logall(...) \
  do { \
    if(cpu->sys->trace /* && cpu->crs->ownerl == 0x8040 && ea_ring(cpu->pb) == 3 */ ) \
    { \
      char _logline[1024]; \
      snprintf(_logline, sizeof(_logline), __VA_ARGS__); \
      pthread_mutex_lock(&_loglock); \
      fputs(_logline, cpu->sys->trace); \
      pthread_mutex_unlock(&_loglock); \
    } \
  } while (0)
#else
 #define logall(...) do {} while (0)
#endif

#ifdef DEBUG
 #define logmsg(...) \
  do { \
    if(cpu->sys->verbose) \
     logall(__VA_ARGS__); \
  } while (0)
#else
 #define logmsg(...) do {} while (0)
#endif

#ifdef DEBUG
 #define logcpu(...) \
  do \
  { \
    if(cpu->sys->verbose) \
      logall(__VA_ARGS__); \
  } while (0)
#else
 #define logcpu(...) do {} while (0)
#endif


#define fetch_w(_a) \
  (from_be_16(((uint16_t)(*(uint16_t*)(_a)))))

#define fetch_d(_a) \
  (from_be_32(((uint32_t)(*(uint32_t*)(_a)))))

#define fetch_q(_a) \
  (from_be_64(((uint64_t)(*(uint64_t*)(_a)))))

#define store_w(_a, _v) \
  (((*(uint16_t*)(_a))) = to_be_16(_v))

#define store_d(_a, _v) \
  (((*(uint32_t*)(_a))) = to_be_32(_v))

#define _store_q(_a, _v) \
  (((*(uint64_t*)(_a))) = to_be_64(_v))

static inline void store_q(uint64_t *a, uint64_t v)
{
  _store_q(a, v);
}

typedef union {
  float flt;
  struct {
    uint32_t mantissa : 23;
    uint32_t exponent : 8;
    uint32_t sign : 1;
  } __attribute__ ((packed));
  uint32_t d;
} ieee_flt;
assert_size(ieee_flt, 4);


typedef union {
  struct {
    uint32_t exponent : 8;
    uint32_t mantissa : 23;
    uint32_t sign : 1;
  } __attribute__ ((packed));
  uint32_t d;
} em50_flt;
assert_size(em50_flt, 4);


typedef union {
  double dbl;
  struct {
    uint64_t mantissa : 52;
    uint64_t exponent : 11;
    uint64_t sign : 1;
  } __attribute__ ((packed));
  uint64_t q;
} ieee_dbl;
assert_size(ieee_dbl, 8);


typedef union {
  struct {
    uint64_t exponent : 16;
    uint64_t mantissa : 47;
    uint64_t sign : 1;
  } __attribute__ ((packed));
  uint64_t q;
} em50_dbl;
assert_size(em50_dbl, 8);


#ifdef FLOAT128
typedef union {
  __float128 qad;
  struct {
    __uint128_t mantissa : 112;
    __uint128_t exponent : 15;
    __uint128_t sign : 1;
  } __attribute__ ((packed));
  __uint128_t q;
} ieee_qad;
assert_size(ieee_qad, 16);
#endif


typedef union {
  struct {
    uint64_t reserved : 16;
    uint64_t mantissa : 48;
  } __attribute__ ((packed));
  uint64_t q;
} em50_qex;
assert_size(em50_qex, 8);


typedef struct {
  union {
    struct {
      em50_dbl dbl;
      em50_qex qex;
    } __attribute__ ((packed));
#ifdef UINT128
    __uint128_t q;
#endif
  };
} __attribute__ ((packed)) em50_qad;
assert_size(em50_qad, 16);


static inline void f2d(em50_dbl *d, em50_flt *f)
{
  d->sign = f->sign;
  d->exponent = f->exponent;
  d->mantissa = (uint64_t)f->mantissa << 24;
}

static inline void d2f(em50_flt *f, em50_dbl *d)
{
  f->sign = d->sign;
  f->exponent = d->exponent;
  f->mantissa = (d->mantissa + 0x800000) >> 24;
}

static inline uint64_t _f2d(uint32_t flt)
{
em50_flt f = {.d = flt};
em50_dbl d;

  f2d(&d, &f);
  return d.q;
}

static inline uint32_t _d2f(uint64_t dbl)
{
em50_dbl d = {.q = dbl};
em50_flt f;

  d2f(&f, &d);
  return f.d;
}

#if 0
static inline uint64_t _setdac(uint64_t dac)
{
  return dac;
}

static inline uint64_t _getdac(uint64_t dac)
{
  return dac;
}
#endif


#define G_R(_u, _r)      ((_u)->crs->gr[(_r)].r)
#define G_RL(_u, _r)     ((_u)->crs->gr[(_r)].l)
#define G_RH(_u, _r)     ((_u)->crs->gr[(_r)].h)

#define G_DR(_u)         (((_u)->inst & op_DR) >> 7)

#define S_R(_u, _r, _v)  ((_u)->crs->gr[(_r)].r = (_v))
#define S_RL(_u, _r, _v) ((_u)->crs->gr[(_r)].l = (_v))
#define S_RH(_u, _r, _v) ((_u)->crs->gr[(_r)].h = (_v))


#define G_A(_u)          G_RH(_u, 2)
#define G_B(_u)          G_RL(_u, 2)
#define G_L(_u)          G_R(_u, 2)
#define G_E(_u)          G_R(_u, 3)
#define G_EH(_u)         G_RH(_u, 3)
#define G_Y(_u)          G_RH(_u, 5)
#define G_T(_u)          G_RL(_u, 5)
#define G_X(_u)          G_RH(_u, 7)

#define S_A(_u, _v)      S_RH(_u, 2, _v)
#define S_B(_u, _v)      S_RL(_u, 2, _v)
#define S_L(_u, _v)      S_R(_u, 2, _v)
#define S_E(_u, _v)      S_R(_u, 3, _v)
#define S_Y(_u, _v)      S_RH(_u, 5, _v)
#define S_T(_u, _v)      S_RL(_u, 5, _v)
#define S_X(_u, _v)      S_RH(_u, 7, _v)

#define G_SP(_c) G_Y(_c)
#define S_SP(_c, _s) S_Y(_c, _s)

#define G_ZB(_u, _i)     ((_i) ? ((_u)->crs->zb[(_i)]) : G_PB(_u))

#define S_ZB(_u, _i, _v) ((_u)->crs->zb[(_i)] = (_v))

#define G_P(_u)          ((_u)->crs->pb)
#define G_PB(_u)         (G_P(_u) & 0xffff0000)
#define G_SB(_u)         ((_u)->crs->sb)
#define G_LB(_u)         ((_u)->crs->lb)
#define G_XB(_u)         ((_u)->crs->xb)

#define S_P(_u, _v)      ((_u)->crs->pb = (_v))
#define S_RB(_u, _v)     S_P((_u), ((_u)->pb = (_v)) & 0xffff0000)
#define S_PB(_u, _v)     S_RB((_u), (((_u)->pb & ea_r) | (_v)))
#define S_SB(_u, _v)     ((_u)->crs->sb = (_v))
#define S_LB(_u, _v)     ((_u)->crs->lb = (_v))
#define S_XB(_u, _v)     ((_u)->crs->xb = (_v))

#define G_KEYS(_u)       ((_u)->crs->km.keys)

#define S_KEYS(_u, _k) \
do {  \
  km_t _km = { .keys = (_k) }; \
  if(_km.mode == km_e101 || _km.mode == km_e111) \
    _km.mode = (_u)->crs->km.mode; \
  (_u)->crs->km.keys = _km.keys; \
} while (0)

#define G_DMA_X(_u, _c)  ((_u)->srf.drf.dma[(_c)].xfer)
#define G_DMA_0(_u, _c)  ((_u)->srf.drf.dma[(_c)].addr)
#define G_DMA_D(_u, _c)  ((_u)->srf.drf.dma[(_c)].cell)

#define S_DMA_X(_u, _c, _v) ((_u)->srf.drf.dma[(_c)].xfer = (_v))
#define S_DMA_0(_u, _c, _v) ((_u)->srf.drf.dma[(_c)].addr = (_v))
#define S_DMA_D(_u, _c, _v) ((_u)->srf.drf.dma[(_c)].cell = (_v))

#define G_DMA_A(_u, _c) (G_DMA_D(_u, _c) & 0x0003ffff)
#define G_DMA_L(_u, _c) ((uint16_t)(-((int16_t)(0xf000 | (G_DMA_X(_u, _c) >> 4)))))

#define S_DMA_A(_u, _c, _v) (S_DMA_D(_u, _c, (G_DMA_D(_u, _c) & 0xfffc0000) | ((_v) & 0x0003ffff)))
#define S_DMA_L(_u, _c, _v) (S_DMA_X(_u, _c, (G_DMA_X(_u, _c) & 0x000f) | ((-(int16_t)(_v)) << 4)))

#define FAR(_o) ((_o[1] & 0b1000) >> 3)
#define G_FAR(_u, _n) ((_u)->crs->fr[(_n)].h)
#define G_FXR(_u, _n) (((_u)->crs->fr[(_n)].l << 16) | ((_u)->crs->fr[(_n)].l >> 16))
#define G_FLR(_u, _n) (G_FXR(_u,_n) & 0x001fffff)
#define G_FBR(_u, _n) (G_FXR(_u,_n) >> 28) 

#define S_FAR(_u, _n, _v) ((_u)->crs->fr[(_n)].h = (_v))
#define S_FXR(_u, _n, _v) ((_u)->crs->fr[(_n)].l = (((_v) << 16) | ((_v) >> 16)))
#define S_FLR(_u, _n, _v) (S_FXR((_u), (_n), ((G_FXR(_u, _n) & 0xf0000000) | ((_v) & 0x001fffff))))
#define S_FBR(_u, _n, _v) (S_FXR((_u), (_n), (G_FLR(_u, _n) | ((_v) << 28))))

#define G_PCB(_u)       ((_u)->owner)
#define S_FCODE(_u, _v) ((_u)->fcode = (_v))
#define S_FADDR(_u, _v) ((_u)->fcode = (_v))

#define G_DTAR(_c, _d) ((_c)->crs->dtar[(_d)])
#define G_DTAR_L(_c, _d) (1024 - ((((_c)->crs->dtar[(_d)]) & sdt_s) >> sdt_s_s))
static inline uint32_t sdt_r_elim(uint32_t u) { return ((u & sdt_h) >> 1) | (u & sdt_l); }
#define G_DTAR_A(_c, _d) (sdt_r_elim((_c)->crs->dtar[(_d)]) << 1)

#define FAC(_o) ((_o[0] & 0b1))
#define IFAC(_o) ((_o[1] & 0b1000) >> 3)
#define G_AC(_c, _f)     ((_c)->crs->fr[(_f)].q)
#define S_AC(_c, _f, _v) ((_c)->crs->fr[(_f)].q = (_v))

#define G_DAC(_c, _a)       G_AC(_c, _a)
#define S_DAC(_c, _a, _v)   S_AC(_c, _a, _v)

#define G_FAC(_c, _a)   _d2f(G_AC(_c, _a))
#define S_FAC(_c, _a, _v) S_AC(_c, _a, _f2d(_v))

#define G_FACH(_c)      ((_c)->crs->fr[1].hh)
#define G_FACL(_c)      ((_c)->crs->fr[1].hl)
#define G_FACE(_c)      ((_c)->crs->fr[1].ll)

#define S_FACH(_c, _v)  ((_c)->crs->fr[1].hh = (_v))
#define S_FACL(_c, _v)  ((_c)->crs->fr[1].hl = (_v))
#define S_FACE(_c, _v)  ((_c)->crs->fr[1].ll = (_v))

#define G_VSC(_c)       (G_FACE(_c) & 0xff)
#define S_VSC(_c, _v)   S_FACE((_c), (G_FACE(_c) & 0xff00) | ((_v) & 0xff))

#define _SET_CC(_c, _r, _v) \
  do { \
    (_c)->crs->km.eq = ((_r) == (_v)); \
    (_c)->crs->km.lt = ((_r) < (_v)); \
  } while (0)

#define SET_CC(_c, _r) _SET_CC(_c, _r, 0)

#define CC_EQ(_c) ((_c)->crs->km.eq)
#define CC_LT(_c) ((_c)->crs->km.lt)
#define CC_NE(_c) (!CC_EQ(_c))
#define CC_GE(_c) (!CC_LT(_c))
#define CC_LE(_c) (CC_LT(_c) || CC_EQ(_c))
#define CC_GT(_c) ((!CC_LT(_c)) && (!CC_EQ(_c)))


static inline void __attribute__ ((noreturn)) set_cpu_mode(cpu_t *cpu, int mode)
{
  cpu->crs->km.mode = mode;
  longjmp(cpu->smode, smode_setmode);
}


static inline void dump_physstor(cpu_t *cpu, uint32_t start, uint32_t end)
{
static const uint8_t zero[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t *stor = cpu->sys->physstor;
int addr = (start << 1);
int last = end ? (end << 1) : cpu->sys->physsize;
int skip = 0;

  do {
    if(memcmp(stor + addr, zero, sizeof(zero)))
    {
      skip = 0;
      logall("%8.8x:%8.8x", addr, addr >> 1);
      for(int n = 0; n < 16; ++n)
        logall("%s%2.2x", n & 1 ? "" : " ", stor[addr+n]);
      logall("  ");
      for(int n = 0; n < 16; ++n)
        logall("%c", isprint(stor[addr+n] & 0x7f) ? stor[addr+n] & 0x7f : '.');
      logall("\n");
    }
    else
    {
      if(!skip)
        logall("%8.8x:%8.8x zeros...\n", addr, addr >> 1);
      skip = 1;
    }
    addr += 16;
  } while(addr < last);

}

typedef void (*inst_t)(cpu_t *, op_t);

typedef int (*ioop_t)(cpu_t *, int, int, int, int, void **, int, char *[]);

int em50_init(cpu_t *);


#endif
