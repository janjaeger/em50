/* Branch Instructions
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


/* BDX
 * Branch on Decremented X
 * 1100000111011100 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(bdx)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bdx", ea);

  uint16_t x = G_X(cpu);

#if defined(IDLE_WAIT)
  if(cpu->bp == ea && cpu->crs->km.ie && (cpu->po & ea_r) == 0)
  {
    if(io_idle_wait(cpu, x))
    {
      S_X(cpu, 0);
      longjmp(cpu->endop, endop_intrchk);
    }

    x = 1;
  }
#endif

  S_X(cpu, --x);

  if(x != 0)
    cpu->p = ea;


}
#endif


/* BIX
 * Branch on Incremented X
 * 1100001011011100 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(bix)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bix", ea);

  uint16_t x = G_X(cpu);

  S_X(cpu, ++x);

  if(x != 0)
    cpu->p = ea;
}
#endif


/* BDY
 * Branch on Decremented Y
 * 1100000111010100 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(bdy)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bdy", ea);

  uint16_t y = G_Y(cpu);

  S_Y(cpu, --y);

  if(y != 0)
    cpu->p = ea;
}
#endif


/* BIY
 * Branch on Incremented Y
 * 1100001011010100 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(biy)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "biy", ea);

  uint16_t y = G_Y(cpu);

  S_Y(cpu, ++y);

  if(y != 0)
    cpu->p = ea;
}
#endif


/* BEQ
 * Branch on A Equal to 0
 * 1100000110001010 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(beq)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "beq", ea);

  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_EQ(cpu))
    cpu->p = ea;
}
#endif


E50I(bne)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bne", ea);

  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_NE(cpu))
    cpu->p = ea;
}


/* BGE
 * Branch on A Greater Then or Equal to 0
 * 1100000110001101 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(bge)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bge", ea);

  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_GE(cpu))
    cpu->p = ea;
}
#endif


/* BGT
 * Branch on A Greater Then 0
 * 1100000110001001 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(bgt)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bgt", ea);

  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_GT(cpu))
    cpu->p = ea;
}
#endif


/* BLE
 * Branch on A Less Then or Equal to 0
 * 1100000110001000 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(ble)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "ble", ea);

  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_LE(cpu))
    cpu->p = ea;
}
#endif


/* BLT
 * Branch on A Less Then 0
 * 1100000110001100 ADDRESS\16 (V mode form)
 */
#if defined V_MODE || defined R_MODE
E50I(blt)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "blt", ea);

  int16_t a = (int16_t)G_A(cpu);

  SET_CC(cpu, a);

  if(CC_LT(cpu))
    cpu->p = ea;
}
#endif


/* BCEQ
 * Branch on Condition Code EQ
 * 1100001110000010 ADDRESS\16 (V mode form) 
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bceq)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bceq", ea);

  if(CC_EQ(cpu))
    cpu->p = ea;
}
#endif


/* BCGE
 * Branch on Condition Code GE
 * 1100001110000101 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bcge)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bcge", ea);

  if(CC_GE(cpu))
    cpu->p = ea;
}
#endif


/* BCGT
 * Branch on Condition Code GT
 * 1100001110000001 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bcgt)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bcgt", ea);

  if(CC_GT(cpu))
    cpu->p = ea;
}
#endif


/* BCLE
 * Branch on Condition Code LE
 * 1100001110000000 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bcle)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bcle", ea);

  if(CC_LE(cpu))
    cpu->p = ea;
}
#endif


/* BCLT
 * Branch on Condition Code LT
 * 1100001110000100 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bclt)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bclt", ea);

  if(CC_LT(cpu))
    cpu->p = ea;
}
#endif


/* BCNE
 * Branch on Condition Code NE
 * 1100001110000011 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bcne)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bcne", ea);

  if(CC_NE(cpu))
    cpu->p = ea;
}
#endif


/* BCR
 * Branch on CBIT Reset to 0
 * 1100001111000101 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bcr)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bcr", ea);

  if(!cpu->crs->km.cbit)
    cpu->p = ea;
}
#endif


/* BCS
 * Branch on CBIT Set to 1
 * 1100001111000100 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bcs)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bcr", ea);

  if(cpu->crs->km.cbit)
    cpu->p = ea;
}
#endif


/* BLEQ
 * Branch on L Equal To 0
 * 1100000111000010 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(bleq)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bleq", ea);

  int32_t l = (int32_t)G_L(cpu);

  SET_CC(cpu, l);

  if(CC_EQ(cpu))
    cpu->p = ea;
}
#endif


/* BLNE
 * Branch on L Not Equal To 0
 * 1100000111000011 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(blne)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "blne", ea);

  int32_t l = (int32_t)G_L(cpu);

  SET_CC(cpu, l);

  if(CC_NE(cpu))
    cpu->p = ea;
}
#endif


/* BLLE
 * Branch on L Less Then or Equal To 0
 * 1100000111000000 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(blle)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "blle", ea);

  int32_t l = (int32_t)G_L(cpu);

  SET_CC(cpu, l);

  if(CC_LE(cpu))
    cpu->p = ea;
}
#endif


/* BLGT
 * Branch on L Greater Then 0
 * 1100000111000001 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE
E50I(blgt)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "blgt", ea);

  int32_t l = (int32_t)G_L(cpu);

  SET_CC(cpu, l);

  if(CC_GT(cpu))
    cpu->p = ea;
}
#endif


/* BLGE
 * Branch on L Greater Then or Equal to 0
 * 1100000110001101 ADDRESS\16 (V mode form)
 */
#if 0 // Funtionally equal to BGE
E50I(blge)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "blge", ea);

  int32_t l = (int32_t)G_L(cpu);

  SET_CC(cpu, l);

  if(CC_GE(cpu))
    cpu->p = ea;
}
#endif


/* BLLT
 * Branch on L Less Then 0
 * 1100000110001100 ADDRESS\16 (V mode form)
 */
#if 0 // Funtionally equal to BLT
E50I(bllt)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bllt", ea);

  int32_t l = (int32_t)G_L(cpu);

  SET_CC(cpu, l);

  if(CC_LT(cpu))
    cpu->p = ea;
}
#endif



/* BLR
 * Branch on Link Reset to 0
 * BLR  1100001111000111 ADDRESS\16 (V mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(blr)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "blr", ea);

  if(!cpu->crs->km.link)
    cpu->p = ea;
}
#endif


/* BLS
 * Branch on Link Set to 0
 * 1100001111000110 ADDRESS\16 (V  mode form)
 */
#if defined R_MODE || defined V_MODE || defined I_MODE
E50I(bls)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bls", ea);

  if(cpu->crs->km.link)
    cpu->p = ea;
}
#endif


E50I(bmeq)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bmeq", ea);

  if(CC_EQ(cpu))
    cpu->p = ea;
}


E50I(bmge)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bmge", ea);

  if(cpu->crs->km.link)
    cpu->p = ea;
}


E50I(bmgt)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bmgt", ea);

  if(cpu->crs->km.link && CC_NE(cpu))
    cpu->p = ea;
}


E50I(bmle)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bmle", ea);

  if((!cpu->crs->km.link) || CC_EQ(cpu))
    cpu->p = ea;
}


E50I(bmlt)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bmlt", ea);

  if(!(cpu->crs->km.link) || CC_GE(cpu))
    cpu->p = ea;
}


E50I(bmne)
{
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o(op, "bmne", ea);

  if(CC_NE(cpu))
    cpu->p = ea;
}


E50I(cgt)
{
uint16_t n = E50X(vfetch_w)(cpu, cpu->pb);
#if defined I_MODE
uint16_t r = G_RH(cpu, op_dr(op));
#else
uint16_t r = G_A(cpu);
#endif

  logop2o(op, "cgt", n);

  if(r > 0 && r < n)
    cpu->p = E50X(vfetch_w)(cpu, cpu->pb + r);
  else
    cpu->p += n;
}


#ifdef I_MODE
E50I(bheq)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bheq", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_EQ(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhge)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhge", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_GE(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhle)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhle", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_LE(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhlt)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhlt", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_LT(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhne)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhne", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_NE(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhgt)
{
int dr = op_dr(op);
int16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhgt", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_GT(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(breq)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "breq", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_EQ(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brge)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brge", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_GE(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brle)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brle", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_LE(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brlt)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brlt", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_LT(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brne)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brne", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_NE(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brgt)
{
int dr = op_dr(op);
int32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brgt", dr, r, ea);

  SET_CC(cpu, r);

  if(CC_GT(cpu))
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brbr)
{
int dr = op_dr(op);
int bit = op[1] & 0b11111;
uint32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brbr", dr, r, ea);

  if(((0x80000000UL >> bit) & r) == 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brbs)
{
int dr = op_dr(op);
int bit = op[1] & 0b11111;
uint32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brbs", dr, r, ea);

  if(((0x80000000UL >> bit) & r) != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhi1)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhi1", dr, r, ea);

  ++r;

  S_RH(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhi2)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhi2", dr, r, ea);

  r += 2;

  S_RH(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhi4)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhi4", dr, r, ea);

  r += 4;

  S_RH(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bri1)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bri1", dr, r, ea);

  ++r;

  S_R(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bri2)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bri2", dr, r, ea);

  r += 2;

  S_R(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bri4)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bri4", dr, r, ea);

  r += 4;

  S_R(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhd1)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhd1", dr, r, ea);

  --r;

  S_RH(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhd2)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhd2", dr, r, ea);

  r -= 2;

  S_RH(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(bhd4)
{
int dr = op_dr(op);
uint16_t r = G_RH(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "bhd4", dr, r, ea);

  r -= 4;

  S_RH(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brd1)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brd1", dr, r, ea);

  --r;

  S_R(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brd2)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brd2", dr, r, ea);

  r -= 2;

  S_R(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifdef I_MODE
E50I(brd4)
{
int dr = op_dr(op);
uint32_t r = G_R(cpu, dr);
uint16_t ea = E50X(vfetch_iw)(cpu);

  logop2o3(op, "brd4", dr, r, ea);

  r -= 4;

  S_R(cpu, dr, r);

  if(r != 0)
    cpu->p = ea;
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
