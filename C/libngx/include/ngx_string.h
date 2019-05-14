
/*
 * Based on NGINX (core/ngx_string.h):
 *     Copyright (C) Igor Sysoev
 *     Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_STRING_H_INCLUDED_
#define _NGX_STRING_H_INCLUDED_


#include <ngx.h>


typedef struct {
    size_t   len;
    char    *data;
} ngx_str_t;


#endif /* _NGX_STRING_H_INCLUDED_ */

