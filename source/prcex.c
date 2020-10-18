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


#include "emu.h"

#include "mode.h"

#include "opcode.h"

#include "io.h"

#define _prcex_c
#include "prcex.h"


#if 0
#undef logmsg
#define logmsg(...) logall(__VA_ARGS__)
#endif


E50I(nfyb)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*nfyb", ap);

  E50X(rxm_check)(cpu);

  int16_t counter = E50X(vfetch_w)(cpu, ap);

  if(counter == -32768)
    E50X(semaphore_fault)(cpu, semaphore_fault_under, ap);

  --counter;

  E50X(vstore_w)(cpu, ap, counter);

  if(counter >= 0)
    E50X(pxm_notify)(cpu, ap, 1);

}


E50I(nfye)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*nfye", ap);

  E50X(rxm_check)(cpu);

  int16_t counter = E50X(vfetch_w)(cpu, ap);

  if(counter == -32768)
    E50X(semaphore_fault)(cpu, semaphore_fault_under, ap);

  --counter;

  E50X(vstore_w)(cpu, ap, counter);

  if(counter >= 0)
    E50X(pxm_notify)(cpu, ap, 0);

}


E50I(wait)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*wait", ap);

  E50X(rxm_check)(cpu);

  int16_t counter = E50X(vfetch_w)(cpu, ap);

  if(counter == 32767)
    E50X(semaphore_fault)(cpu, semaphore_fault_over, ap);

  ++counter;

  E50X(vstore_w)(cpu, ap, counter);

  if(counter > 0)
    E50X(pxm_wait)(cpu, ap);

}


E50I(inec)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*inec", ap);

  E50X(rxm_check)(cpu);

  int16_t counter = E50X(vfetch_w)(cpu, ap);

  if(counter == -32768)
    E50X(semaphore_fault)(cpu, semaphore_fault_under, ap);

  --counter;

  E50X(vstore_w)(cpu, ap, counter);

  io_clrai(cpu);

  S_RB(cpu, cpu->srf.mrf.pswpb);
  S_KEYS(cpu, cpu->srf.mrf.pswkeys.keys);

  cpu->crs->km.ie = 1;

  if(counter >= 0)
    E50X(pxm_notify)(cpu, ap, 0);
  else
    if(counter < -1)
      E50X(pxm_disp)(cpu);

  set_cpu_mode(cpu, cpu->crs->km.mode);
}


E50I(inen)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*inen", ap);

  E50X(rxm_check)(cpu);

  int16_t counter = E50X(vfetch_w)(cpu, ap);

  if(counter == -32768)
    E50X(semaphore_fault)(cpu, semaphore_fault_under, ap);

  --counter;

  E50X(vstore_w)(cpu, ap, counter);

  S_RB(cpu, cpu->srf.mrf.pswpb);
  S_KEYS(cpu, cpu->srf.mrf.pswkeys.keys);

  cpu->crs->km.ie = 1;

  if(counter >= 0)
    E50X(pxm_notify)(cpu, ap, 0);
  else
    if(counter < -1)
      E50X(pxm_disp)(cpu);

  set_cpu_mode(cpu, cpu->crs->km.mode);
}


E50I(inbc)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*inbc", ap);

  E50X(rxm_check)(cpu);

  int16_t counter = E50X(vfetch_w)(cpu, ap);

  if(counter == -32768)
    E50X(semaphore_fault)(cpu, semaphore_fault_under, ap);

  --counter;

  E50X(vstore_w)(cpu, ap, counter);

  io_clrai(cpu);

  S_RB(cpu, cpu->srf.mrf.pswpb);
  S_KEYS(cpu, cpu->srf.mrf.pswkeys.keys);

  cpu->crs->km.ie = 1;

  if(counter >= 0)
    E50X(pxm_notify)(cpu, ap, 1);

  set_cpu_mode(cpu, cpu->crs->km.mode);
}


E50I(inbn)
{
uint32_t ap = E50X(vfetch_iap)(cpu, NULL);

  logop2o(op, "*inbn", ap);

  E50X(rxm_check)(cpu);

  int16_t counter = E50X(vfetch_w)(cpu, ap);

  if(counter == -32768)
    E50X(semaphore_fault)(cpu, semaphore_fault_under, ap);

  --counter;

  E50X(vstore_w)(cpu, ap, counter);

  S_RB(cpu, cpu->srf.mrf.pswpb);
  S_KEYS(cpu, cpu->srf.mrf.pswkeys.keys);

  cpu->crs->km.ie = 1;

  if(counter >= 0)
    E50X(pxm_notify)(cpu, ap, 1);

  set_cpu_mode(cpu, cpu->crs->km.mode);
}


#ifndef EMDE
 #include __FILE__
#endif
