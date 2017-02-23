/*=============================================================================
 * Date:           2017-08-20
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    Linux native AIO
 * =============================================================================*/


#define _GNU_SOURCE         /* O_DIRECT */
#include <stdio.h>
#include <stdlib.h>         /* aligned_alloc */
#include <stdint.h>         /* uint64_t */
#include <string.h>         /* memset */
#include <fcntl.h>          /* O_DIRECT */
#include <unistd.h>         /* syscall */
#include <sys/syscall.h>    /* __NR_io_setup */
#include <linux/aio_abi.h>  /* aio_context_t */


#define logerr(format) fprintf(stderr, "[ERR] (%s:%d) ", __func__, __LINE__); \
    perror(format)

#define BUFSIZE 1024


static int
io_setup(unsigned nr_events, aio_context_t* ctx_idp)
{
    return syscall(__NR_io_setup, nr_events, ctx_idp);
}


static int
io_destroy(aio_context_t ctx)
{
    return syscall(__NR_io_destroy, ctx);
}


static int
io_submit(aio_context_t ctx, long nr, struct iocb** iocbpp)
{
    return syscall(__NR_io_submit, ctx, nr, iocbpp);
}


static int
io_getevents(aio_context_t ctx, long min_nr, long nr,
	struct io_event* events, struct timespec* timeout)
{
    return syscall(__NR_io_getevents, ctx, min_nr, nr, events, timeout);
}


int
main()
{
    int               fd, rc, ready, n;
    char             *buffer;
    aio_context_t     ctx;
    struct iocb       aiocb, *piocb[1];
    struct io_event   event[1];

    fd = open("aio.txt", O_RDONLY | O_DIRECT);
    if (fd == -1) {
        logerr("open() failed");
        return 1;
    }

    buffer = (char *) aligned_alloc(512, BUFSIZE);
    if (buffer == NULL) {
        logerr("aligned_alloc() failed");
        return 1;
    }

    memset(&ctx, 0, sizeof(ctx));
    rc = io_setup(1024, &ctx);
    if (rc != 0) {
        logerr("io_setup() failed");
        return 1;
    }

    memset(&aiocb, 0, sizeof(aiocb));
    aiocb.aio_lio_opcode = IOCB_CMD_PREAD;
    aiocb.aio_fildes = fd;
    aiocb.aio_offset = 0;
    aiocb.aio_nbytes = 512;  // The bytes you want to read
    aiocb.aio_buf = (uint64_t) buffer;

    piocb[0] = &aiocb;

    rc = io_submit(ctx, 1, piocb);
    if (rc != 1) {
        // TODO: io_destroy
        logerr("io_submit() failed");
        return 1;
    }

    ready = io_getevents(ctx, 1, 1, event, NULL);
    if (ready != 1) {
        logerr("io_getevents() failed");
        return 1;
    }

    n = event[0].res;
    if (n <= 0) {
        logerr("event.res invalid");
        return 1;
    }

    buffer[n] = '\0';
    printf ("read %d bytes: %s", n, buffer);

    io_destroy(ctx);

    return 0;
}


/*
 * 读写每次需对齐，那么 Nginx 是怎么保证读完全部数据的
 * 结合 eventfd
 * */
