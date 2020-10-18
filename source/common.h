/* Common
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


#ifndef _common_h
#define _common_h


#define _GNU_SOURCE

#if defined(EBUG) && !defined(DEBUG)
 #define DEBUG
#endif

#if defined(__SIZEOF_INT128__) && defined(__SIZEOF_FLOAT128__)
 #define FLOAT128 __float128
 #define UINT128  __uint128
 #define INT128   __int128
#endif

#if defined(FLOAT128)
 #define FLOAT __float128
#else
 #define FLOAT double
#endif


#include <stdint.h>
#include <getopt.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <poll.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <setjmp.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <spawn.h>
#include <pthread.h>
#if defined(__APPLE__) || defined(__OSX__)
 #define pthread_yield() sched_yield()
 #define pthread_setname_np(_t, _n) pthread_setname_np(_n)
 extern char **environ;
 #define POSIX_SPAWN_SETSCHEDPARAM (0)
 #define POSIX_SPAWN_SETSCHEDULER (0)
 #define posix_spawnattr_setschedpolicy(_a, _s)
 #define posix_spawnattr_setschedparam(_a, _p) ((_p)->sched_priority)
#endif
#include <dirent.h>
#include <math.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef LIBTELNET
#include <libtelnet.h>
#endif


/* static assertion of structure sizes
 */
#define static_assert(_c, _m) typedef char static_assertion_## _m[(_c)?1:-1]
#define assert_size(_a, _s) static_assert(sizeof(_a) == (_s), _a ## _size_error)


static inline size_t a2i(char *a)
{
int i;
char c;
int n;

  if(a == NULL || *a == '\0')
    return 0;

  n = sscanf(a,"%i%c",&i,&c);

  if(n == 0)
    return 0;

  if(n == 1)
    return i;

  switch(tolower(c)) {
    case 'g':
      i *= 1024;
    case 'm':
      i *= 1024;
    case 'k':
      i *= 1024;
  }
  return i;
}


#if defined(__APPLE__) || defined(__OSX__)
static inline char *basename(char *path)
{
char *fs;
  if((fs = strrchr(path, '/')) && *(++fs))
    return fs;
  else
    return path;
}
#endif

static inline char *c_fname(char *path)
{
  if(!path || !*path)
    return path;

  struct stat st;
  if(!stat(path, &st))
    return path;

  char *bname = basename(path);

  if(!bname || !*bname)
    return path;

  DIR *dir;
  if(bname <= path)
    dir = opendir(".");
  else
  {
    char fs = *(bname - 1);
    *(bname - 1) = '\0';
    dir = opendir(path);
    *(bname - 1) = fs;
  }

  if(!dir)
    return path;

  struct dirent *dirent;
  while((dirent = readdir(dir)))
    if(!strcasecmp(dirent->d_name, bname))
    {
      strcpy(bname, dirent->d_name);
      closedir(dir);
      return path;
    }
  closedir(dir);
  return path;
}


static inline int isfilex(const char *fname)
{
  if(!fname) 
    return ENOENT;

  struct stat st;

  int rc = stat(fname, &st);

  if(!rc && S_ISDIR(st.st_mode))
    rc = errno = EISDIR;

  return rc;
}

static inline int isfile(const char *fname)
{
int rc = isfilex(fname);

  if(rc)
    perror(fname);

  return rc;
}


static inline int isdirx(const char *fname)
{
  if(!fname) 
    return ENOENT;

  struct stat st;

  int rc = stat(fname, &st);

  if(!rc && !S_ISDIR(st.st_mode))
    rc = errno = ENOTDIR;

  return rc;
}

static inline int isdir(const char *fname)
{
int rc = isdirx(fname);

  if(rc)
    perror(fname);

  return rc;
}

#endif
