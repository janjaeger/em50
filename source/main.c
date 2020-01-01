/* main
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


#include "emu.h"  // Emulator Common

#include "cmd.h"

#define RCFILE "%s/rc"

int main(int argc, char *argv[], char **envp)
{
static const char short_options[] =
   "c:"
   "p:"
   "s:"
   "b:"
   "l:"
   "i:"
   "o:"
#ifdef DEBUG
   "v"
#endif
   "h?";

static const struct option long_options[] = {
  {"config",                  1, 0, 'c'},
  {"path",                    1, 0, 'p'},
  {"storage",                 1, 0, 's'},
  {"bind",                    1, 0, 'b'},
  {"port",                    1, 0, 'l'},
  {"pncbind",                 1, 0, 'i'},
  {"pncport",                 1, 0, 'o'},
#ifdef DEBUG
  {"verbose",                 2, 0, 'v'},
#endif
  {"help",                    0, 0, 'h'},
  {0, 0, 0, 0}
};

char c;

sys_t sys = { .physsize = physsize_default, .hdir = hdir_default,
              .serial = { 'F', 'N', ' ', ' ', ' ', ' ', ' ', ' ', '0', '1', '2', '3', '4', '5', ' ', ' ' } };

  while((c = getopt_long(argc, argv, short_options, long_options, NULL)) != (char)-1)
  {
    switch (c) {

      case 'c':
        sys.rcfile = isfile(optarg) ? "" : optarg;
        break;

      case 's':
        sys.physsize = a2i(optarg);
        break;

      case 'p':
        if(!isdir(optarg))
          sys.hdir = strdup(optarg);
        break;

      case 'b':
        sys.bind = optarg;
        break;

      case 'l':
        sys.port = optarg;
        break;

      case 'i':
        sys.pncbind = optarg;
        break;

      case 'o':
        sys.pncport = optarg;
        break;

#ifdef DEBUG
      case 'v':
        sys.verbose = 1;
        if(optarg)
          sys.trace = fopen(optarg, "w");
        break;
#endif

      case 'h':
      case '?':
        fprintf(stderr, "Valid options are:\n");
        for(int n = 0; long_options[n].name; ++n)
          fprintf(stderr, "  -%c --%s\n", long_options[n].val, long_options[n].name);
        exit(c == '?' ? EXIT_FAILURE : EXIT_SUCCESS);
        break;

      default:
        exit(EXIT_FAILURE);
    }
  }

  struct stat st;
  int strc = stat(sys.hdir, &st);
  if(strc)
    strc = errno;
  int mkrc = 0;
  if(strc == ENOENT)
    mkrc = mkdir(sys.hdir, S_IRWXU);
  if((strc != 0 && mkrc != 0) || (strc == 0 && !S_ISDIR(st.st_mode)))
  {
    errno = (strc == ENOENT) ? errno : ENOTDIR;
    perror(sys.hdir);
    exit(EXIT_FAILURE);
  }

  if(!sys.rcfile)
  {
  char rcfile[PATH_MAX];
    snprintf(rcfile, sizeof(rcfile), RCFILE, sys.hdir);
    sys.rcfile = strdup(rcfile);
  }

  if(sys.physsize < physsize_min)
    sys.physsize = physsize_min;
  else
    if(sys.physsize > physsize_max)
      sys.physsize = physsize_max;

  sys.physsize = (sys.physsize + em50_pgoc_offm) & em50_pgoc_mask;

  if((sys.physstor = mmap(NULL, sys.physsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)) == MAP_FAILED)
  {
    fprintf(stderr, "mmap(physstor) failed rc=%d: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  cpu_t cpu = { .sys = &sys, .maxmem = sys.physsize >> 1 };

  em50_init(&cpu);

  if(sys.rcfile && *sys.rcfile)
    cmd_mainrc(&cpu, sys.rcfile);

  exit(cmd_main(&cpu));
}
