/*=============================================================================
 * Date:           2019-03-26
 * Author:         wine93 <wine93.info@gmail.com>
 * ComplieFlags:   -lpthread
 * Description:    Implement lock by gcc(>4.1.2) built-in atomic operation
 * =============================================================================*/


#include "atomic.h"
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <inttypes.h>


#define logerr(format) fprintf(stderr, "[ERR] (%s:%d) ", __func__, __LINE__); \
    perror(format)


typedef void (*plus_proc_pt) (uint64_t *count);


typedef struct {
    int            max_plus;
    uint64_t      *count;
    plus_proc_pt   plus;
} thread_t;


void *
thread_start(void *arg)
{
    int  i;

    thread_t *t = (thread_t *) arg;

    for (i = 0; i < t->max_plus; i++) {
        t->plus(t->count);
    }
}


/*
 * Create max_threads threads, every thread add count to max_plus
 */
void
TEST(char *case_name, int max_threads, int max_plus, plus_proc_pt plus)
{
    int        i, rc;
    time_t     start, end;
    uint64_t   count, expect;
    thread_t   t;
    pthread_t  tid[max_threads + 5];

    count = 0;

    t.max_plus = max_plus;
    t.count = &count;
    t.plus = plus;

    start = clock();

    for (i = 0; i < max_threads; i++) {
        rc = pthread_create(&tid[i], NULL, thread_start, &t);
        if (rc != 0) {
            logerr("pthread_create() failed");
        }
    }

    for (i = 0; i < max_threads; i++) {
        rc = pthread_join(tid[i], NULL);
        if (rc != 0) {
            logerr("pthread_join() failed");
        }
    }

    end = clock();

    expect = (uint64_t) max_threads * max_plus;

    /*
     * TEST_NORMAL:
     *     [✘] [0.53s]: (10 * 10000000 = 100000000) != 42687521
     */
    printf ("%s:\n    [%s] [%.2lfs]: (%d * %d = %"PRIu64") %s %"PRIu64"\n",
            case_name,
            count == expect ? "√" : "✘",
            (double) (end - start) / CLOCKS_PER_SEC,
            max_threads,
            max_plus,
            expect,
            count == expect ? "==" : "!=",
            count);
}


/*
 * TEST CASE
 */
static ngx_atomic_t  lock;


void
normal(uint64_t *count)
{
    (*count)++;
}


void
atomic_add(uint64_t *count)
{
    ngx_atomic_fetch_add(count, 1);
}


void
trylock(uint64_t *count)
{
    for ( ;; ) {

        if (ngx_trylock(&lock)) {
            (*count)++;
            ngx_unlock(&lock);
            break;
        }
    }

}


void
trylock_sleep(uint64_t *count)
{
    for ( ;; ) {

        if (ngx_trylock(&lock)) {
            (*count)++;
            ngx_unlock(&lock);
            break;
        }

        usleep(1);
    }
}


int
main()
{
    int  max_threads, max_plus;

    lock = 0;
    max_threads = 10;
    max_plus = 10000000;

    TEST("TEST_NORMAL", max_threads, max_plus, normal);
    TEST("TEST_ATOMIC_ADD", max_threads, max_plus, atomic_add);
    TEST("TEST_TRYLOCK", max_threads, max_plus, trylock);
    TEST("TEST_TRYLOCK_SLEEP", max_threads, max_plus, trylock_sleep);

    /*
     * Sample output:
     *
     *     TEST_NORMAL:
     *         [✘] [0.53s]: (10 * 10000000 = 100000000) != 42687521
     *     TEST_ATOMIC_ADD:
     *         [√] [0.90s]: (10 * 10000000 = 100000000) == 100000000
     *     TEST_TRYLOCK:
     *         [√] [6.78s]: (10 * 10000000 = 100000000) == 100000000
     *     TEST_TRYLOCK_SLEEP:
     *         [√] [1.31s]: (10 * 10000000 = 100000000) == 100000000
     */

    return 0;
}
