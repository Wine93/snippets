/*=============================================================================
 * Date:           2017-09-07
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    How to create daemon
 * =============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


static int
ngx_daemon()
{
    int  fd;

    switch (fork()) {
    case -1:
        fprintf(stderr, "fork() failed\n");
        return -1;

    case 0:  // child
        break;

    default:  // parent
        exit(0);
    }

    if (setsid() == -1) {
        fprintf(stderr, "setsid() failed\n");
        return -1;
    }

    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "open(\"/dev/null\") failed\n");
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
        fprintf(stderr, "dup2(STDIN) failed");
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        fprintf(stderr, "dup2(STDOUT) failed");
        return -1;
    }

    if (dup2(fd, STDERR_FILENO) == -1) {
        fprintf(stderr, "dup2(STDERR) failed");
        return -1;
    }

    return 0;
}


int
main()
{
    ngx_daemon();

    for ( ;; ) {
        // do something for child
    }

    return 0;
}
