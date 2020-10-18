/* TODO
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



#undef  E5SD
#define E5SD(_n) E50I(_n) \
{ \
  logop1(op,"*" #_n); \
  E50X(rxm_check)(cpu); \
  ++cpu->p; \
}

#undef  E51D
#define E51D(_n) E50I(_n) \
{ \
  logop1(op, "*" #_n); \
  E50X(uii_fault)(cpu, 0); \
}

#undef  E52D
#define E52D(_n) E50I(_n) \
{ \
  logop2(op, "*" #_n); \
  E50X(uii_fault)(cpu, 0); \
}

#undef  E5XD
#define E5XD(_n) E50I(_n) \
{ \
  if(op_is_long(op)) \
  { \
    logop2(op, "*" #_n); \
  } \
  else \
    logop1(op, "*" #_n); \
  E50X(uii_fault)(cpu, 0); \
}

E51D(cxcs)
E51D(lwcs)
E51D(mdei)
E51D(mdii)
E51D(mdiw)
E51D(mdrs)
E5XD(mia)
E52D(mib)
E5SD(viry)
E5SD(xvry)
E51D(wcs)

E51D(epmj)
E51D(evmj)
E51D(ermj)
E51D(lpmj)

E51D(epmx)
E51D(evmx)
E51D(ermx)
E51D(lpmx)

E51D(isi)
E51D(osi)
