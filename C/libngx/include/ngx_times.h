
/*
 * Based on NGINX (core/ngx_times.h):
 *     Copyright (C) Igor Sysoev
 *     Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_TIMES_H_INCLUDED_
#define _NGX_TIMES_H_INCLUDED_


#include <ngx.h>


typedef unsigned int  ngx_msec_t;

typedef struct {
    time_t        sec;
    unsigned int  msec;
    int           gmtoff;
} ngx_time_t;


void ngx_time_init();
void ngx_time_update();


extern volatile ngx_time_t  *ngx_cached_time;

#define ngx_time()           ngx_cached_time->sec
#define ngx_timeofday()      (ngx_time_t *) ngx_cached_time

extern volatile ngx_str_t    ngx_cached_http_time;
extern volatile ngx_str_t    ngx_cached_err_log_time;


#endif /* _NGX_TIMES_H_INCLUDED_ */

