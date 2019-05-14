
/*
 * Based on NGINX (os/unix/ngx_process.h):
 *     Copyright (C) Igor Sysoev
 *     Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PROCESS_H_INCLUDED_
#define _NGX_PROCESS_H_INCLUDED_


#include <ngx.h>


typedef void (*ngx_spawn_proc_pt) (void *data);


pid_t ngx_spawn_process(ngx_spawn_proc_pt proc, void *data);

int ngx_init_setproctitle(int argc, char *const *argv);
void ngx_setproctitle(char *title);

int ngx_daemon();


#endif /* _NGX_PROCESS_H_INCLUDED_ */

