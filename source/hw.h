/* Hardware
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


#include "model.h"


#ifndef _hw_h
#define _hw_h

/* Addressing modes
 * V - Virtual
 * I - General Register
 * R - Relative
 * S - Sectored
 *
 * word is 2 octets
 * sector is 512 (01000) octets
 * page is 2048 (04000) octets
 * segment is 64K words
 * dtar is 1024 segments
 * 4 dtars is 512M Virtual Memory
 *
 * An address addresses a word
 *
 * data types
 * - fixed
 * - float
 * - decimal
 * - character
 * - queue
 *
 *
 * instruction formats
 *
 * I X OP DISPLACEMENT
 *
 */

#define em50_word_size     (2)        // octets
#define em50_sector_size   (0400}     // words
#define em50_page_size     (02000)    // words
#define em50_page_offm     (em50_page_size - 1)
#define em50_page_shift    (10)
#define em50_page_mask     ~(em50_page_offm)
#define em50_segment_size  (04000000) // octets
#define em50_pages_in_segment (0100)

#define em50_pgoc_size     (04000)    // octets
#define em50_pgoc_offm     (em50_pgoc_size - 1)
#define em50_pgoc_mask     ~(em50_pgoc_offm)

#define em50_maxnrf        (8)

#define RESET_PC (01000)


#define offsetin(_s, _f) (__builtin_offsetof(_s, _f) >> 1)

typedef uint8_t op_t[];

typedef struct sw_t {
  union {
    struct {
      uint16_t w;
    };
    struct {
      uint8_t l; // little endian
      uint8_t h;
    };
  };
} __attribute__ ((packed)) sw_t;

typedef struct dw_t {
  union {
    struct {
      uint32_t w;
    };
    struct {
      uint16_t l; // little endian
      uint16_t h;
    };
  };
} __attribute__ ((packed)) dw_t;

typedef struct dq_t {
  union {
    struct {
      uint64_t q;
    };
    struct {
      uint32_t l; // little endian
      uint32_t h;
    };
  };
} __attribute__ ((packed)) dq_t;

typedef struct gr_t {
  union {
    struct {
      uint32_t r;
    };
    struct {
      uint16_t l; // little endian
      uint16_t h;
    };
  };
} __attribute__ ((packed)) gr_t;

typedef struct km_t {
  union {
    struct {
    uint8_t mcm:2,        // Machine Check Mode
#define km_mcn   0b00     // No Errors
#define km_mce   0b01     // ECCU
#define km_mcu   0b10     // Unrecoverable Errors
#define km_mca   0b11     // All Errors
            sm:1,         // Segment Mode
            pxm:1,        // Process eXchange Mode
            mio:1,        // Mapped I/O Mode
            crs:3;        // Current Register Set
    uint8_t resv:4,
            dio:1,        // Disable Indirect Overlap (P750)
            dpo:1,        // Disable Prefetch Overlap (P750)
            vim:1,        // Vectored Interrupt Mode
            ie:1;         // Interrupts Enabled
    union {
      struct {
      uint8_t sd:1,       // VI: Save Done
              in:1,       // VI: Displatcher
              check:1,    // VI: Check (P850)
              rnd:1,
              ascii:1,
              dex:1,      // VI: Decimal Exception
              eq:1,       // VI: Equal
              lt:1;       // VI: Less Then
      };
      struct {
      uint8_t vsc;        // Visible Shift Count
      };
    };
    uint8_t iex:1,        // VI: Integer Exception
            fex:1,        // VI: Floating Point Exception
            mode:3,       // Mode
#define km_e16s  0b000
#define km_e32s  0b001
#define km_e64r  0b010
#define km_e32r  0b011
#define km_e32i  0b100
#define km_e101  0b101
#define km_e64v  0b110
#define km_e111  0b111
            link:1,       // VI: Link
            dp:1,         // SR: Double Precision
            cbit:1;       // SR: Arithmetic Error Cond VI: Carry
    };
    union {
      struct {
      uint16_t modals;
      uint16_t keys;
      };
      uint32_t d;
    };
  };
} __attribute__ ((packed)) km_t;


/* Microcode Scratch Area
 */
typedef uint32_t  ucs_t[7];


typedef uint32_t ea_t; // Effecvice Address
#define ea_f     0x80000000  // Fault bit
#define ea_r     0x60000000  // Ring number
#define ea_r_s   29
#define ea_e     0x10000000  // Extention bit
#define ea_d     0x0c000000  // DTAR number
#define ea_d_s   26
#define ea_s     0x0fff0000  // Segment number (12 bits)
#define ea_sd    0x03ff0000  // Segment number within DTAR (10 bits)
#define ea_s_s   16
#define ea_w     0x0000ffff  // Word number (within segment)
#define ea_p     0x0000fc00  // Page number (within segment)
#define ea_p_s   10
#define ea_o     0x000003ff  // Word number (within page)
#define ea_ring(_ea)  (((_ea) & ea_r) >> ea_r_s)
#define ea_dtar(_ea)  ((((_ea) & ea_d) >> ea_d_s) ^ 0b11)
#define ea_seg(_ea)   (((_ea) & ea_s) >> ea_s_s)
#define ea_segd(_ea)  (((_ea) & ea_sd) >> ea_s_s)
#define ea_word(_ea)  ((_ea) & ea_w)
#define ea_page(_ea)  (((_ea) & ea_p) >> ea_p_s)
#define ea_off(_ea)   ((_ea) & ea_o)
#define ea_fault(_ea) ((_ea) & ea_f)


typedef uint32_t dtar_t;
#define sdt_s    0xffc00000  // 1024 - Number of entries in SDT
#define sdt_s_s  22
#define sdt_h    0x003f0000  // High order 6 bits
#define sdt_r    0x00008000  // Reserved - must be zero
#define sdt_l    0x00007fff  // Low order 15 bits


typedef uint32_t vadr_t; // Virtual Address
typedef uint32_t phys_t; // Physical Address


/* Full Address Translation
 *
 * DTAR - Destriptor Table Adress Register
 * SDT  - Segment Descriptor Table
 * SWD  - Segment Descriptor Word
 * HMAP - Hardware page Map
 * PPN  - Physical Page Number
 *
 * sdt = urs->dtar[3-dtar]
 * hmap = sdw = sdt[segn]
 * ppn = hmap[pagn]
 * addr = ppn << 16-6 | pago
 */
#define sdw_s    0xffff0000 // Physical Address of HMAP (bits 7 to 22)
#define sdw_s_s  16
#define sdw_r    0x002f0000 // Must be zero
#define sdw_f    0x00008000 // Fault 
#define sdw_a1   0x00007000 // Ring 1 Access
#define sdw_a2   0x00000e00 // Ring 2 Access (reserved)
#define sdw_a3   0x000001c0 // Ring 3 Access 
#define sdw_h    0x0000003f // Physical Address of HMAP (bits 1 to 6)
#define sdw_h_s  16
#define sdw_a(_s) ((((_s) & sdw_s) >> sdw_s_s) | (((_s) & sdw_h) << sdw_h_s))
#define sdw_aaa(_s, _r) (((_s) >> (15 - ((_r) * 3))) & 0b111)


#define hmap_r   0x8000     // Page Resident
#define hmap_u   0x4000     // Page Used (Set to 1 by hw)
#define hmap_m   0x2000     // Page unModified (Set to 0 by hw)
#define hmap_s   0x1000     // Page Shared
#define hmap_phy 0x0fff     // Physical Page Number (sw use only)


#define pmt_r    0x80000000 // Page Resident
#define pmt_u    0x40000000 // Page Used (Set to 1 by hw)
#define pmt_m    0x20000000 // Page unModified (Set to 0 by hw)
#define pmt_s    0x10000000 // Page Shared
#define pmt_ppn  0x0fff0000 // Physical Page Number (sw use only)
#define pmt_phy  0x0000ffff // Physical Page address high order bits


#define aaa_none 0b000 // No Access
#define aaa_gate 0b001 // Gate Only Access
#define aaa_read 0b010 // Read Only Access
#define aaa_rdwr 0b011 // Read and Write Access
//      aaa_r100 0b100 // Reserved
//      aaa_r101 0b101 // Reserved 
#define aaa_rdex 0b110 // Read and Execute Access
#define aaa_all  0b111 // Read, Write and Execute Access


#define fat_r    (0b11 << 32-3)
#define fat_dtar (0b11 << 32-6)
#define fat_sdw  (0x3ff << 16)
#define fat_ppn  (0x3f << 16-6)
#define fat_poff (0b3ff)


typedef struct dma_t {
  union {
    struct {
      uint16_t addr;
      uint16_t xfer;
#define xfer_mask 0xfffc
//#define dma_chain_s 11
//#define dma_chain   (0b1111 << dma_chain_s)
//#define dma_dmc     0x0800
//#define dma_chaddr  0x07FF
    };
    uint32_t cell;
  };
} __attribute__ ((packed)) dma_t;


/* DMA Register File 
 */
typedef struct drf_t {
  union {
    struct {
/*000*/ dma_t dma[040];
    };
    struct {
/*000*/ uint16_t dma_h[0100];
    };
  };
} __attribute__ ((packed)) drf_t;

assert_size(drf_t, 040*4);


/* Microcode Register File
 */
typedef struct mrf_t {
/*000*/                  uint32_t tr0;
/*001*/                  uint32_t tr1;
/*002*/                  uint32_t tr2;
/*003*/                  uint32_t tr3;
/*004*/                  uint32_t tr4;
/*005*/                  uint32_t tr5;
/*006*/                  uint32_t tr6;
/*007*/                  uint32_t tr7;
/*010*/                  uint32_t tr8;
/*011*/                  uint32_t tr9;
/*012*/                  uint32_t tr10;
/*013*/                  uint32_t tr11;
/*014*/                  uint32_t ucsaddr;
/*015*/                  uint32_t rdsave;
/*016*/ uint16_t c00ff;  uint16_t cff00;
/*017*/ uint16_t l017;   uint16_t ratmp;
/*020*/                  uint32_t rmasave;
/*021*/                  uint32_t r021;
/*022*/                  uint32_t parreg1;
/*023*/                  uint32_t parreg2;
/*024*/                  uint32_t dswparity2;
/*025*/                  uint32_t pbsave;
/*026*/                  uint32_t sysreg1;
/*027*/                  uint32_t dswparity;
/*030*/                  uint32_t pswpb;
/*031*/                  km_t     pswkeys;
/*032*/ uint16_t pcba;   uint16_t ppa;
/*033*/ uint16_t pcbb;   uint16_t ppb;
/*034*/                  uint32_t dswrma;
/*035*/                union {
                         uint32_t dswstat;  
                         uint32_t resv:30,
                                  mc:1,
                                  ci:1;
                       };
/*036*/                  uint32_t dswpb;
/*037*/                  uint32_t rsavptr;
} __attribute__ ((packed)) mrf_t;

assert_size(mrf_t, 040*4);

typedef struct fr_t {
  union {
/*000*/ uint64_t q;
    struct {
      union {
/*000*/ uint32_t l;
        struct {
/*000*/ uint16_t ll;
/*001*/ uint16_t lh;
        };
      };
      union {
/*002*/ uint32_t a;
        struct {
/*002*/ uint16_t al;
/*003*/ uint16_t ah;
        };
      };
    };
  };
} __attribute__ ((packed)) fr_t;

/* User Register Set 
 */
typedef struct urs_t {
  union {
    struct {
/*000*/ gr_t     gr[010];
#define A gr[2].h
#define B gr[2].l
#define Y gr[5].h
#define X gr[7].h
  union {
    struct {
/*020*/ uint32_t far0;
/*022*/ uint32_t flr0;
/*024*/ uint32_t far1;
/*026*/ uint32_t flr1;
    };
    struct {
/*020*/ fr_t fr[2];
    };
  };
  union {
    struct {
/*030*/ uint32_t zb[4];
#define urs_pb 0
#define urs_sb 1
#define urs_lb 2
#define urs_xb 3
    };
    struct {
/*030*/ uint32_t pb;
/*032*/ uint32_t sb;
/*034*/ uint32_t lb;
/*036*/ uint32_t xb;
    };
  };
  union {
    struct {
/*040*/ uint32_t dtar[4];
    };
    struct {
/*040*/ uint32_t dtar3;
/*042*/ uint32_t dtar2;
/*044*/ uint32_t dtar1;
/*046*/ uint32_t dtar0;
    };
  };
/*050*/ km_t km;
  union {
    struct {
/*053*/ uint16_t ownerl;  /* PCB address */
/*052*/ uint16_t ownerh;  /* PCB segment */
    };
/*052*/ uint32_t owner;   /* PCB address */
  };
  union {
    struct {
/*055*/ uint16_t fcodel;
/*054*/ uint16_t fcodeh;
    };
/*054*/ uint32_t fcode;
  };
  union {
    struct {
/*057*/ uint16_t faddrl;
/*056*/ uint16_t faddrh;
    };
/*056*/ uint32_t faddr;
  };
/*060*/ uint32_t timer;
/*062*/ ucs_t ucs;
    };
    uint32_t r[0100/2];
    uint16_t w[0100];
  };
} __attribute__ ((packed)) urs_t;

assert_size(urs_t, 040*4);


/* System register files
 */
typedef struct srf_t {
  union {
    struct {
/*000*/  mrf_t mrf;
/*040*/  drf_t drf;
/*100*/  urs_t urs[em50_maxnrf];
         mrf_t mr2[14-em50_maxnrf]; // todo
    };
  uint32_t r[(sizeof(mrf_t) + sizeof(drf_t) + (sizeof(urs_t) * em50_maxnrf) + (sizeof(mrf_t) * (14-em50_maxnrf)))/sizeof(uint32_t)];
  };
} __attribute__ ((packed)) srf_t;

/* Stack Header
 */
typedef struct sth_t {
/*000*/ uint32_t free;
/*001*/ uint32_t next;
} __attribute__ ((packed)) sth_t;

assert_size(sth_t, 04*2);


/* Stack Framme
 */
typedef struct stk_t {
/*000*/ uint16_t flag;
/*001*/ uint16_t rseg;
/*002*/ uint32_t retp;
/*004*/ uint32_t sb;
/*006*/ uint32_t lb;
/*010*/ uint16_t keys;
/*011*/ uint16_t argo;
} __attribute__ ((packed)) stk_t;

assert_size(stk_t, 012*2);


/* Entry Control Block
 */
typedef struct ecb_t {
/*000*/ uint32_t pb;
/*002*/ uint16_t sfsize;
/*003*/ uint16_t rootsn;
/*004*/ uint16_t argd;
/*005*/ uint16_t argn;
/*006*/ uint32_t lb;
/*010*/ uint16_t keys;
/*011*/ uint16_t r011[7];
} __attribute__ ((packed)) ecb_t;

assert_size(ecb_t, 020*2);


/* Process Control Block
 */
typedef struct pcb_t {
/*000*/ uint16_t level;
/*001*/ uint16_t next;
/*002*/ uint32_t wsem;
/*004*/ uint16_t abort;
/*005*/ uint16_t last;
/*006*/ uint32_t r006;
/*010*/ uint32_t etimer;
/*012*/ uint32_t dtar2;
/*014*/ uint32_t dtar3;
  union {
    struct {
/*016*/ uint16_t itimerh;
/*017*/ uint16_t itimerl;
    };
/*016*/ uint32_t itimer;
  };
/*020*/ uint16_t smask;
/*021*/ uint16_t keys;
/*022*/ uint32_t gr[020];
/*062*/ uint32_t fault[4]; /* fault vectors for each ring */
/*072*/ uint32_t pfault;   /* page fault vector (ring 0) */
/*074*/ uint16_t csf_first;
/*075*/ uint16_t csf_next;
/*076*/ uint16_t csf_last;
/*077*/ uint16_t r077;
} __attribute__ ((packed)) pcb_t;

assert_size(pcb_t, 0100*2);


/* Concealed Stack Frame
 */
typedef struct csf_t {
/*000*/ uint32_t pc;    /* segment and offset */
/*002*/ uint16_t keys;
/*003*/ uint16_t fcode; /* bits 1 to 16 */
/*004*/ uint32_t faddr; /* segment and offset) */
} __attribute__ ((packed)) csf_t;

assert_size(csf_t, 06*2);


/* CALF Stack Framme
 */
typedef struct caf_t {
/*000*/ uint16_t flag;
/*001*/ uint16_t rseg;
/*002*/ uint32_t retp;
/*004*/ uint32_t sb;
/*006*/ uint32_t lb;
/*010*/ uint16_t keys;
/*011*/ uint16_t nsi;
/*012*/ uint16_t fcode;
/*013*/ uint32_t faddr;
/*015*/ uint16_t r015[3];
} __attribute__ ((packed)) caf_t;

assert_size(caf_t, 020*2);


#endif
