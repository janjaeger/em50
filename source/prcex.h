/* Program Exchange Instructions
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



#ifndef _prcex_h
#define _prcex_h


#ifdef DEBUG
 #if 0
  #define PRINTK(...) do { PRINTF(__VA_ARGS__); logall(__VA_ARGS__); } while (0)
 #else
  #define PRINTK(...) logall(__VA_ARGS__)
 #endif
#else
 #define PRINTK(...) do {} while (0)
#endif


#if !defined(MODEL)
 #define em50_nrf (cpu->model.nrf)
#endif

static inline int loc2crn(cpu_t *cpu, uint16_t loc)
{
  return (((loc-0x40) >> 5) & (em50_nrf -1));
}

static inline void set_crs(cpu_t *cpu, int nrs)
{
  uint32_t timer = em50_timer();

  uint16_t modals = cpu->crs->km.modals;

  cpu->crs->timer += timer;

  cpu->crn = (nrs & (em50_nrf - 1));

  cpu->crs->pb = cpu->pb;
  cpu->crs = &cpu->srf.urs[cpu->crn];
  cpu->pb = cpu->crs->pb;

  cpu->crs->timer -= timer;

  cpu->crs->km.modals = modals;

  cpu->crs->km.crs = cpu->crn;
}

#endif


static inline uint16_t E50X(vfetch_wp)(cpu_t *cpu, uint32_t addr)
{
  return E50X(vfetch_wx)(cpu, addr, acc_nn);
}

static inline uint32_t E50X(vfetch_dp)(cpu_t *cpu, uint32_t addr)
{
  return E50X(vfetch_dx)(cpu, addr, acc_nn);
}

static inline void E50X(vstore_wp)(cpu_t *cpu, uint32_t addr, uint16_t val)
{
  E50X(vstore_wx)(cpu, addr, val, acc_nn);
}

static inline void E50X(vstore_dp)(cpu_t *cpu, uint32_t addr, uint32_t val)
{
  E50X(vstore_dx)(cpu, addr, val, acc_nn);
}


static inline void E50X(timer_set)(cpu_t *cpu, uint32_t value)
{
  uint32_t timer = value - em50_timer();
  timer &= ~1;
  timer |= value >> 31;
  cpu->crs->timer = timer;
  uint32_t owner = cpu->crs->owner;
  if(cpu->crs->km.pxm && owner)
    E50X(vstore_dp)(cpu, owner + offsetin(pcb_t, itimer), value);
}

static inline uint32_t E50X(timer_get)(cpu_t *cpu)
{
  uint32_t timer = cpu->crs->timer + em50_timer();
  timer &= ~1;
  timer |= timer >> 31;
  uint32_t owner = cpu->crs->owner;
  if(cpu->crs->km.pxm && owner)
  {
    uint32_t otimer = E50X(vfetch_dp)(cpu, owner + offsetin(pcb_t, itimer));
    if((1 & (otimer ^ timer)))
      E50X(timer_set)(cpu, timer);
    if((otimer & 1) && !(timer & 1))
    {
      uint16_t abrt = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, abort));
      abrt |= 0x0001;
      E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, abort), abrt);
    }
  }
  return timer;
}


static inline void E50X(store_rs)(cpu_t *cpu, int loc, uint32_t value)
{
  if(loc & 0x4000)
  {
  switch(loc) {
    case 0x4058:
    case 0x4078:
    case 0x4098:
    case 0x40b8:
      if(loc2crn(cpu, loc) == cpu->crn)
      {
        E50X(timer_set)(cpu, value);
        break;
      }
    default:
      cpu->srf.r[loc & 0777] = value;
    }
  }
  else if(loc & 0x0010)
  {
    switch(loc) {
      case 0x18:
        E50X(timer_set)(cpu, value);
        break;
      default:
        cpu->crs->r[020 + (loc & 017)] = value;
    }
  }
  else
  {
    switch(loc) {
      case 0x08:
      case 0x09:
      case 0x0A:
      case 0x0B:
        loc ^= 1;
      default:
        cpu->crs->r[loc & 017] = value;
    }
  }
}
  
  
static inline uint32_t E50X(fetch_rs)(cpu_t *cpu, int loc)
{
  if(loc & 0x4000)
  {
    switch(loc) {
      case 0x4058:
      case 0x4078:
      case 0x4098:
      case 0x40b8:
        if(loc2crn(cpu, loc) == cpu->crn)
          return E50X(timer_get)(cpu);
      default:
        return cpu->srf.r[loc & 0777];
    }
  }
  else if(loc & 0x0010)
  {
    switch(loc) {
      case 0x0C:
        return cpu->crs->pb & 0xffff0000;
      case 0x18:
        return E50X(timer_get)(cpu);
      case 0x08:
      case 0x09:
      case 0x0A:
      case 0x0B:
        loc ^= 1;
      default:
        return cpu->crs->r[020 + (loc & 017)];
    }
  }
  else
  {
    switch(loc) {
      case 0x0C:
        return cpu->crs->pb & 0xffff0000;
      case 0x08:
      case 0x09:
      case 0x0A:
      case 0x0B:
        loc ^= 1;
      default:
        return cpu->crs->r[loc & 017];
    }
  }
}


static inline uint32_t E50X(csf_push)(cpu_t *cpu)
{
uint32_t owner = cpu->crs->owner;
uint16_t csf_seg = owner >> 16;

#if !defined(MODEL)
  if(cpu->model.cs_high)
#endif
#if !defined(MODEL) || em50_cs_high
    ++csf_seg;
#endif

  uint16_t next = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, csf_next));
  uint32_t csfa = (csf_seg << 16) | next;
  uint16_t last = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, csf_last));

  if(next == last)
  {
    uint16_t first = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, csf_first));
    E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, csf_next), first);
  }
  else
  {
    next += 6;
    E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, csf_next), next);
  }

logall("-> push csf %8.8x\n", csfa);
  return csfa;
}


static inline uint32_t E50X(csf_pop)(cpu_t *cpu)
{
uint32_t owner = cpu->crs->owner;
uint16_t csf_seg = owner >> 16;

#if !defined(MODEL)
  if(cpu->model.cs_high)
#endif
#if !defined(MODEL) || em50_cs_high
    ++csf_seg;
#endif
  
  uint16_t next = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, csf_next));
  uint16_t first = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, csf_first));

  if(next == first)
  {
    uint16_t last = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, csf_last));
    next = last;
    E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, csf_next), next);
  }
  else
  {
    next -= 6;
    E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, csf_next), next);
  }

  uint32_t csfa = (csf_seg << 16) | next;

logall("-> pop csf %8.8x\n", csfa);
  return csfa;
}


#if defined HMDE


static inline void pxm_intrchk(cpu_t *cpu, uint32_t v)
{   
  cpu->srf.mrf.pswpb   = cpu->pb;
  cpu->srf.mrf.pswkeys = cpu->crs->km;
  cpu->crs->km.ie = 0;
    
  S_RB(cpu, 0x00040000 | v);
  S_KEYS(cpu, 014000);
  set_cpu_mode(cpu, cpu->crs->km.mode);
  
  longjmp(cpu->endop, endop_nointr5); // FIXME TODO CHECK
}


static inline void __attribute__ ((noreturn)) pxm_fault(cpu_t *cpu)
{
  uint32_t owner  = cpu->crs->owner;
  uint32_t vector = E50X(vfetch_dp)(cpu, owner + cpu->fault.vector);
  uint32_t csfa   = E50X(csf_push)(cpu);
  E50X(vstore_dp)(cpu, csfa + offsetin(csf_t, pc),    cpu->fault.pc);
  E50X(vstore_wp)(cpu, csfa + offsetin(csf_t, keys),  cpu->fault.km.keys);
  E50X(vstore_wp)(cpu, csfa + offsetin(csf_t, fcode), cpu->fault.fcode);
  E50X(vstore_dp)(cpu, csfa + offsetin(csf_t, faddr), cpu->fault.faddr);

  S_RB(cpu, (vector + cpu->fault.vecoff) | cpu->fault.ring);

logall("-> %s fault offset %2.2x vector %2.2x/%2.2x pc %8.8x keys %4.4x fcodeh %4.4x faddr %8.8x pb %8.8x\n",
fault_name(cpu->fault.vecoff), cpu->fault.offset, cpu->fault.vector,cpu->fault.vecoff, cpu->fault.pc, cpu->fault.km.keys, cpu->fault.fcode, cpu->fault.faddr, cpu->pb);

  set_cpu_mode(cpu, km_e64v);

  longjmp(cpu->endop, endop_nointr5); // FIXME TODO CHECK
}

#endif
  

static inline void E50X(pxm_save)(cpu_t *cpu)
{
  uint32_t owner = cpu->crs->owner;
pcb_t *pcb = physad(cpu, E50X(v2r)(cpu, owner, acc_nn));

logmsg("pxm_save crs %d pcb %8.8x, level %4.4x/%4.4x\n", cpu->crs->km.crs, owner, cpu->srf.mrf.ppa, from_be_16(pcb->level));

  pcb->last = to_be_16(cpu->crs->km.crs << 5);
//E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, last), cpu->crs->km.crs << 5);
  
  S_P(cpu, cpu->pb);

  uint16_t smask = 0;
  for(uint16_t m = 0x8000, r = 0, s = 0; r < 020; m >>= 1, ++r)
  {
    int e; // endianess corrected r
    switch(r) {
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
        e = r ^ 1;
        break;
      default:
        e = r;
    }
    if(cpu->crs->r[e])
    {
      pcb->gr[s] = to_be_32(cpu->crs->r[e]);
//E50X(vstore_dp)(cpu, owner + offsetin(pcb_t, gr) + (e << 1), cpu->crs->r[e]);
      smask |= m;
      ++s;
    }
  }
  pcb->smask = to_be_16(smask);
//E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, smask), smask);
  S_RB(cpu, cpu->pb); // Reset low order pb in crs

//PRINTK("keys %6.6o\n", cpu->crs->km.keys);
  cpu->crs->km.sd = 1;
  pcb->keys = to_be_16(G_KEYS(cpu));
//E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, keys), cpu->crs->km.keys);
}


static inline void E50X(pxm_rest)(cpu_t *cpu, uint32_t pcba, uint16_t level)
{
logmsg("pxm_rest crs %d %4.4x\n", cpu->crs->km.crs, pcba);
pcb_t *pcb = physad(cpu, E50X(v2r)(cpu, pcba, acc_nn));

  cpu->crs->owner = pcba;
  cpu->crs->dtar[3-2] = from_be_32(pcb->dtar2);
  cpu->crs->dtar[3-3] = from_be_32(pcb->dtar3);
  E50X(timer_set)(cpu, from_be_32(pcb->itimer));
  S_KEYS(cpu, from_be_16(pcb->keys));

PRINTK("pxm_rest pcb %8.8x, level %4.4x smask %4.4x\n", pcba, cpu->srf.mrf.ppa, from_be_16(pcb->smask));

  for(uint16_t m = 0x8000, r = 0, s = 0, smask = from_be_16(pcb->smask); r < 020; m >>= 1, ++r)
  {
    int e; // endianess corrected r
    switch(r) {
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
        e = r ^ 1;
        break;
      default:
        e = r;
    }
    if((m & smask))
    {
      cpu->crs->r[e] = from_be_32(pcb->gr[s]);
//    cpu->crs->r[e] = to_be_32(E50X(vfetch_dp)(cpu, pcba + offsetin(pcb_t, gr) + (e << 1)));
      ++s;
    }
    else
    {
      cpu->crs->r[e] = 0;
    }
  }
  S_RB(cpu, G_P(cpu));

  cpu->crs->km.sd = 0;
}


static inline void E50X(pxm_check)(cpu_t *cpu)
{
  uint32_t owner = cpu->crs->owner;

  uint16_t abrt = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, abort));

  if(abrt)
  {
    E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, abort), 0);
    E50X(process_fault)(cpu, abrt);
  }
}


static inline void __attribute__ ((noreturn)) E50X(pxm_disp)(cpu_t *cpu)
{
  uint16_t pcb = cpu->srf.mrf.pcba;
  uint16_t ppa = cpu->srf.mrf.ppa;
  uint16_t seg = cpu->crs->ownerh;

  if(!pcb)
  {
    uint32_t rl = (seg << 16) | ppa;

    do
    {
      pcb = E50X(vfetch_wp)(cpu, rl);
      ppa = rl & 0xffff;
      rl += 2;
    } while (!pcb);

if(pcb == 1) { PRINTK("\nEND OF READY LIST\n"); sleep(1); }

  }

PRINTK("bfore KEYS/MODALS: %6.6o %6.6o\n", cpu->crs->km.keys, cpu->crs->km.modals);
  uint32_t pcba = (seg << 16) | pcb;

  int nrs = E50X(vfetch_wp)(cpu, pcba + offsetin(pcb_t, last)) >> 5;
  nrs &= em50_nrf - 1;

PRINTK("\npcba %8.8x last %d crs %d\n", pcba, nrs, cpu->crs->km.crs);

  if(pcb == cpu->srf.urs[nrs].ownerl)
  {

PRINTK("\nreuse ppa\n");

    set_crs(cpu, nrs);
    mm_ptlb(cpu);
  }
  else
  {
    nrs = (cpu->crs->km.crs + 1) & (em50_nrf - 1);
    while(nrs != cpu->crs->km.crs && !cpu->srf.urs[nrs].km.sd) // && cpu->srf.urs[nrs].km.in)
      nrs = (nrs + 1) & (em50_nrf - 1);

    if(!cpu->srf.urs[nrs].km.sd)
    {
      nrs = (cpu->crs->km.crs + 1) & (em50_nrf - 1);

PRINTK("-> a pxm crs %d -> %d owner %8.8x -> %8.8x\n", cpu->crs->km.crs, nrs, cpu->crs->owner, pcba);

      set_crs(cpu, nrs);
      mm_ptlb(cpu);
      E50X(pxm_save)(cpu);
    }
    else
    {

PRINTK("-> b pxm crs %d -> %d owner %8.8x -> %8.8x\n", cpu->crs->km.crs, nrs, cpu->crs->owner, pcba);

      set_crs(cpu, nrs);
      mm_ptlb(cpu);
    }

    E50X(pxm_rest)(cpu, pcba, ppa);
    mm_ptlb(cpu);
    E50X(vstore_wp)(cpu, pcba + offsetin(pcb_t, last), nrs << 5);

  }

  cpu->srf.mrf.ppa = ppa;
  cpu->srf.mrf.pcba = pcb;

  cpu->crs->km.in = 0;
  cpu->crs->km.sd = 0;

  E50X(timer_get)(cpu);

PRINTK("after KEYS/MODALS: %6.6o %6.6o\n", cpu->crs->km.keys, cpu->crs->km.modals);
  E50X(pxm_check)(cpu);

  cpu->crs->km.ie = 1; // FIXME TODO UNSURE

PRINTK("after KEYS/MODALS: %6.6o %6.6o\n", cpu->crs->km.keys, cpu->crs->km.modals);


  set_cpu_mode(cpu, cpu->crs->km.mode);
}


static inline uint16_t E50X(pxm_addrdy)(cpu_t *cpu, uint16_t pcb, int tb)
{
  uint16_t seg = cpu->crs->ownerh;
  uint32_t pcba = (seg << 16) | pcb;

  uint16_t level = E50X(vfetch_wp)(cpu, pcba + offsetin(pcb_t, level));

  uint32_t rl = (seg << 16) | level;

  uint16_t bol  = E50X(vfetch_wp)(cpu, rl);
  uint16_t eol  = E50X(vfetch_wp)(cpu, rl + 1);
  uint32_t eola = (seg << 16) | eol;

//if(bol == pcb) PRINTK("\nalready on list pcb %4.4x\n",pcb);

  if(tb || bol == 0)
  {
    E50X(vstore_wp)(cpu, pcba + offsetin(pcb_t, next), bol);
    E50X(vstore_wp)(cpu, rl, pcb);
    if(eol == 0)
      E50X(vstore_wp)(cpu, rl + 1, pcb);
  }
  else
  {
    E50X(vstore_wp)(cpu, pcba + offsetin(pcb_t, next), 0);

    if(eol != 0)
      E50X(vstore_wp)(cpu, eola + offsetin(pcb_t, next), pcb);

    E50X(vstore_wp)(cpu, rl + 1, pcb);
  }

  return level;
}


static inline void E50X(pxm_remrdy)(cpu_t *cpu)
{
  uint32_t owner = cpu->crs->owner;
  uint16_t segno = owner >> 16;

  /* Save the timer */
  E50X(timer_get)(cpu);

  uint16_t lvl = cpu->srf.mrf.ppa;
  uint16_t pcb = cpu->srf.mrf.pcba;

  uint32_t rl  = (segno << 16) | lvl;
  uint16_t bol = E50X(vfetch_wp)(cpu, rl);
  uint16_t eol = E50X(vfetch_wp)(cpu, rl + 1);

  uint16_t next = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, next));

  if(pcb == bol)
  {
    E50X(vstore_wp)(cpu, rl, next);
    if(next == 0 || pcb == eol)
      E50X(vstore_wp)(cpu, rl + 1, 0);
  }
  else
  {
PRINTK("\nCURRENT PCB %4.4x LVL %4.4x NOT ON TOP OF READY LIST\n", pcb, lvl);
    uint16_t curr = bol;
    uint16_t prev;
    do {
PRINTK("-> curr %4.4x\n", curr);
      prev = curr;
      curr = E50X(vfetch_wp)(cpu, ((segno << 16) | prev) + offsetin(pcb_t, next));
    } while (curr != 0 && pcb != curr);

if(pcb != curr) PRINTK("\npcb %4.4x not found on list\n", pcb);
  
    E50X(vstore_wp)(cpu, ((segno << 16) | prev) + offsetin(pcb_t, next), next);

    if(curr == 0)
      E50X(vstore_wp)(cpu, rl + 1, prev);

  }

  if((cpu->srf.mrf.pcba = cpu->srf.mrf.pcbb))
  {
    cpu->srf.mrf.ppa = cpu->srf.mrf.ppb;
    cpu->srf.mrf.pcbb = 0;
  }

}


static inline void E50X(pxm_addwait)(cpu_t *cpu, uint32_t wl)
{
  uint32_t owner = cpu->crs->owner;
  uint16_t level = E50X(vfetch_wp)(cpu, owner + offsetin(pcb_t, level));
  uint16_t segno = owner >> 16;
  uint16_t lvln;

  uint16_t bol = E50X(vfetch_wp)(cpu, wl + 1);
  uint32_t bola = (segno << 16) | bol;

  if(bol != 0)
    lvln = E50X(vfetch_wp)(cpu, bola + offsetin(pcb_t, level));

  if(bol == 0 || level < lvln)
  {
    E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, next), bol);
    E50X(vstore_wp)(cpu, wl + 1, owner & 0xffff);
  }
  else
  {
    uint32_t prva;
    do {
      prva = bola;
      bol = E50X(vfetch_wp)(cpu, prva + offsetin(pcb_t, next));
      if(bol == 0)
        break;
      bola = (segno << 16) | bol;
      lvln = E50X(vfetch_wp)(cpu, bola + offsetin(pcb_t, level));
    } while (level >= lvln);

    E50X(vstore_wp)(cpu, owner + offsetin(pcb_t, next), bol);
    E50X(vstore_wp)(cpu, prva + offsetin(pcb_t, next), owner & 0xffff);
  }

}


static inline uint32_t E50X(pxm_remwait)(cpu_t *cpu, uint32_t wl)
{
  uint16_t pcb = E50X(vfetch_wp)(cpu, wl + 1);
if(!pcb) PRINTK("\nremwait error wl %8.8x\n", wl);
  uint16_t seg = cpu->crs->ownerh;
  uint32_t pcba = (seg << 16) | pcb;

  E50X(vstore_dp)(cpu, pcba + offsetin(pcb_t, wsem), 0);

  uint16_t next = E50X(vfetch_wp)(cpu, pcba + offsetin(pcb_t, next));
  E50X(vstore_wp)(cpu, wl + 1, next);
  E50X(vstore_wp)(cpu, pcba + offsetin(pcb_t, next), 0);

  return pcba;
}

static inline void E50X(pxm_notify)(cpu_t *cpu, uint32_t ap, int tobol)
{
  uint32_t pcba = E50X(pxm_remwait)(cpu, ap);

  uint16_t level = E50X(pxm_addrdy)(cpu, pcba & 0xffff, tobol);

  uint16_t levelc = cpu->srf.mrf.ppa;

logmsg("level current pcb %4.4x level new pcb %4.4x\n", levelc, level);

PRINTK("ntfy level %4.4x levelc %4.4x tobol %d\n",level, levelc, tobol);

  if(level < levelc || (level == levelc && tobol))
  {
    cpu->srf.mrf.pcbb = cpu->srf.mrf.pcba;
    cpu->srf.mrf.ppb = cpu->srf.mrf.ppa;
    cpu->srf.mrf.pcba = pcba;
    cpu->srf.mrf.ppa = level;
    E50X(pxm_disp)(cpu);
  }
else
{
PRINTK("nontfy\n");

    uint16_t ppb  = cpu->srf.mrf.ppb;
    if(level < ppb || (level == ppb && tobol))
    {
      cpu->srf.mrf.ppb = level;
      cpu->srf.mrf.pcbb = pcba;
      E50X(pxm_disp)(cpu);
    }
}

PRINTK("ntfy ppa %4.4x ppb %4.4x tobol %d\n", cpu->srf.mrf.ppa, cpu->srf.mrf.ppb, tobol);

}


static inline void E50X(pxm_wait)(cpu_t *cpu, uint32_t ap)
{
  uint32_t owner = cpu->crs->owner;

  /* Store semaphore address in pcb */
  E50X(vstore_dp)(cpu, owner + offsetin(pcb_t, wsem), ap);

  E50X(pxm_remrdy)(cpu);

  E50X(pxm_addwait)(cpu, ap);

  E50X(pxm_save)(cpu);

  if((cpu->srf.mrf.pcba = cpu->srf.mrf.pcbb))
    cpu->srf.mrf.ppa = cpu->srf.mrf.ppb;

  E50X(pxm_disp)(cpu);
}


E50I(nfyb);
E50I(nfye);
E50I(wait);
E50I(inec);
E50I(inen);
E50I(inbc);
E50I(inbn);