/* Opcode Decoding
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

#include "keys.h"
#include "mctl.h"
#include "intgy.h"
#include "shift.h"
#include "move.h"
#include "bran.h"
#include "skip.h"
#include "clear.h"
#include "ltsts.h"
#include "pctlj.h"
#include "logic.h"
#include "int.h"
#include "admod.h"
#include "field.h"
#include "char.h"
#include "flpt.h"
#include "queue.h"
#include "prcex.h"
#include "grr.h"
#include "deci.h"

#include "todo.h"


static E50I(003)
{
  logop1(op, "*003"); // UNKNOWN - PANEL COMMS OR DMA SYNC?
}


static E50I(uii)
{
uint32_t ea = E50X(ea)(cpu, op);

  logop1o(op, "*uii", ea);
  E50X(uii_fault)(cpu, ea);
}


static E50I(ill)
{
  logop1(op, "*ill");
  E50X(ill_fault)(cpu, 0);
}


static E50I(o00)
{
int opcde = ((op[0] & 0b00000011) << 8) | op[1];

  switch(opcde)
  {
// ALFA 000000101100F001 (V mode format)
#ifdef V_MODE
       case 0b1011000001:
       case 0b1011001001: return E50X(alfa)(cpu, op);
#endif

// ARGT 0000000110000101 (V mode form)
//#if defined V_MODE || defined I_MODE
       case 0b0110000101: return E50X(argt)(cpu, op);
//#endif

// CAI  0000000100001001 (S.R, V, I mode form)
       case 0b0100001001: return E50X(cai)(cpu, op);

// CALF 0000000111000101 AP\32 (V mode form)
       case 0b0111000101: return E50X(calf)(cpu, op);

// CEA  0000000001001001 (S R mode form)
       case 0b0001001001: return E50X(cea)(cpu, op);

// CGT  0000001011001100 INTEGER\16 ... (V mode form)
       case 0b1011001100: return E50X(cgt)(cpu, op);

// CXCS 0000001111001100 (V, I mode form)
       case 0b1111001100: return E50X(cxcs)(cpu, op);
      
// DBL  0000000000000111 (S R mode form)
       case 0b0000000111: return E50X(dbl)(cpu, op);

// E16S 0000000000001001 (S R V mode form)
       case 0b0000001001: return E50X(e16s)(cpu, op);

// E32I 0000001000001000 (S R V mode form)
       case 0b1000001000: return E50X(e32i)(cpu, op);

// E32R 0000001000001011 (S R V mode form)
       case 0b1000001011: return E50X(e32r)(cpu, op);

// E32S 0000000000001011 (S R V mode form)
       case 0b0000001011: return E50X(e32s)(cpu, op);

// E64R 0000001000001001 (S R V mode form)
       case 0b1000001001: return E50X(e64r)(cpu, op);

// E64V 0000000000001000 (S R V mode form)
       case 0b0000001000: return E50X(e64v)(cpu, op);

// EAFA 000000101100 FAR 000 AP\32 (V mode form)
       case 0b1011000000:
       case 0b1011001000: return E50X(eafa)(cpu, op);

// EMCM 0000000101000011 (S, R, V, I mode form)
       case 0b0101000011: return E50X(emcm)(cpu, op);

// ENB  0000000100000001 (S R V  -- ENBL
       case 0b0100000001: return E50X(enb)(cpu, op);

// ENBM 0000000100000000 (S R V mode form)
       case 0b0100000000: return E50X(enbm)(cpu, op);

// ENBP 0000000100000010 (S R V mode form)
       case 0b0100000010: return E50X(enbp)(cpu, op);

// ESIM 0000000100001101 (S, R, V, I mode form)
       case 0b0100001101: return E50X(esim)(cpu, op);

// EVIM 0000000100001111 (S, R, V, I mode form)
       case 0b0100001111: return E50X(evim)(cpu, op);

// HLT  0000000000000000 (R, V mode form)
       case 0b0000000000: return E50X(hlt)(cpu, op);

// IAB  0000000010000001 (S, R, V mode form)
       case 0b0010000001: return E50X(iab)(cpu, op);

// INBC 0000001010001111 AP\32 (V mode form)
       case 0b1010001111: return E50X(inbc)(cpu, op);

// INBN 0000001010001101 AP\32 (V mode form)
       case 0b1010001101: return E50X(inbn)(cpu, op);

// INEC 0000001010001110 AP\32 (V mode form)
       case 0b1010001110: return E50X(inec)(cpu, op);

// INEN 0000001010001100 AP\32 (V mode form)
       case 0b1010001100: return E50X(inen)(cpu, op);

// INH  0000001000000001 (S, R, V mode form) -- INHL
       case 0b1000000001: return E50X(inh)(cpu, op);

// INHM 0000001000000000 (S, R, V mode form)
       case 0b1000000000: return E50X(inhm)(cpu, op);

// INHP 0000001000000010 (S, R, V mode form)
       case 0b1000000010: return E50X(inhp)(cpu, op);

#if defined S_MODE || defined R_MODE || defined V_MODE
// INK  0000000000100011 (S, R mode form)
       case 0b0000100011: return E50X(ink)(cpu, op);
#endif

// IRTC 0000000110000011 (V mode form)
       case 0b0110000011: return E50X(irtc)(cpu, op);

// IRTN 0000000110000001 (V mode form)
       case 0b0110000001: return E50X(irtn)(cpu, op);

// ITLB 0000000110001101 (V mode form)
       case 0b0110001101: return E50X(itlb)(cpu, op);

#ifdef V_MODE
// LDC  000000101100 FLR 010 (V mode form)
       case 0b1011000010:
       case 0b1011001010: return E50X(ldc)(cpu, op);
#endif

// LFLI 000000101100 FLR 011 (V mode form)
       case 0b1011000011:
       case 0b1011001011: return E50X(lfli)(cpu, op);

// EPMX 0000000010011111 (000237)
       case 0b0010011111: return E50X(epmx)(cpu,op);

// LPMX 0000000010011101 (000235)
       case 0b0010011101: return E50X(lpmx)(cpu,op);

// EVMX 0000000111010011 (000723)
       case 0b0111010011: return E50X(evmx)(cpu,op);

// ERMX 0000000111010001 (000721)
       case 0b0111010001: return E50X(ermx)(cpu,op);

// EPMJ 0000000010001111 (000217)
       case 0b0010001111: return E50X(epmj)(cpu,op);

// LPMJ 0000000010001101 (000215)
       case 0b0010001101: return E50X(lpmj)(cpu,op);

// EVMJ 0000000111000011 (000703)
       case 0b0111000011: return E50X(evmj)(cpu,op);

// ERMJ 0000000111000001 (000701)
       case 0b0111000001: return E50X(ermj)(cpu,op);

#if defined S_MODE || defined R_MODE
// ISI  0000000101001001 (000511)
       case 0b0101001001: return E50X(isi)(cpu,op);

// OSI  0000000101001101 (000515)
       case 0b0101001101: return E50X(osi)(cpu,op);
#endif

// LMCM 0000000101000001 (S, R, V, I mode form)
       case 0b0101000001: return E50X(lmcm)(cpu, op);

// LMCS 0000001111001000 (V, I mode form)
       case 0b1111001000: return E50X(lwcs)(cpu, op);

// MDEI 0000001011000100 (V, I mode form)
       case 0b1011000100: return E50X(mdei)(cpu, op);

// MDII 0000001011000101 (V, I mode form)
       case 0b1011000101: return E50X(mdii)(cpu, op);

// MDIW 0000001011010100 (V, I mode form)
       case 0b1011010100: return E50X(mdiw)(cpu, op);

// MDRS 0000001011000110 (V, I mode form)
       case 0b1011000110: return E50X(mdrs)(cpu, op);

// MDWC 0000001011000111 (V, I mode form)
       case 0b1011000111: return E50X(mdwc)(cpu, op);

// NRM  0000000001000001 (S, R mode form)
       case 0b0001000001: return E50X(nrm)(cpu, op);

// LIOT 0000000000100100 AP\32 (V mode form)
       case 0b0000100100: return E50X(liot)(cpu, op);

// LPID 0000000110001111 (V mode form)
       case 0b0110001111: return E50X(lpid)(cpu, op);

// LPSW 0000000111001001 AP\32 (V mode form)
       case 0b0111001001: return E50X(lpsw)(cpu, op);

// NFYB 0000001010001001 AP\32 (V mode form)
       case 0b1010001001: return E50X(nfyb)(cpu, op);

// NFYE 0000001010001000 AP\32 (V mode form)
       case 0b1010001000: return E50X(nfye)(cpu, op);

// NOP  0000000000000001 (S, R, V mode form)
       case 0b0000000001: return E50X(nop)(cpu, op);

#if defined S_MODE || defined R_MODE
// OTK  0000000100000101 (S, R mode form)
       case 0b0100000101: return E50X(otk)(cpu, op);
#endif

// PID  0000000010001001 (S, R mode form)
       case 0b0010001001: return E50X(pid)(cpu, op);

// PIDA 0000000001001101 (V mode form)
       case 0b0001001101: return E50X(pida)(cpu, op);

// PIDL 0000000011000101 (V mode form)
       case 0b0011000101: return E50X(pidl)(cpu, op);

// PIM  0000000010000101 (S, R mode form)
       case 0b0010000101: return E50X(pim)(cpu, op);

// PIMA 0000000000001101 (V mode form)
       case 0b0000001101: return E50X(pima)(cpu, op);

// PIML 0000000011000001 (V mode form)
       case 0b0011000001: return E50X(piml)(cpu, op);

// PRTN 0000000110001001 (V mode foim)
       case 0b0110001001: return E50X(prtn)(cpu, op);

// PTLB 0000000000110100 (V mode foim)
       case 0b0000110100: return E50X(ptlb)(cpu, op);

// RMC  0000000000010001 (S, R, V mode form)
       case 0b0000010001: return E50X(rmc)(cpu, op);

// RRST 0000000111001111 AP\32 (V mode form)
       case 0b0111001111: return E50X(rrst)(cpu, op);

// RSAV 0000000111001101 AP\32 (V mode form)
       case 0b0111001101: return E50X(rsav)(cpu, op);

#if defined R_MODE
// RTN  0000000001000101 (R mode form)
       case 0b0001000101: return E50X(rtn)(cpu, op);
#endif

#if defined V_MODE
// RTS  0000000101001001 (V mode form)
       case 0b0101001001: return E50X(rts)(cpu, op);
#endif

// SCA  0000000000100001 (S, R mode form)
       case 0b0000100001: return E50X(sca)(cpu, op);

// SGL  0000000000000101 (S, R mode form)  -- Enter Single Precision mode  AFFECTS LDA STA ADD SUB
       case 0b0000000101: return E50X(sgl)(cpu, op);

// STAC 0000001010000000 AP\32 (V mode form)
       case 0b1010000000: return E50X(stac)(cpu, op);

// STC  000000101101 FLR 010 AP\32 (V mode form)
       case 0b1011010010:
       case 0b1011011010: return E50X(stc)(cpu, op);

// STEX 0000001011001101 (V mode form)
       case 0b1011001101: return E50X(stex)(cpu, op);

// STFA 000000101101 FAR 000 AP\32 (V mode form)
       case 0b1011010000:
       case 0b1011011000: return E50X(stfa)(cpu, op);

// STLC 0000001010000100 AP\32 (V mode form)
       case 0b1010000100: return E50X(stlc)(cpu, op);

// STPM 0000000000010100 (V mode form)
       case 0b0000010100: return E50X(stpm)(cpu, op);

// STTM 0000000101001000 (V mode form)
       case 0b0101001000: return E50X(sttm)(cpu, op);

// SVC  0000000101000101 (S, R, V mode form)
       case 0b0101000101: return E50X(svc)(cpu, op);

#if defined V_MODE || defined R_MODE
// TAK  0000001000001101 (V mode form)
       case 0b1000001101: return E50X(tak)(cpu, op);
#endif

// TFLL 000000101101 FLR 011 (V mode form)
       case 0b1011010011:
       case 0b1011011011: return E50X(tfll)(cpu, op);

//#if defined V_MODE || defined R_MODE || defined S_MODE
// TKA  0000001000000101 (V mode form)
       case 0b1000000101: return E50X(tka)(cpu, op);
//#endif

// TLFL 000000101101 FLR 001 (V mode form)
       case 0b1011010001:
       case 0b1011011001: return E50X(tlfl)(cpu, op);

// VIRY 0000000011001001 (S, R, V mode form)
       case 0b0011001001: return E50X(viry)(cpu, op);

// WAIT 0000000011001101 AP\32 (V mode form)
       case 0b0011001101: return E50X(wait)(cpu, op);

// XAD  0000001001000000 (V mode form)
       case 0b1001000000: return E50X(xad)(cpu, op);

// XMP  0000001001000100 (V mode form)
       case 0b1001000100: return E50X(xmp)(cpu, op);

// XBTD 0000001001100101 (V mode form)
       case 0b1001100101: return E50X(xbtd)(cpu, op);

// XCM  0000001001000010 (V mode form)
       case 0b1001000010: return E50X(xcm)(cpu, op);

// XDTB 0000001001100110 (V mode form)
       case 0b1001100110: return E50X(xdtb)(cpu, op);

// XDV  0000001001000111 (V mode form)
       case 0b1001000111: return E50X(xdv)(cpu, op);

// XED  0000001001001010 (V mode form)
       case 0b1001001010: return E50X(xed)(cpu, op);

// XMV  0000001001000001 (V mode form)
       case 0b1001000001: return E50X(xmv)(cpu, op);

// XVRY 0000001001001011 (S, R, V mode form)
       case 0b1001001011: return E50X(xvry)(cpu, op);

// ZCM  0000001001001111 (V mode form)
       case 0b1001001111: return E50X(zcm)(cpu, op);

// ZED  0000001001001001 (V mode form)
       case 0b1001001001: return E50X(zed)(cpu, op);

// ZFIL 0000001001001110 (V mode form)
       case 0b1001001110: return E50X(zfil)(cpu, op);

// ZMV  0000001001001100 (V mode form)
       case 0b1001001100: return E50X(zmv)(cpu, op);

// ZMVD 0000001001001101 (V mode form)
       case 0b1001001101: return E50X(zmvd)(cpu, op);

// ZTRN 0000001001001000 (V mode form)
       case 0b1001001000: return E50X(ztrn)(cpu, op);

// ???  0000000000000011
       case 0b0000000011: return E50X(003)(cpu, op); // ZZ ???

    default:
      switch(opcde & 0b1111000000)
      {
// WCS  0000001110 N\6 (R, V, I mode form)
       case 0b1110000000: return E50X(wcs)(cpu, op);
// DBG  0000001111 N\6 (R, V, I mode form)
       case 0b1111000000: return E50X(ill)(cpu, op); // ZZ ???
      }
  }

  return E50X(ill)(cpu, op);
}


static E50I(o60)
{
int opcde = ((op[0] & 0b00000011) << 8) | op[1];

  switch(opcde)
  {
#ifndef I_MODE
// A1A  1100001010000110 (S, R, V mode form)
       case 0b1010000110: return E50X(a1a)(cpu, op);

// A2A  1100000011000100 (S, R, V mode form)
       case 0b0011000100: return E50X(a2a)(cpu, op);
#endif

// ABQ  1100001111001110 (V mode form)
#ifdef V_MODE
       case 0b1111001110: return E50X(abq)(cpu, op);
#endif

#ifndef I_MODE
// ACA  1100001010001110 (S, R, V mode form)
       case 0b1010001110: return E50X(aca)(cpu, op);
#endif

// ALL  1100001000000000 (V mode form)
#ifdef V_MODE
       case 0b1000000000: return E50X(adll)(cpu, op);
#endif

// ATQ  1100001111001111 AP\32 (V mode form)
#ifdef V_MODE
       case 0b1111001111: return E50X(atq)(cpu, op);
#endif

#if defined R_MODE || defined V_MODE || defined I_MODE
// BCEQ 1100001110000010 ADDRESS\16 (V mode form)
       case 0b1110000010: return E50X(bceq)(cpu, op);

// BCGE 1100001110000101 ADDRESS\16 (V mode form)
       case 0b1110000101: return E50X(bcge)(cpu, op);

// BCGT 1100001110000001 ADDRESS\16 (V mode form)
       case 0b1110000001: return E50X(bcgt)(cpu, op);

// BCLE 1100001110000000 ADDRESS\16 (V mode form)
       case 0b1110000000: return E50X(bcle)(cpu, op);

// BCLT 1100001110000100 ADDRESS\16 (V mode form)
       case 0b1110000100: return E50X(bclt)(cpu, op);

// BCNE 1100001110000011 ADDRESS\16 (V mode form)
       case 0b1110000011: return E50X(bcne)(cpu, op);

// BCR  1100001111000101 ADDRESS\16 (V mode form)
       case 0b1111000101: return E50X(bcr)(cpu, op);

// BCS  1100001111000100 ADDRESS\16 (V mode form)
       case 0b1111000100: return E50X(bcs)(cpu, op); 
#endif

#if defined R_MODE || defined V_MODE
// BDX  1100000111011100 ADDRESS\16 (V mode form)
       case 0b0111011100: return E50X(bdx)(cpu, op);

// BEQ  1100000110001010 ADDRESS\16 (V mode form)
       case 0b0110001010: return E50X(beq)(cpu, op);
#endif

#if defined R_MODE || defined V_MODE
// BDY  1100000111010100 ADDRESS\16 (V mode form)
       case 0b0111010100: return E50X(bdy)(cpu, op);

// BIY  1100001011010100 ADDRESS\16 (V mode form)
       case 0b1011010100: return E50X(biy)(cpu, op);
#endif

#if defined R_MODE || defined V_MODE
// BFEQ 1100001110001010 ADDRESS\16 (V mode form)
       case 0b1110001010: return E50X(bfeq)(cpu, op);

// BFGE 1100001110001101 ADDRESS\16 (V mode form)
       case 0b1110001101: return E50X(bfge)(cpu, op);

// BFGT 1100001110001001 ADDRESS\16 (V mode form)
       case 0b1110001001: return E50X(bfgt)(cpu, op);

// BFLE 1100001110001000 ADDRESS\16 (V mode form)
       case 0b1110001000: return E50X(bfle)(cpu, op);

// BFLT 1100001110001100 ADDRESS\16 (V mode form)
       case 0b1110001100: return E50X(bflt)(cpu, op);

// BFNE 1100001110001011 ADDRESS\16 (V mode form)
       case 0b1110001011: return E50X(bfne)(cpu, op);

// BGE  1100000110001101 ADDRESS\16 (V mode form)
       case 0b0110001101: return E50X(bge)(cpu, op);

// BGT  1100000110001001 ADDRESS\16 (V mode form)
       case 0b0110001001: return E50X(bgt)(cpu, op);

// BIX  1100001011011100 ADDRESS\16 (V mode form)
       case 0b1011011100: return E50X(bix)(cpu, op);

// BLE  1100000110001000 ADDRESS\16 (V mode form)
       case 0b0110001000: return E50X(ble)(cpu, op);

// BLEQ 1100000111000010 ADDRESS\16 (V mode form)
       case 0b0111000010: return E50X(bleq)(cpu, op);

// BLGE 1100000110001101 ADDRESS\16 (V mode form)
// BGE case 0b0110001101: return E50X(blge)(cpu, op);

// BLGT 1100000111000001 ADDRESS\16 (V mode form)
       case 0b0111000001: return E50X(blgt)(cpu, op);

// BLLE 1100000111000000 ADDRESS\16 (V mode form)
       case 0b0111000000: return E50X(blle)(cpu, op);

// BLLT 1100000110001100 ADDRESS\16 (V mode form)
// BLT case 0b0110001100: return E50X(bllt)(cpu, op);

// BLNE 1100000111000011 ADDRESS\16 (V mode form)
       case 0b0111000011: return E50X(blne)(cpu, op);
#endif

#if defined R_MODE || defined V_MODE || defined I_MODE
// BLR  1100001111000111 ADDRESS\16 (V mode form)
       case 0b1111000111: return E50X(blr)(cpu, op);

// BLS  1100001111000110 ADDRESS\16 (V mode form)
       case 0b1111000110: return E50X(bls)(cpu, op);
#endif

#if defined V_MODE || defined R_MODE
// BLT  1100000110001100 ADDRESS\16 (V mode form)
       case 0b0110001100: return E50X(blt)(cpu, op);
#endif

#if defined V_MODE || defined I_MODE
// BMEQ 1100001110000010 ADDRESS\16 (V mode form)
//BCEQ case 0b1110000010: return E50X(bmeq)(cpu, op);

// BMGE 1100001111000110 ADDRESS\16 (V mode form)
// BLS case 0b1111000110: return E50X(bmge)(cpu, op);

// BMGT 1100001111001000 ADDRESS\16 (V mode form)
       case 0b1111001000: return E50X(bmgt)(cpu, op);

// BMLE 1100001111001001 ADDRESS\16 (V mode form)
       case 0b1111001001: return E50X(bmle)(cpu, op);

// BMLT 1100001111000111 ADDRESS\16 (V mode form)
// BLR case 0b1111000111: return E50X(bmlt)(cpu, op);

// BMNE 1100001110000011 ADDRESS\16 (V mode form)
//BCNE case 0b1110000011: return E50X(bmne)(cpu, op);
#endif

// BNE  1100000110001011 ADDRESS\16 (V mode form)
       case 0b0110001011: return E50X(bne)(cpu, op);

// CAL  1100001000101000 (S R V mode form)
       case 0b1000101000: return E50X(cal)(cpu, op);

// CAR  1100001000100100 (S R V mode form)
       case 0b1000100100: return E50X(car)(cpu, op);

// CAZ  1100000010001100 (S R V mode form)
       case 0b0010001100: return E50X(caz)(cpu, op);

// CHS  1100000000010100 (S R V mode form)
       case 0b0000010100: return E50X(chs)(cpu, op);

// CMA  1100000100000001 (S R V mode form)
       case 0b0100000001: return E50X(cma)(cpu, op);

// CRA  1100000000100000 (S R V mode form)
       case 0b0000100000: return E50X(cra)(cpu, op);

// CRB  1100000000001101 (S R V mode form)
       case 0b0000001101: return E50X(crb)(cpu, op);

// CRBx 1100000000001100 (S R V mode form)
       case 0b0000001100: return E50X(crbx)(cpu, op);

// CRE  1100001100000100 (V mode form)
       case 0b1100000100: return E50X(cre)(cpu, op);

// CRL  1100000000001000 (S R V mode form)
       case 0b0000001000: return E50X(crl)(cpu, op);

// CRLE 1100001100001000 (V mode form)
       case 0b1100001000: return E50X(crle)(cpu, op);

// CSA  1100000011010000 (S R V mode form)
       case 0b0011010000: return E50X(csa)(cpu, op);

// DFCM 1100000101111100 (R V mode form)
       case 0b0101111100: return E50X(dfcm)(cpu, op);

// DRX  1100000010001000 (S R V mode form)
       case 0b0010001000: return E50X(drx)(cpu, op);

// FCDQ 1100000101111001 (V mode form)
// DRNM 1100000101111001 (V mode form)
       case 0b0101111001: return E50X(fcdq)(cpu, op);

// FCM  1100000101011000 (R, V mode form)
       case 0b0101011000: return E50X(fcm)(cpu, op);

// FDBL 1100000000001110 (V mode form)
       case 0b0000001110: return E50X(fdbl)(cpu, op);

// FLOT 1100000101101000 (R mode form)
       case 0b0101101000: return E50X(flot)(cpu, op);

// FLTA 1100000101011010 (V mode form)
       case 0b0101011010: return E50X(flta)(cpu, op);

// FLTL 1100000101011101 (V mode form)
       case 0b0101011101: return E50X(fltl)(cpu, op);

// FRN  1100000101011100 (R, V mode form)
       case 0b0101011100: return E50X(frn)(cpu, op);

#if defined R_MODE || defined V_MODE
// FSGT 1100000101001101 (R, V mode form)
       case 0b0101001101: return E50X(fsgt)(cpu, op);

// FSLE 1100000101001100 (R, V mode form)
       case 0b0101001100: return E50X(fsle)(cpu, op);

// FSMI 1100000101001010 (R, V mode form)
       case 0b0101001010: return E50X(fsmi)(cpu, op);

// FSNZ 1100000101001001 (R, V mode form)
       case 0b0101001001: return E50X(fsnz)(cpu, op);

// FSPL 1100000101001011 (R, V mode form)
       case 0b0101001011: return E50X(fspl)(cpu, op);

// FSZE 1100000101001000 (R, V mode form)
       case 0b0101001000: return E50X(fsze)(cpu, op);
#endif

// ICA  1100001011100000 (S, R, V mode form)
       case 0b1011100000: return E50X(ica)(cpu, op);

// ICL  1100001001100000 (S, R, V mode form)
       case 0b1001100000: return E50X(icl)(cpu, op);

// ICR  1100001010100000 (S, R, V mode form)
       case 0b1010100000: return E50X(icr)(cpu, op);

// ILE  1100001100001100 (S, R, V mode form)
       case 0b1100001100: return E50X(ile)(cpu, op);

// INT  1100000101101100 (S, R mode form)
       case 0b0101101100: return E50X(int)(cpu, op);

// INTA 1100000101011001 (V mode form)
       case 0b0101011001: return E50X(inta)(cpu, op);

// INTL 1100000101011011 (V mode form)
       case 0b0101011011: return E50X(intl)(cpu, op);

// IRX  1100000001001100 (S, R, V mode form)
       case 0b0001001100: return E50X(irx)(cpu, op);

// LCEQ 1100001101000011 (V mode form)
       case 0b1101000011: return E50X(lceq)(cpu, op);

// LCGE 1100001101000100 (V mode form)
       case 0b1101000100: return E50X(lcge)(cpu, op);

// LCGT 1100001101000101 (V mode form)
       case 0b1101000101: return E50X(lcgt)(cpu, op);

// LCLE 1100001101000001 (V mode form)
       case 0b1101000001: return E50X(lcle)(cpu, op);

// LCLT 1100001101000000 (V mode form)
       case 0b1101000000: return E50X(lclt)(cpu, op);

// LCNE 1100001101000010 (V mode form)
       case 0b1101000010: return E50X(lcne)(cpu, op);

// LEQ  1100000100001011 (S, R, V mode form)
       case 0b0100001011: return E50X(leq)(cpu, op);

#if defined R_MODE || defined V_MODE
// LFEQ 1100001001001011 (V mode form)
       case 0b1001001011: return E50X(lfeq)(cpu, op);

// LFGE 1100001001001100 (V mode form)
       case 0b1001001100: return E50X(lfge)(cpu, op);

// LFGT 1100001001001101 (V mode form)
       case 0b1001001101: return E50X(lfgt)(cpu, op);

// LFLE 1100001001001001 (V mode form)
       case 0b1001001001: return E50X(lfle)(cpu, op);

// LFLT 1100001001001000 (V mode form)
       case 0b1001001000: return E50X(lflt)(cpu, op);

// LFNE 1100001001001010 (V mode form)
       case 0b1001001010: return E50X(lfne)(cpu, op);
#endif

// LGE  1100000100001100 (S, R, V mode form)
       case 0b0100001100: return E50X(lge)(cpu, op);

// LGT  1100000100001101 (S, R, V mode form)
       case 0b0100001101: return E50X(lgt)(cpu, op);

// LLE  1100000100001001 (S, R, V mode form)
       case 0b0100001001: return E50X(lle)(cpu, op);

// LLEQ 1100001101001011 (V mode form)
       case 0b1101001011: return E50X(lleq)(cpu, op);

// LLGE 1100000100001100 (V mode form)
// LLE case 0b0100001100: return E50X(llge)(cpu, op);

// LLGT 1100001101001101 (V mode form)
       case 0b1101001101: return E50X(llgt)(cpu, op);

// LLLE 1100001101001001 (V mode form)
       case 0b1101001001: return E50X(llle)(cpu, op);

// LLLT 1100000100001000 (V mode form)
// LLT case 0b0100001000: return E50X(lllt)(cpu, op);

// LLNE 1100001101001010 (V mode form)
       case 0b1101001010: return E50X(llne)(cpu, op);

// LLT  1100000100001000 (S, R, V mode form)
       case 0b0100001000: return E50X(llt)(cpu, op);

// LNE  1100000100001010 (S, R, V mode form)
       case 0b0100001010: return E50X(lne)(cpu, op);

// LF   1100000100001110 (S, R, V mode form)
       case 0b0100001110: return E50X(lf)(cpu, op);

// LT   1100000100001111 (S, R, V mode form)
       case 0b0100001111: return E50X(lt)(cpu, op);

// QFCM 1100000101111000 (V mode form)
       case 0b0101111000: return E50X(qfcm)(cpu, op);

// QINQ 1100000101111010 (V mode form)
       case 0b0101111010: return E50X(qinq)(cpu, op);

// QIQR 1100000101111011 (V mode form)
       case 0b0101111011: return E50X(qiqr)(cpu, op);

#ifdef V_MODE
// RBQ  1100001111001101 AP\32 (V mode form)
       case 0b1111001101: return E50X(rbq)(cpu, op);
#endif

// RCB  1100000010000000 (S, R, V mode form)
       case 0b0010000000: return E50X(rcb)(cpu, op);

#ifdef V_MODE
// RTQ  1100001111001100 AP\32 (V mode form)
       case 0b1111001100: return E50X(rtq)(cpu, op);
#endif

// S1A  1100000001001000 (S, R, V mode form)
       case 0b0001001000: return E50X(s1a)(cpu, op);

// S2A  1100000011001000 (S, R, V mode form)
       case 0b0011001000: return E50X(s2a)(cpu, op);

// SCB  1100000110000000 (S, R, V mode form)
       case 0b0110000000: return E50X(scb)(cpu, op);

// SSM  1100000101000000 (S, R, V mode form)
       case 0b0101000000: return E50X(ssm)(cpu, op);

// SSP  1100000001000000 (S, R, V mode form)
       case 0b0001000000: return E50X(ssp)(cpu, op);

// TAB  1100000011001100 (V mode form)
       case 0b0011001100: return E50X(tab)(cpu, op);

// TAX  1100000101000100 (V mode form)
       case 0b0101000100: return E50X(tax)(cpu, op);

// TAY  1100000101000101 (V mode form)
       case 0b0101000101: return E50X(tay)(cpu, op);

// TBA  1100000110000100 (V mode form)
       case 0b0110000100: return E50X(tba)(cpu, op);

// TCA  1100000100000111 (S, R, V mode form)
       case 0b0100000111: return E50X(tca)(cpu, op);

// TCL  1100001010001000 (V mode form)
       case 0b1010001000: return E50X(tcl)(cpu, op);

#ifdef V_MODE
// TSTQ 1100001111101111 AP\32 (V mode form)
       case 0b1111101111: return E50X(tstq)(cpu, op);
#endif

// TXA  1100001000011100 (V mode form)
       case 0b1000011100: return E50X(txa)(cpu, op);

// TYA  1100001001010100 (V mode form)
       case 0b1001010100: return E50X(tya)(cpu, op);

// XCA  1100000001000100 (S, R, V mode form)
       case 0b0001000100: return E50X(xca)(cpu, op);

// XCB  1100000010000100 (S, R, V mode form)
       case 0b0010000100: return E50X(xcb)(cpu, op);

  }

  return E50X(ill)(cpu, op);
}


#if defined E16S || defined E32S || defined E32R || defined E64R || defined E64V
static inline E50I(ix1101_decode)
{
static const inst_t ix1101_tab[8] = {
// DFLX I0 1101 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      I0 1101 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// FLX  I0 1101 11000Y01 BR\2 DISPLACEMENT\16 (V mode long)
//      I0 1101 11000001 CB\2 DISPLACEMENT\16 (R mode long)
// JDX  I0 1101 11000010 CB\2 DISPLACEMENT\16 (R mode long)
// JIX  I0 1101 11000011 CB\2 DISPLACEMENT\16 (R mode form)
// JSX  I1 1101 11000Y11 BR\2 DISPLACEMENT\16 (V mode long)
//      I1 1101 11000011 CB\2 DISPLACEMENT\16 (R mode long)
// LDX  I1 1101 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      I1 1101 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      I1 1101 DISPLACEMENT\10 (S, R V mode short)
// LDY  I1 1101 11000Y01 BR\2 DISPLACEMENT\16 (V mode form)
// QFLX I0 1101 11000Y11 BR\2 DISPLACEMENT\16 (V mode long)
// STX  I0 1101 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      I0 1101 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      I0 1101 DISPLACEMENT\10 (S mode; R, V mode short)
// STY  I1 1101 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
  E50X(stx),
  E50X(flx),
#if defined R_MODE
  E50X(jdx),
  E50X(jix),
#else
  E50X(dflx),
  E50X(qflx),
#endif
  E50X(ldx),
  E50X(ldy),
  E50X(sty),
  E50X(jsx)
};
int opcde = (op[0] & 0b01000000) >> 4;

  if(op_is_long(op))
    opcde |= ((op[1] & 0b1100) >> 2);
  
  return ix1101_tab[opcde](cpu, op);
}


static inline E50I(ix_decode)
{
static const inst_t ix_tab[64] = {
  E50X(uii),
  E50X(uii),
  E50X(uii),
  E50X(uii),
// EAA  IX 0001 11000001 CB\2 DISPLACEMENT\16 (R mode form)
// EAL  IX 0001 11000Y01 BR\2 DISPLACEMENT\16 (V mode form)
// JMP  IX 0001 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 0001 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      IX 0001 DISPLACEMENT\16 (S, R V short)
// XEC  IX 0001 11000Y10 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 0001 11000010 CB\2 DISPLACEMENT\16 (R mode long)
// ENTR IX 0001 11000011 CB\2 DISPLACEMENT\16 (R mode long form)
  E50X(jmp),
#if defined V_MODE
  E50X(eal),
#else
  E50X(eaa),
#endif
  E50X(xec),
#if defined R_MODE
  E50X(entr),
#else
  E50X(uii),
#endif
// DFLD IX 0010 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 0010 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// DLD  IX 0010 11000000 CB\2 DISPLACEMENT\16 (R mode form)
//      IX 0010 DISPLACEMENT\10 (S R mode form)
// FLD  IX 0010 11000Y01 BR\2 DISPLACEMENT\16 (V mode long)

// LDA  IX 0010 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 0010 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      IX 0010 DISPLACEMENT\10 (S mode, R, V mode short)
// LDL  IX 0010 11000Y11 BR\2 DISPLACEMENT\16 (V mode form)
// JEQ  1X 0010 11000011 CB\2 DISPLACEMENT\16 (R mode form)
  E50X(lda),
  E50X(fld),
  E50X(dfld),
#if defined V_MODE
  E50X(ldl),
#else
  E50X(jeq),
#endif
// ANA  IX 0011 11000Y00 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 0011 11000000 CB\2 DISPLACEMENT\16 (R mode form)
//      IX 0011 DISPLACEMENT\10 (S R V)
// ANL  IX 0011 11000Y11 BR\2 DISPLACEMENT\16 (V mode form)
// ORA  IX 0011 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
// STLR IX 0011 11000Y01 BR\2 DISPLACEMENT\16 (V mode form)
// JNE  IX 0011 11000011 CB\2 DISPLACEMENT\16 (R mode form)
  E50X(ana),
  E50X(stlr),
  E50X(ora),
#if defined V_MODE
  E50X(anl),
#else
  E50X(jne),
#endif
// DFST IX 0100 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 0100 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// DST  IX 0100 11000000 CB\2 DISPLACEMENT\16 (R mode form)
//      IX 0100 DISPLACEMENT\16 (S R V)
// FST  IX 0100 11000Y01 BR\2 DISPLACEMNET\16 (V mode long)
//      IX 0100 11000001 CB\2 DISPLACEMNET\16 (R mode long)
// STA  IX 0100 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 0100 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      IX 0100 DISPLACEMENT\10 (S mode, R, V short)
// STL  IX 0100 11000Y11 BR\2 DISPLACEMENT\16 (V mode form)
// JLE  IX 0100 11000011 CB\2 DISPLACEMENT\16 (R mode form)
// MIA  11 0100 ER\3 TM\2 SR\3 BR\2 DISPLACEMENT\16 (I mode form)
  E50X(sta),
  E50X(fst),
  E50X(dfst),
#if defined V_MODE
  E50X(stl),
#else
  E50X(jle),
#endif
// ERA  IX 0101 11000Y00 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 0101 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      IX 0101 DISPLACEMENT\10 (S mode; R, V mode short)
// ERL  IX 0101 11000Y11 BR\2 DISPLACEMENT\16 (V mode long)
// QFAD IX 0101 11000Y10 BR\2 DISPLACEMENT\16 00000000000000I0 (V mode long)
// QFCS IX 0101 11000Y10 BR\2 DISPLACEMENT\16 0000000000000110 (V mode long)
// QFDV IX 0101 11000Y10 BR\2 DISPLACEMENT\16 0000000000000101 (V mode long)
// QFLD IX 0101 11000Y10 BR\2 DISPLACEMENT\16 0000000000000000 (V mode long)
// QFMP IX 0101 11000Y10 BR\2 DISPLACEMENT\16 0000000000000100 (V mode long)
// QFSB IX 0101 11000Y10 BR\2 DISPLACEMENT\16 0000000000000011 (V mode long)
// QFST IX 0101 11000Y10 BR\2 DISPLACEMENT\16 0000000000000001 (V mode long)
// LDLR IX 0101 11000Y01 BR\2 DISPLACEMENT\16 (V mode form)
// JGT  IX 0101 11000011 GB\2 DISPLACEMENT\16 (R mode form)
  E50X(era),
  E50X(ldlr),
#if defined V_MODE
  E50X(qfxx),
#else
  E50X(uii),
#endif
#if defined V_MODE
  E50X(erl),
#else
  E50X(jgt),
#endif
// ADD  IX 0110 11000Y00 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 0110 11000000 CB\2 DISPLACEMENT\16 (R mode form)
// ADL  IX 0110 11000Y11 BR\2 DISPLACEMENT\16 (V mode form)
// DAD  IX 0110 11000000 CB\2 DISPLACEMENT\16 (R mode form)
//      IX 0110 DISPLACEMENT\10 (S R mode form)
// DFAD IX 0110 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 0110 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// FAD  IX 0110 11000Y01 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 0110 11000001 CB\2 DISPLACEMENT\16 (R mode long)
// JLT  IX 0110 11000011 CB\2 DISPLACEMENT\16 (R mode form)
  E50X(add),
  E50X(fad),
  E50X(dfad),
#if defined V_MODE
  E50X(adl),
#else
  E50X(jlt),
#endif
// DFSB IX 0111 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 0111 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// DSB  IX 0111 11000000 CB\2 DISPLACEMENT\16 (R mode form)
//      IX 0111 DISPLACEMENT\10 (S ; R mode form)
// FSB  IX 0111 11000Y01 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 0111 11000001 CB\2 DISPLACEMENT\16 (R mode long)
// SBL  IX 0111 11000Y11 BR\2 DISPLACEMENT\16 (V mode form)
// SUB  IX 0111 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 0111 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      IX 0111 DISPLACEMENT\10 (S ; R V mode short)
// JGE  IX 0111 11000011 CB\2 DISPLACEMENT\16 (R mode form)
  E50X(sub),
  E50X(fsb),
  E50X(dfsb),
#if defined V_MODE
  E50X(sbl),
#else
  E50X(jge),
#endif
// JST  IX 1000 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 1000 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      IX 1000 DISPLACEMENT\10 (S R V short) 
// PCL  IX 1000 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
// CREP IX 1000 11000010 CB\2 DISPLACEMENT\16 (R mode form)
  E50X(jst),
  E50X(uii),
#if defined V_MODE
  E50X(pcl),
#elif defined R_MODE
  E50X(crep),
#else
  E50X(uii),
#endif
  E50X(uii),
// CAS  IX 1001 11000Y00 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 1001 11000000 CB\2 DISPLACEMENT\16 (R mode form)
//      IX 1001 DISPLACEMENT\10 (S R V mode form)
// CLS  IX 1001 11000Y11 BR\2 DISPLACEMENT\16 (V mode form)
// DFCS IX 1001 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 1001 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// FCS  IX 1001 11000Y01 BR\2 DISPLACEMENT\16 (V long)
//      IX 1001 11000001 CB\2 DISPLACEMENT\16 (R long)
  E50X(cas),
  E50X(fcs),
  E50X(dfcs),
  E50X(cls),
// EAXB IX 1010 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
// IRS  IX 1010 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 1010 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      IX 1010 DISPLACEMNET\10 (S, R V SHORT)
// MIA  IX 1010 11000Y01 BR\2 DISPLACEMENT\16 (V mode long form)
  E50X(irs),
  E50X(mia),
  E50X(eaxb),
  E50X(uii),
// EALB IX 1011 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
// IMA  IX 1011 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 1011 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      IX 1011 DISPLACEMENT\16 (S mode; R, V mode short)
// MIB  IX 1011 11000Y01 BR\2 iDISPLACEMENT\16 (V mode long)
  E50X(ima),
  E50X(mib),
  E50X(ealb),
  E50X(uii),
// EIO  I0 1100 11000Y01 BR\2 DISPLACEMENT\16 (V mode form)
// INA  10 1100 FUNCTI0N\4 DEVICE\6  (S, R mode form)
// JSXB IX 1100 11000Y10 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 1100 11000010 CB\2 DISPLACEMENT\16 (R mode long)
// JSY  IX 1100 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 1100 DISPLACEMENT\10 (V mode short)
// OCP  00 1100 FUNCTION\4 DEVTCE\6 (S, R mode form)
// OTA  11 1100 FUNCTION\4 DEVTCE\6 (S, R mode form)
// SKS  01 1100 FUNCTICN\4 DEVTCE\6 (S, R mode form)
// MIB  11 1100 ER\3 TM\2 SR\3 BR\2 DISPLACEMENT\16 (I mode form)
#ifdef V_MODE
  E50X(jsy),
  E50X(eio),
  E50X(jsxb),
  E50X(uii),
#else
  E50X(pio),
  E50X(pio),
  E50X(pio),
  E50X(pio),
#endif
// DFLX I0 1101 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      I0 1101 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// FLX  I0 1101 11000Y01 BR\2 DISPLACEMENT\16 (V mode long)
//      I0 1101 11000001 CB\2 DISPLACEMENT\16 (R mode long)
// JDX  I0 1101 11000010 CB\2 DISPLACEMENT\16 (R mode long)
// JIX  I0 1101 11000011 CB\2 DISPLACEMENT\16 (R mode form)
// JSX  I1 1101 11000Y11 BR\2 DISPLACEMENT\16 (V mode long)
//      I1 1101 11000011 CB\2 DISPLACEMENT\16 (R mode long)
// LDX  I1 1101 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      I1 1101 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      I1 1101 DISPLACEMENT\10 (S, R V mode short)
// LDY  I1 1101 11000Y01 BR\2 DISPLACEMENT\16 (V mode form)
// QFLX I0 1101 11000Y11 BR\2 DISPLACEMENT\16 (V mode long)
// STX  I0 1101 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      I0 1101 11000000 CB\2 DISPLACEMENT\16 (R mode long)
//      I0 1101 DISPLACEMENT\10 (S mode; R, V mode short)
// STY  I1 1101 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
  E50X(ix1101_decode),
  E50X(ix1101_decode),
  E50X(ix1101_decode),
  E50X(ix1101_decode),
// DFMP IX 1110 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 1110 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// FMP  IX 1110 11000Y01 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 1110 11000Y01 CB\2 DISPLACEMENT\16 (R mode long)
// MPL  IX 1110 11000Y11 BR\2 DISPLACEMENT\16 (V mode form)
// MPY  IX 1110 11000Y00 BR\2 DISPLACEMENT\16 (V mode long)
//      IX 1110 DISPLACEMENT\10 (S mode; R mode short)
//      IX 1110 11000000 CB\2 DISPLACEMENT\16 (R mode long)
#ifndef S_MODE
  E50X(mpy),
  E50X(fmp),
  E50X(dfmp),
#ifdef V_MODE
  E50X(mpl),
#else
  E50X(uii),
#endif
#else
  E50X(mpy),
  E50X(mpy),
  E50X(mpy),
  E50X(mpy),
#endif
// DFDV IX 1111 11000Y10 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 1111 11000010 CB\2 DISPLACEMENT\16 (R mode form)
// DIV  IX 1111 11000000 CB\2 DISPLACEMENT\16 (R mode form)
//      IX 1111 DISPLACEMENT\10 (R S
// DIV  IX 1111 11000Y00 BR\2 DISPLACEMENT\16 (V mode form)
//      IX 1111  DISPLACEMENT\10 (V mode form)
// DVL  IX 1111 11000Y11 BR\2 DISPLACEMENT\16 (V mode form)
// FDV  IX 1111 11000Y01 BR\2 DISPLACEMENT\16 (V long)
// FDV  IX 1111 11000001 BR\2 DISPLACEMENT\16 (R long)
#ifndef S_MODE
  E50X(div),
  E50X(fdv),
  E50X(dfdv),
#ifdef V_MODE
  E50X(dvl)
#else
  E50X(uii)
#endif
#else
  E50X(div),
  E50X(div),
  E50X(div),
  E50X(div)
#endif
};
int opcde = (op[0] & 0b111100);

  if(op_is_long(op))
    opcde |= ((op[1] & 0b1100) >> 2);
  
  return ix_tab[opcde](cpu, op);
}


static E50I(o21)
{
int opcde = ((op[0] & 0b00000011) << 8) | op[1];

  switch(opcde)
  {
// DRN  0100000011000000 (V mode form)
       case 0b0011000000: return E50X(drn)(cpu, op);

// DRNP 0100000011000001 (V mode form)
       case 0b0011000001: return E50X(drnp)(cpu, op);

// DRNZ 0100000011000010 (V mode form)
       case 0b0011000010: return E50X(drnz)(cpu, op);

// FRNM 0100000011010000 (V mode form)
       case 0b0011010000: return E50X(frnm)(cpu, op);

// FRNP 0100000011000011 (V mode form)
       case 0b0011000011: return E50X(frnp)(cpu, op);

// FRNZ 0100000011010001 (V mode form)
       case 0b0011010001: return E50X(frnz)(cpu, op);

// SSSN 0100000011001000 (V mode form)
       case 0b0011001000: return E50X(sssn)(cpu, op);
  }

  return E50X(uii)(cpu, op);
}

static E50I(o20)
{
static const inst_t o20_tab[16] = {
             // 12 3456 7890 123456
             // 01 0000 XXXX NNNNNN
  E50X(lrl), //         0000
  E50X(lrs), //         0001
  E50X(lrr), //         0010
  E50X(o21), //         0011
  E50X(arl), //         0100
  E50X(ars), //         0101
  E50X(arr), //         0110
  E50X(uii), //         0111
  E50X(lll), //         1000
  E50X(lls), //         1001
  E50X(llr), //         1010
  E50X(uii), //         1011
  E50X(all), //         1100
  E50X(als), //         1101
  E50X(alr), //         1110
  E50X(uii)  //         1111
};
int opcde = ((op[0] & 0b00000011) << 2) | ((op[1] & 0b11000000) >> 6);
  
  return o20_tab[opcde](cpu, op);
}


static E50I(o40)
{
int opcde = ((op[0] & 0b00000011) << 8) | op[1];

  switch(opcde)
  {
// SS1  1000001000010000 (S, R, V mode form)
       case 0b1000010000: return E50X(ss1)(cpu, op);

// SR1  1000000000010000 (S, R, V mode form)
       case 0b0000010000: return E50X(sr1)(cpu, op);

// SS2  1000001000001000 (S, R, V mode form)
       case 0b1000001000: return E50X(ss2)(cpu, op);

// SR2  1000000000001000 (S, R, V mode form)
       case 0b0000001000: return E50X(sr2)(cpu, op);

// SR3  1000000000000100 (S, R, V mode form)
       case 0b0000000100: return E50X(sr3)(cpu, op);

// SS3  1000001000000100 (S, R, V mode form)
       case 0b1000000100: return E50X(ss3)(cpu, op);

// SS4  1000001000000010 (S, R, V mode form)
       case 0b1000000010: return E50X(ss4)(cpu, op);

// SR4  1000000000000010 (S, R, V mode form)
       case 0b0000000010: return E50X(sr4)(cpu, op);

// SSS  1000001000011110 (S, R, V mode form)
       case 0b1000011110: return E50X(sss)(cpu, op);

// SSR  1000000000011110 (S, R, V mode form)
       case 0b0000011110: return E50X(ssr)(cpu, op);

// NOP  1000001000000000 (S, R, V mode form)
       case 0b1000000000: return E50X(nop)(cpu, op);

// SGT  1000000010010000 (S, R, V mode form)
       case 0b0010010000: return E50X(sgt)(cpu, op);

// SKP  1000000000000000 (S, R, V mode form)
       case 0b0000000000: return E50X(skp)(cpu, op);

// SLE  1000001010010000 (S, R, V mode form)
       case 0b1010010000: return E50X(sle)(cpu, op);

// SLN  1000001001000000 (S, R, V mode form)
       case 0b1001000000: return E50X(sln)(cpu, op);

// SLZ  1000000001000000 (S, R, V mode form)
       case 0b0001000000: return E50X(slz)(cpu, op);

// SMCR 1000000010000000 (S, R, V mode form)
       case 0b0010000000: return E50X(smcr)(cpu, op);

// SMCS 1000001010000000 (S, R, V mode form)
       case 0b1010000000: return E50X(smcs)(cpu, op);

// SMI  1000001100000000 (S, R, V mode form) -- also slt
       case 0b1100000000: return E50X(smi)(cpu, op);

// SNZ  1000001000100000 (S, R, V mode form) -- also sne
       case 0b1000100000: return E50X(snz)(cpu, op);

// SPL  1000000100000000 (S, R, V mode form) -- also sge
       case 0b0100000000: return E50X(spl)(cpu, op);

// SRC  1000000000000001 (S, R, V mode form)
       case 0b0000000001: return E50X(src)(cpu, op);

// SSC  1000001000000001 (S, R, V mode form)
       case 0b1000000001: return E50X(ssc)(cpu, op);

// SZE  1000000000100000 (S, R, V mode form) -- also seq
       case 0b0000100000: return E50X(sze)(cpu, op);

    default:
      switch((opcde & 0b1111110000))
      {
// SAR  100000001011 N\4 (S, R, V mode form)
       case 0b0010110000: return E50X(sar)(cpu, op);

// SAS  100000101011 N\4 (S, R, V mode form)
       case 0b1010110000: return E50X(sas)(cpu, op);

// SNR  100000001010 N\4 (S, R, V mode form)
       case 0b0010100000: return E50X(snr)(cpu, op);

// SNS  100000101010 N\4 (S, R, V mode form)
       case 0b1010100000: return E50X(sns)(cpu, op);
      }

  }

  return E50X(uii)(cpu, op);
}


E50I(decode)
{
static const inst_t ix_tab[4] = {
  E50X(o00),  // 00 0000 Generic B Instructions
  E50X(o20),  // 01 0000 Shift Instructions
  E50X(o40),  // 10 0000 Skip Instructions
  E50X(o60)   // 11 0000 Generic A Instructions
};
int opcde = op[0] >> 6;

  if((op[0] & 0b00111100) == 0)
    return ix_tab[opcde](cpu, op);
  else
    return E50X(ix_decode)(cpu, op);
}

#elif defined E32I


static E50I(o06)
{
static const inst_t op_tab[8] = {
  E50X(fl),    // 000 0F0
  E50X(dfl),   // 001 0F1
  E50X(fl),    // 010 0F0
  E50X(dfl),   // 011 0F1
  E50X(fc),    // 100 1F0
  E50X(dfc),   // 101 1F1
  E50X(fc),    // 110 1F0
  E50X(dfc)    // 111 1F1
};
int opcde = ((op[0] & 0b11) << 1) | (op[1] >> 7);

  return op_tab[opcde](cpu, op);
}


static E50I(o10)
{
int opcde = ((op[0] & 0b11) << 8) | op[1];

  switch(opcde) {

// BFEQ 0010000 F 01010010 ADDRESS\16
         case 0b0001010010:
         case 0b0101010010: return E50X(bfeq)(cpu, op);

// BFGE 0010000 F 01010101 ADDRESS\16
         case 0b0001010101:
         case 0b0101010101: return E50X(bfge)(cpu, op);

// BFGT 0010000 F 01010001 ADDRESS\16
         case 0b0001010001:
         case 0b0101010001: return E50X(bfgt)(cpu, op);

// BFLE 0010000 F 01010000 ADDRESS\16
         case 0b0001010000:
         case 0b0101010000: return E50X(bfle)(cpu, op);

// BFLT 0010000 F 01010100 ADDRESS\16
         case 0b0001010100:
         case 0b0101010100: return E50X(bflt)(cpu, op);

// BFNE 0010000 F 01010011 ADDRESS\16
         case 0b0001010011:
         case 0b0101010011: return E50X(bfne)(cpu, op);

    default: switch(opcde & 0b01111111) {

// BHD1 001000 R\3 1100100 ADDRESS\16
            case 0b1100100: return E50X(bhd1)(cpu, op);

// BHD2 001000 R\3 1100101 ADDRESS\16
            case 0b1100101: return E50X(bhd2)(cpu, op);

// BHD4 001000 R\3 1100110 ADDRESS\16
            case 0b1100110: return E50X(bhd4)(cpu, op);

// BHEQ 001000 R\3 1001010 ADDRESS\16
            case 0b1001010: return E50X(bheq)(cpu, op);

// BHGE 001000 R\3 1010101 ADDRESS\16
            case 0b1010101: return E50X(bhge)(cpu, op);

// BHGT 001000 R\3 1001001 ADDRESS\16
            case 0b1001001: return E50X(bhgt)(cpu, op);

// BHLE 001000 R\3 1001000 ADDRESS\16
            case 0b1001000: return E50X(bhle)(cpu, op);

// BHLT 001000 R\3 1001100 ADDRESS\16
            case 0b1001100: return E50X(bhlt)(cpu, op);

// BHNE 001000 R\3 1001011 ADDRESS\16
            case 0b1001011: return E50X(bhne)(cpu, op);

// BHI1 001000 R\3 1100000 ADDRESS\16
            case 0b1100000: return E50X(bhi1)(cpu, op);

// BHI2 001000 R\3 1100001 ADDRESS\16
            case 0b1100001: return E50X(bhi2)(cpu, op);

// BHI4 001000 R\3 1100010 ADDRESS\16
            case 0b1100010: return E50X(bhi4)(cpu, op);

// BRD1 001000 R\3 1011100 ADDRESS\16
            case 0b1011100: return E50X(brd1)(cpu, op);

// BRD2 001000 R\3 1011101 ADDRESS\16
            case 0b1011101: return E50X(brd2)(cpu, op);

// BRD4 001000 R\3 1011110 ADDRESS\16
            case 0b1011110: return E50X(brd4)(cpu, op);

// BREQ 001000 R\3 1000010 ADDRESS\16
            case 0b1000010: return E50X(breq)(cpu, op);

// BRGT 001000 R\3 1000001 ADDRESS\16
            case 0b1000001: return E50X(brgt)(cpu, op);

// BRGE 001000 R\3 1000101 ADDRESS\16
            case 0b1000101: return E50X(brge)(cpu, op);

// BRLE 001000 R\3 1000000 ADDRESS\16
            case 0b1000000: return E50X(brle)(cpu, op);

// BRLT 001000 R\3 1000100 ADDRESS\16
            case 0b1000100: return E50X(brlt)(cpu, op);

// BRNE 001000 R\3 1000011 ADDRESS\16
            case 0b1000011: return E50X(brne)(cpu, op);

// BRI1 001000 R\3 1011000 ADDRESS\16
            case 0b1011000: return E50X(bri1)(cpu, op);

// BRI2 001000 R\3 1011001 ADDRESS\16
            case 0b1011001: return E50X(bri2)(cpu, op);

// BRI4 001000 R\3 1011010 ADDRESS\16
            case 0b1011010: return E50X(bri4)(cpu, op);

    default: switch(opcde & 0b01100000) {
// BRBR 001000 R\3 01 BT\5 ADDRESS\16
            case 0b0100000: return E50X(brbr)(cpu, op);

// BRBS 001000 R\3 00 BT\5 ADDRESS\16
            case 0b0000000: return E50X(brbs)(cpu, op);

            default:        return E50X(uii)(cpu, op);
      }
    } 
  }
}


static E50I(o16)
{
static const inst_t op_tab[8] = {
  E50X(fst),   // 000 0F0
  E50X(dfst),  // 001 0F1
  E50X(fst),   // 010 0F0
  E50X(dfst),  // 011 0F1
  E50X(fa),    // 100 1F0
  E50X(dfa),   // 101 1F1
  E50X(fa),    // 110 1F0
  E50X(dfa)    // 111 1F1
};
int opcde = ((op[0] & 0b11) << 1) | (op[1] >> 7);

  return op_tab[opcde](cpu, op);
}


static E50I(o20)
{
int opcde = ((op[0] & 0b00000011) << 8) | op[1];

  switch(opcde)
  {
// DRNZ 0100000011000010 (I mode form)
       case 0b0011000010: return E50X(drnz)(cpu, op);

// DRN  0100000011000000 (I mode form)
       case 0b0011000000: return E50X(drn)(cpu, op);

// DRNP 0100000011000001 (I mode form)
       case 0b0011000001: return E50X(drnp)(cpu, op);

// SSSN 0100000011001000 (I mode form)
       case 0b0011001000: return E50X(sssn)(cpu, op);

  }

  return E50X(uii)(cpu, op);
}


static E50I(o26)
{
static const inst_t op_tab[8] = {
  E50X(fs),    // 000 0F0
  E50X(dfs),   // 001 0F1
  E50X(fs),    // 010 0F0
  E50X(dfs),   // 011 0F1
  E50X(fm),    // 100 1F0
  E50X(dfm),   // 101 1F1
  E50X(fm),    // 110 1F0
  E50X(dfm)    // 111 1F1
};
int opcde = ((op[0] & 0b11) << 1) | (op[1] >> 7);

  return op_tab[opcde](cpu, op);
}


static E50I(o30)
{
int opcde = op[1] & 0b01111111;

  switch(opcde) {

// ABQ  011000 R\3 1011100
            case 0b1011100: return E50X(abq)(cpu, op);

// ADLR 011000 R\3 0001100
            case 0b0001100: return E50X(adlr)(cpu, op);

// ARFA 011000 R\3 111 FAR 001
            case 0b1110001:
            case 0b1111001: return E50X(arfa)(cpu, op);

// ATQ  011000 R\3 1011101
            case 0b1011101: return E50X(atq)(cpu, op);

// CGT  011000 R\3 0010110
            case 0b0010110: return E50X(cgt)(cpu, op);

// CHS  011000 R\3 0100000
            case 0b0100000: return E50X(chs)(cpu, op);

// CMH  011000 R\3 0100101
            case 0b0100101: return E50X(cmh)(cpu, op);

// CMR  011000 R\3 0100100
            case 0b0100100: return E50X(cmr)(cpu, op);

// CR   011000 R\3 0101110
            case 0b0101110: return E50X(cr)(cpu, op);

// CRBL 011000 R\3 0110010
            case 0b0110010: return E50X(crbl)(cpu, op);

// CRBR 011000 R\3 0110011
            case 0b0110011: return E50X(crbr)(cpu, op);

// CRHL 011000 R\3 0101100
            case 0b0101100: return E50X(crhl)(cpu, op);

// CRHR 011000 R\3 0101101
            case 0b0101101: return E50X(crhr)(cpu, op);

// CSR  011000 R\3 0100001
            case 0b0100001: return E50X(csr)(cpu, op);

// DH1  011000 R\3 1011000
            case 0b1011000: return E50X(dh1)(cpu, op);

// DH2  011000 R\3 1011001
            case 0b1011001: return E50X(dh2)(cpu, op);

// DR1  011000 R\3 1010100
            case 0b1010100: return E50X(dr1)(cpu, op);

// DR2  011000 R\3 1010101
            case 0b1010101: return E50X(dr2)(cpu, op);

// INK  011000 R\3 0111000
            case 0b0111000: return E50X(ink)(cpu, op);

// LF   011000 R\3 0001110
            case 0b0001110: return E50X(lf)(cpu, op);

// LT   011000 R\3 0001111
            case 0b0001111: return E50X(lt)(cpu, op);

// LNE  011000 R\3 0000010
            case 0b0000010: return E50X(lne)(cpu, op);

// LEQ  011000 R\3 0000011
            case 0b0000011: return E50X(leq)(cpu, op);

// LGE  011000 R\3 0000100
// LHGE     case 0b0000100: return E50X(lge)(cpu, op);

// LGT  011000 R\3 0000101
            case 0b0000101: return E50X(lgt)(cpu, op);

// LLE  011000 R\3 0000001
            case 0b0000001: return E50X(lle)(cpu, op);

// LLT  011000 R\3 0000000
// LHLT     case 0b0000000: return E50X(llt)(cpu, op);

// LCEQ 011000 R\3 1101011
            case 0b1101011: return E50X(lceq)(cpu, op);

// LCGE 011000 R\3 1101100
            case 0b1101100: return E50X(lcge)(cpu, op);

// LCGT 011000 R\3 1101101
            case 0b1101101: return E50X(lcgt)(cpu, op);

// LCLE 011000 R\3 1101001
            case 0b1101001: return E50X(lcle)(cpu, op);

// LCLT 011000 R\3 1101000
            case 0b1101000: return E50X(lclt)(cpu, op);

// LCNE 011000 R\3 1101010
            case 0b1101010: return E50X(lcne)(cpu, op);

// LHEQ 011000 R\3 0001011
            case 0b0001011: return E50X(lheq)(cpu, op);

// LHGE 011000 R\3 0000100
            case 0b0000100: return E50X(lhge)(cpu, op);

// LHGT 011000 R\3 0001101
            case 0b0001101: return E50X(lhgt)(cpu, op);

// LHLE 011000 R\3 0001001
            case 0b0001001: return E50X(lhle)(cpu, op);

// LHLT 011000 R\3 0000000
            case 0b0000000: return E50X(lhlt)(cpu, op);

// LHNE 011000 R\3 0001010
            case 0b0001010: return E50X(lhne)(cpu, op);

// LFEQ 011000 R\3 001F011
            case 0b0010011:
            case 0b0011011: return E50X(lfeq)(cpu, op);

// LFGE 011000 R\3 001F100
            case 0b0010100:
            case 0b0011100: return E50X(lfge)(cpu, op);

// LFGT 011000 R\3 001F101
            case 0b0010101:
            case 0b0011101: return E50X(lfgt)(cpu, op);

// LFLE 011000 R\3 001F001
            case 0b0010001:
            case 0b0011001: return E50X(lfle)(cpu, op);

// LFLT 011000 R\3 001F000
            case 0b0010000:
            case 0b0011000: return E50X(lflt)(cpu, op);

// LFNE 011000 R\3 001F010
            case 0b0010010:
            case 0b0011010: return E50X(lfne)(cpu, op);

// ICBL 011000 R\3 0110101
            case 0b0110101: return E50X(icbl)(cpu, op);

// ICBR 011000 R\3 0110110
            case 0b0110110: return E50X(icbr)(cpu, op);

// ICHL 011000 R\3 0110000
            case 0b0110000: return E50X(ichl)(cpu, op);

// ICHR 011000 R\3 0110001
            case 0b0110001: return E50X(ichr)(cpu, op);

// IH1  011000 R\3 1010110
            case 0b1010110: return E50X(ih1)(cpu, op);

// IH2  011000 R\3 1010111
            case 0b1010111: return E50X(ih2)(cpu, op);

// IR1  011000 R\3 1010010
            case 0b1010010: return E50X(ir1)(cpu, op);

// IR2  011000 R\3 1010011
            case 0b1010011: return E50X(ir2)(cpu, op);

// IRB  011000 R\3 0110100
            case 0b0110100: return E50X(irb)(cpu, op);

// IRH  011000 R\3 0101111
            case 0b0101111: return E50X(irh)(cpu, op);

// OTK  011000 R\3 0111001
            case 0b0111001: return E50X(otk)(cpu, op);

// PID  011000 R\3 0101010
            case 0b0101010: return E50X(pid)(cpu, op);

// PIDH 011000 R\3 0101011
            case 0b0101011: return E50X(pidh)(cpu, op);

// PIM  011000 R\3 0101000
            case 0b0101000: return E50X(pim)(cpu, op);

// PIMH 011000 R\3 0101001
            case 0b0101001: return E50X(pimh)(cpu, op);

// RBQ  011000 R\3 1011011
            case 0b1011011: return E50X(rbq)(cpu, op);

// RTQ  011000 R\3 1011010
            case 0b1011010: return E50X(rtq)(cpu, op);

// SHL1 011000 R\3 0111110
            case 0b0111110: return E50X(shl1)(cpu, op);

// SHL2 011000 R\3 0111111
            case 0b0111111: return E50X(shl2)(cpu, op);

// SHR1 011000 R\3 1010000
            case 0b1010000: return E50X(shr1)(cpu, op);

// SHR2 011000 R\3 1010001
            case 0b1010001: return E50X(shr2)(cpu, op);

// SL1  011000 R\3 0111010
            case 0b0111010: return E50X(sl1)(cpu, op);

// SL2  011000 R\3 0111011
            case 0b0111011: return E50X(sl2)(cpu, op);

// SR1  011000 R\3 0111100
            case 0b0111100: return E50X(sr1)(cpu, op);

// SR2  011000 R\3 0111101
            case 0b0111101: return E50X(sr2)(cpu, op);

// SSM  011000 R\3 0100010
            case 0b0100010: return E50X(ssm)(cpu, op);

// SSP  011000 R\3 0100011
            case 0b0100011: return E50X(ssp)(cpu, op);

// STEX 011000 R\3 0010111
            case 0b0010111: return E50X(stex)(cpu, op);

// TC   011000 R\3 0100110
            case 0b0100110: return E50X(tc)(cpu, op);

// TCH  011000 R\3 0100111
            case 0b0100111: return E50X(tch)(cpu, op);

// TFLR 011000 R\3 111 FLR 101
            case 0b1110011:
            case 0b1111011: return E50X(tflr)(cpu, op);

// TRFL 011000 R\3 111 FLR 101
            case 0b1110101:
            case 0b1111101: return E50X(trfl)(cpu, op);

// TSTQ 011000 R\3 1000100
            case 0b1000100: return E50X(tstq)(cpu, op);

// LDC  011000 R\3 111 FLR 010
            case 0b1110010:
            case 0b1111010: return E50X(ldc)(cpu, op);

// STC  011000 R\3 111 FLR 110
            case 0b1110110:
            case 0b1111110: return E50X(stc)(cpu, op);

// STCD 011000 R\3 1011111
            case 0b1011111: return E50X(stcd)(cpu, op);

// STCH 011000 R\3 1011111
            case 0b1011110: return E50X(stch)(cpu, op);

// FCM  011000 0F0 1000000
            case 0b1000000: return E50X(fcm)(cpu, op);

// DFCM 011000 0F0 1100100
            case 0b1100100: return E50X(dfcm)(cpu, op);

// FRN  011000 0F0 1000111
            case 0b1000111: return E50X(frn)(cpu, op);

// FRNM 011000 0F0 1100110
            case 0b1100110: return E50X(frnm)(cpu, op);

// FRNP 011000 0F0 1100101
            case 0b1100101: return E50X(frnp)(cpu, op);

// FRNZ 011000 0F0 1100111
            case 0b1100111: return E50X(frnz)(cpu, op);

// DBLE 011000 0F0 1100110
            case 0b1000110: return E50X(dble)(cpu, op);

// FLT  011000 R\3 100F101
            case 0b1000101:
            case 0b1001101: return E50X(flt)(cpu, op);

// FLTH 011000 R\3 100F010
            case 0b1000010:
            case 0b1001010: return E50X(flth)(cpu, op);

// INT  011000 R\3 100F011
            case 0b1000011:
            case 0b1001011: return E50X(int)(cpu, op);

// INTH 011000 R\3 100F001
            case 0b1000001:
            case 0b1001001: return E50X(inth)(cpu, op);

// ICP  011000 R\3 1110111
            case 0b1110111: return E50X(icp)(cpu, op);

// DCP  011000 R\3 1110000
            case 0b1110000: return E50X(dcp)(cpu, op);

// TCNP 011000 R\3 1111000
            case 0b1111000: return E50X(tcnpr)(cpu, op);

    default: return E50X(uii)(cpu, op);
  }
}


static E50I(o36)
{
static const inst_t op_tab[8] = {
  E50X(fd),    // 000 0F0
  E50X(dfd),   // 001 0F1
  E50X(fd),    // 010 0F0
  E50X(dfd),   // 011 0F1
  E50X(qfld),  // 100 100
  E50X(qfst),  // 101 101
  E50X(qfad),  // 110 110
  E50X(qfsb),  // 111 111
};
int opcde = ((op[0] & 0b11) << 1) | (op[1] >> 7);

  return op_tab[opcde](cpu, op);
}


static E50I(o46)
{
static inst_t op_tab[8] = {
  E50X(im),      // 100110 000 
  E50X(pcl),     // 100110 001 
  E50X(ealb),    // 100110 010 
  E50X(zm),      // 100110 011 
  E50X(tm),      // 100110 100 
  E50X(qfmp),    // 100110 101 
  E50X(qfdv),    // 100110 110 
  E50X(qfc)      // 100110 111 
};

int opcde = ((op[0] & 0b11) << 1) | (op[1] >> 7);

  return op_tab[opcde](cpu, op);
}


static E50I(o56)
{
static inst_t op_tab[8] = {
  E50X(imh),     // 101110 000 
  E50X(jmp),     // 101110 001 
  E50X(eaxb),    // 101110 010 
  E50X(zmh),     // 101110 011 
  E50X(tmh),     // 101110 100 
  E50X(uii),     // 101110 101 
  E50X(uii),     // 101110 110 
  E50X(uii)      // 101110 111 
};

int opcde = ((op[0] & 0b11) << 1) | (op[1] >> 7);

  return op_tab[opcde](cpu, op);
}


static E50I(o66)
{
static inst_t op_tab[8] = {
  E50X(dm),      // 110110 000 
  E50X(jsxb),    // 110110 001 
  E50X(uii),     // 110110 010 
  E50X(uii),     // 110110 011 
  E50X(uii),     // 110110 100 
  E50X(uii),     // 110110 101 
  E50X(uii),     // 110110 110 
  E50X(uii)      // 110110 111 
};

int opcde = ((op[0] & 0b11) << 1) | (op[1] >> 7);

  return op_tab[opcde](cpu, op);
}


static E50I(o76)
{
static inst_t op_tab[8] = {
  E50X(dmh),     // 111110 000 
  E50X(uii),     // 111110 001 
  E50X(uii),     // 111110 010 
  E50X(uii),     // 111110 011 
  E50X(uii),     // 111110 100 
  E50X(uii),     // 111110 101 
  E50X(tcnp),    // 111110 110 
  E50X(uii)      // 111110 111 
};

int opcde = ((op[0] & 0b11) << 1) | (op[1] >> 7);

  return op_tab[opcde](cpu, op);
}


E50I(decode)
{
static const inst_t op_tab[64] = {
  E50X(o00),     // 000000
  E50X(l),       // 000001
  E50X(a),       // 000010
  E50X(n),       // 000011
  E50X(lhl1),    // 000100
  E50X(shl),     // 000101
  E50X(o06),     // 000110
  E50X(uii),     // 000111
  E50X(o10),     // 001000
  E50X(lh),      // 001001
  E50X(ah),      // 001010
  E50X(nh),      // 001011
  E50X(lhl2),    // 001100
  E50X(sha),     // 001101
  E50X(o16),     // 001110
  E50X(uii),     // 001111
  E50X(o20),     // 010000
  E50X(st),      // 010001
  E50X(s),       // 010010
  E50X(o),       // 010011
  E50X(rot),     // 010100
  E50X(uii),     // 010101
  E50X(o26),     // 010110
  E50X(uii),     // 010111
  E50X(o30),     // 011000
  E50X(sth),     // 011001
  E50X(sh),      // 011010
  E50X(oh),      // 011011
  E50X(eio),     // 011100
  E50X(lhl3),    // 011101
  E50X(o36),     // 011110
  E50X(uii),     // 011111
  E50X(uii),     // 100000
  E50X(i),       // 100001
  E50X(m),       // 100010
  E50X(x),       // 100011
  E50X(ldar),    // 100100
  E50X(lcc_ccp), // 100101
  E50X(o46),     // 100110
  E50X(uii),     // 100111
  E50X(uii),     // 101000
  E50X(ih),      // 101001
  E50X(mh),      // 101010
  E50X(xh),      // 101011
  E50X(star),    // 101100
  E50X(scc_acp), // 101101
  E50X(o56),     // 101110
  E50X(uii),     // 101111
  E50X(o60),     // 110000
  E50X(c),       // 110001
  E50X(d),       // 110010
  E50X(ear),     // 110011
  E50X(uii),     // 110100
  E50X(lip),     // 110101
  E50X(o66),     // 110110
  E50X(uii),     // 110111
  E50X(uii),     // 111000
  E50X(ch),      // 111001
  E50X(dh),      // 111010
  E50X(jsr),     // 111011
  E50X(uii),     // 111100
  E50X(aip),     // 111101
  E50X(o76),     // 111110
  E50X(uii)      // 111111
};
int opcde = op[0] >> 2;

  return op_tab[opcde](cpu, op);
}
#endif


#ifndef EMDE
 #include __FILE__
#endif
