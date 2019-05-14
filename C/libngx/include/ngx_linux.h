
/*
 * Based on NGINX (os/unix/ngx_linux_config.h):
 *     Copyright (C) Igor Sysoev
 *     Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LINUX_H_INCLUDED_
#define _NGX_LINUX_H_INCLUDED_


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>             /* exit malloc */
#include <unistd.h>             /* dup2 fork */
#include <fcntl.h>              /* O_RDWR */
#include <time.h>               /* tm */
#include <sys/time.h>           /* gettimeofday */
#include <sys/stat.h>           /* open */


#endif /* _NGX_LINUX_H_INCLUDED_ */

