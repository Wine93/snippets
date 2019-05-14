
/*
 * Based on NGINX (os/unix/ngx_process.c):
 *     Copyright (C) Igor Sysoev
 *     Copyright (C) Nginx, Inc.
 */


#include <ngx.h>


extern char **environ;

static char **ngx_os_argv;
static char *ngx_os_argv_last;


pid_t
ngx_spawn_process(ngx_spawn_proc_pt proc, void *data)
{
    pid_t  pid;

    pid = fork();

    switch (pid) {

    case -1:
        return -1;

    case 0:
        proc(data);
        break;

    default:
        break;
    }

    return pid;
}


int
ngx_init_setproctitle(int argc, char *const *argv)
{
    int      i;
    char    *p;
    size_t   size;

    size = 0;

    for (i = 0; environ[i]; i++) {
        size += strlen(environ[i]) + 1;
    }

    p = malloc(size);
    if (p == NULL) {
        return -1;
    }

    ngx_os_argv = (char **) argv;
    ngx_os_argv_last = ngx_os_argv[0];

    for (i = 0; ngx_os_argv[i]; i++) {
        if (ngx_os_argv_last == ngx_os_argv[i]) {
            ngx_os_argv_last = ngx_os_argv_last + strlen(ngx_os_argv[i]) + 1;
        }
    }

    for (i = 0; environ[i]; i++) {
        if (ngx_os_argv_last == environ[i]) {

            size = strlen(environ[i]) + 1;
            ngx_os_argv_last = environ[i] + size;

            strncpy(p, environ[i], size);
            environ[i] = p;
            p += size;
        }
    }

    /* Ensure the last char is '\0' when title too long */
    ngx_os_argv_last--;

    return 0;
}


void
ngx_setproctitle(char *title)
{
    ngx_os_argv[1] = NULL;

    strncpy(ngx_os_argv[0], title, ngx_os_argv_last - ngx_os_argv[0]);
}


int
ngx_daemon()
{
    int  fd;

    switch (fork()) {
    case -1:
        return -1;

    case 0:
        break;

    default:
        exit(0);
    }

    if (setsid() == -1) {
        return -1;
    }

    umask(0);

    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        return -1;
    }

    if (fd > STDERR_FILENO) {
        if (close(fd) == -1) {
            return -1;
        }
    }

    return 0;
}
