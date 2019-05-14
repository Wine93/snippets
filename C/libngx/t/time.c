#include <ngx.h>
#include <stdio.h>


int
main(int argc, char *const *argv)
{
    ngx_time_t  *tp;

    ngx_time_init();

    for (;;) {
        sleep(1);

        tp = ngx_timeofday();
        fprintf(stderr, "[%d.%d] [%.*s] [%.*s]\n",
                tp->sec, tp->msec,
                ngx_cached_http_time.len, ngx_cached_http_time.data,
                ngx_cached_err_log_time.len, ngx_cached_err_log_time.data);

        ngx_time_update();
    }

    return 0;
}
