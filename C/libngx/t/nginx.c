#include <ngx.h>
#include <stdio.h>


void
worker(void *data)
{
    ngx_setproctitle("nginx: worker process");

    for (;;) {
        // do something for worker process
        sleep(1);
    }
}


int
main(int argc, char *const *argv)
{
    int   i;
    char  name[1024];

    ngx_daemon();

    ngx_init_setproctitle(argc, argv);

    sprintf (name, "nginx: master process %s", argv[0]);
    ngx_setproctitle(name);

    for (i = 0; i < 10; i++) {
        ngx_spawn_process(worker, NULL);
    }

    for (;;) {
        // do something for master process
        sleep(1);
    }

    return 0;
}
