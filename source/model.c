/* Model
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
 * License Notice:
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


#if !defined(MODEL)

#include "emu.h"

static struct cpumodel_t modeltab[] = {
  { "9950",   15, pmt,  4, cslow,  current, single, 0, 0 }, // DEFAULT MODEL
//     +--------------------------------------------------- Model Name
//     |       +------------------------------------------- Model Number
//     |       |    +-------------------------------------- HMAP or PMT for the 2755, 6350, and 9750 to 9955 II, and above
//     |       |    |   +---------------------------------- Number of register files
//     |       |    |   |    +----------------------------- Consealed stack in high segment
//     |       |    |   |    |        +-------------------- EA formation model for non-indexing instructions
//     |       |    |   |    |        |       +------------ Second instruction stream
//     |       |    |   |    |        |       |     +------ Microcode manufactering level
//     |       |    |   |    |        |       |     |  +--- Microcode engineering level
//     |       |    |   |    |        |       |     |  |
//     V       V    V   V    V        V       V     V  V
  { "400A",    0, hmap, 2, cslow,  earlier, single, 0, 0 },
  { "400B",    1, hmap, 2, cslow,  earlier, single, 0, 0 },
  { "300",     2, hmap, 2, cslow,  earlier, single, 0, 0 },
  { "350",     3, hmap, 2, cslow,  earlier, single, 0, 0 },
  { "450",     4, hmap, 2, cslow,  earlier, single, 0, 0 },
  { "550",     4, hmap, 2, cslow,  earlier, single, 0, 0 },
  { "250II",   4, hmap, 2, cslow,  earlier, single, 0, 0 },
  { "750",     5, hmap, 2, cslow,  p750,    single, 0, 0 },
  { "650",     6, hmap, 2, cslow,  p750,    single, 0, 0 },
  { "150",     7, hmap, 2, cslow,  p750,    single, 0, 0 },
  { "250",     7, hmap, 2, cslow,  p750,    single, 0, 0 },
  { "850",     8, hmap, 2, cslow,  p750,    multi,  0, 0 },
  { "450II",   9, hmap, 2, cslow,  current, single, 0, 0 },
  { "550M",    9, hmap, 2, cslow,  current, single, 0, 0 },
  { "550II",  10, hmap, 2, cslow,  current, single, 0, 0 },
  { "650M",   10, hmap, 2, cslow,  current, single, 0, 0 },
  { "2250",   11, hmap, 2, cslow,  current, single, 0, 0 },
  { "750Y",   12, hmap, 2, cslow,  current, single, 0, 0 },
  { "550Y",   13, hmap, 2, cslow,  current, single, 0, 0 },
  { "850Y",   14, hmap, 2, cslow,  current, multi,  0, 0 },
//{ "9950",   15, pmt,  4, cslow,  current, single, 0, 0 }, // 16M
  { "9650",   16, hmap, 8, cslow,  current, single, 0, 0 }, // 8M
  { "2550",   17, hmap, 8, cslow,  current, single, 0, 0 }, // 8M
  { "9955",   18, pmt,  4, cslow,  current, single, 0, 0 }, // 16M
  { "9750",   19, pmt,  4, cslow,  current, single, 0, 0 }, // 16M
  { "2150",   20, hmap, 2, cslow,  current, single, 0, 0 }, // 8M
  { "2350",   21, hmap, 8, cslow,  current, single, 0, 0 }, // 8M
  { "2655",   22, hmap, 8, cslow,  current, single, 0, 0 }, // 8M
  { "9655",   23, hmap, 8, cslow,  current, single, 0, 0 }, // 8M
  { "9955T",  24, pmt,  2, cslow,  current, single, 0, 0 }, // 32M
  { "2450",   25, hmap, 8, cslow,  current, single, 0, 0 }, // 8M
  { "4050",   26, pmt,  4, cshigh, current, single, 0, 0 }, // 64M
  { "4150",   27, pmt,  4, cshigh, current, single, 0, 0 }, // 64M
  { "6350",   28, pmt,  4, cshigh, current, single, 0, 0 }, // 128M
  { "6550",   29, pmt,  4, cshigh, current, multi,  0, 0 }, // 128M
  { "9955II", 30, pmt,  4, cslow,  current, single, 0, 0 }, // 32M
  { "2755",   31, pmt,  8, cslow,  current, single, 0, 0 }, // 16M
  { "2455",   32, pmt,  8, cslow,  current, single, 0, 0 }, // 16M
  { "5310",   33, pmtx, 4, cshigh, current, single, 0, 0 }, // 512M
  { "9755",   34, pmt,  4, cslow,  current, single, 0, 0 }, // 16M
  { "2850",   35, pmt,  4, cshigh, current, multi,  0, 0 }, // 64M
  { "2950",   36, pmt,  4, cshigh, current, single, 0, 0 }, // 64M
  { "5330",   37, pmtx, 4, cshigh, current, single, 0, 0 }, // 512M
  { "4450",   38, pmt,  4, cslow,  current, single, 0, 0 }, // 32M
  { "5370",   39, pmt,  4, cshigh, current, multi,  0, 0 }, // 512M
  { "6650",   40, pmt,  4, cshigh, current, multi,  0, 0 }, // 128M
  { "6450",   41, pmt,  4, cshigh, current, single, 0, 0 }, // 128M
  { "6150",   42, pmt,  4, cshigh, current, single, 0, 0 }, // 64M
  { "5320",   43, pmtx, 4, cshigh, current, single, 0, 0 }, // 512M
  { "5340",   44, pmtx, 4, cshigh, current, single, 0, 0 }, // 512M
};
static const int nmodels = sizeof(modeltab)/sizeof(*modeltab);

cpumodel_t *get_cpumodel(const char *model)
{
  for(int n = 0; n < nmodels; ++n)
    if(!strcasecmp(model, modeltab[n].name))
      return modeltab + n;
  return NULL;
}

cpumodel_t *default_cpumodel(void)
{
  return modeltab;
}

cpumodel_t *list_cpumodel(const unsigned int n)
{
  if(n < nmodels)
    return modeltab + n;
  else
    return NULL;
}

#endif
