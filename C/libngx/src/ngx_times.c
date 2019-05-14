
/*
 * Based on NGINX (core/ngx_times.c):
 *     Copyright (C) Igor Sysoev
 *     Copyright (C) Nginx, Inc.
 */


#include <ngx.h>


static void ngx_gmtime(time_t t, struct tm *tp);
static void ngx_localtime(time_t s, struct tm *tm);


#define NGX_TIME_SLOTS   64

static unsigned int      slot;
static ngx_atomic_t      ngx_time_lock;


volatile ngx_msec_t      ngx_current_msec;
volatile ngx_time_t     *ngx_cached_time;
volatile ngx_str_t       ngx_cached_http_time;
volatile ngx_str_t       ngx_cached_err_log_time;

static ngx_time_t        cached_time[NGX_TIME_SLOTS];
static char              cached_http_time[NGX_TIME_SLOTS]
                                    [sizeof("Mon, 28 Sep 1970 06:00:00 GMT")];
static char              cached_err_log_time[NGX_TIME_SLOTS]
                                    [sizeof("1970/09/28 12:00:00")];


static char  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


void
ngx_time_init()
{
    ngx_cached_time = &cached_time[0];
    ngx_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
    ngx_cached_err_log_time.len = sizeof("1970/09/28 12:00:00") - 1;

    ngx_time_update();
}


void
ngx_time_update()
{
    char            *p0, *p1;
    time_t           sec;
    struct tm        tm, gmt;
    ngx_time_t      *tp;
    unsigned int     msec;
    struct timeval   tv;

    if (!ngx_trylock(&ngx_time_lock)) {
        return;
    }

    gettimeofday(&tv, NULL);

    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;

    ngx_current_msec = (ngx_msec_t) sec * 1000 + msec;

    tp = &cached_time[slot];

    if (tp->sec == sec) {
        tp->msec = msec;
        return;
    }

    if (slot == NGX_TIME_SLOTS - 1) {
        slot = 0;
    } else {
        slot++;
    }

    tp = &cached_time[slot];

    tp->sec = sec;
    tp->msec = msec;

    ngx_gmtime(sec, &gmt);

    /* ngx_cached_http_time */
    p0 = &cached_http_time[slot][0];

    sprintf(p0, "%s, %02d %s %4d %02d:%02d:%02d GMT",
            week[gmt.tm_wday], gmt.tm_mday,
            months[gmt.tm_mon - 1], gmt.tm_year,
            gmt.tm_hour, gmt.tm_min, gmt.tm_sec);

    ngx_localtime(sec, &tm);

    /* cached_err_log_time */
    p1 = &cached_err_log_time[slot][0];

    sprintf(p1, "%4d/%02d/%02d %02d:%02d:%02d",
            tm.tm_year, tm.tm_mon,
            tm.tm_mday, tm.tm_hour,
            tm.tm_min, tm.tm_sec);

    ngx_memory_barrier();

    ngx_cached_time = tp;
    ngx_cached_http_time.data = p0;
    ngx_cached_err_log_time.data = p1;

    ngx_unlock(&ngx_time_lock);
}


static void
ngx_gmtime(time_t t, struct tm *tp)
{
    int           yday;
    unsigned int  sec, min, hour, mday, mon, year, wday, days, leap;

    /* the calculation is valid for positive time_t only */

    if (t < 0) {
        t = 0;
    }

    days = t / 86400;
    sec = t % 86400;

    /*
     * no more than 4 year digits supported,
     * truncate to December 31, 9999, 23:59:59
     */

    if (days > 2932896) {
        days = 2932896;
        sec = 86399;
    }

    /* January 1, 1970 was Thursday */

    wday = (4 + days) % 7;

    hour = sec / 3600;
    sec %= 3600;
    min = sec / 60;
    sec %= 60;

    /*
     * the algorithm based on Gauss' formula,
     * see src/core/ngx_parse_time.c
     */

    /* days since March 1, 1 BC */
    days = days - (31 + 28) + 719527;

    /*
     * The "days" should be adjusted to 1 only, however, some March 1st's go
     * to previous year, so we adjust them to 2.  This causes also shift of the
     * last February days to next year, but we catch the case when "yday"
     * becomes negative.
     */

    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);

    yday = days - (365 * year + year / 4 - year / 100 + year / 400);

    if (yday < 0) {
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
        yday = 365 + leap + yday;
        year--;
    }

    /*
     * The empirical formula that maps "yday" to month.
     * There are at least 10 variants, some of them are:
     *     mon = (yday + 31) * 15 / 459
     *     mon = (yday + 31) * 17 / 520
     *     mon = (yday + 31) * 20 / 612
     */

    mon = (yday + 31) * 10 / 306;

    /* the Gauss' formula that evaluates days before the month */

    mday = yday - (367 * mon / 12 - 30) + 1;

    if (yday >= 306) {

        year++;
        mon -= 10;

        /*
         * there is no "yday" in Win32 SYSTEMTIME
         *
         * yday -= 306;
         */

    } else {

        mon += 2;

        /*
         * there is no "yday" in Win32 SYSTEMTIME
         *
         * yday += 31 + 28 + leap;
         */
    }

    tp->tm_sec = (int) sec;
    tp->tm_min = (int) min;
    tp->tm_hour = (int) hour;
    tp->tm_mday = (int) mday;
    tp->tm_mon = (int) mon;
    tp->tm_year = (int) year;
    tp->tm_wday = (int) wday;
}


static void
ngx_localtime(time_t s, struct tm *tm)
{
    struct tm  *t;

    t = localtime(&s);
    *tm = *t;

    tm->tm_mon++;
    tm->tm_year += 1900;
}
