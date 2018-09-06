/*=============================================================================
 * Date:           2018-08-21
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    How does the variable of the child process change
 *                 when the parent process changes the variable?
 * =============================================================================*/


#include <stdio.h> /* fprintf */
#include <unistd.h> /* fork exit sleep */


int global = 1;
int static_global = 2;


int
main()
{
    int local = 3;
    int static_local = 4;

    switch(fork()) {

    case -1:
        fprintf(stderr, "fork() error\n");
        break;

    case 0:  // child
        fprintf(stdout, "%-8s global=%d static_global=%d local=%d static_local=%d\n",
                "[child]", global, static_global, local, static_local);
        fprintf(stdout, "%-8s waiting for parent reassign variable...\n", "[child]");

        sleep(2);

        fprintf(stdout, "%-8s after parent reassign variable!\n", "[child]");
        fprintf(stdout, "%-8s global=%d static_global=%d local=%d static_local=%d\n",
                "[child]", global, static_global, local, static_local);

        break;

    default:  // parent
        fprintf(stdout, "%-8s global=%d static_global=%d local=%d static_local=%d\n",
                "[parent]", global, static_global, local, static_local);

        sleep(1);

        global++;
        static_global++;
        local++;
        static_local++;

        fprintf(stdout, "%-8s add ont to all variable!\n", "[parent]");

        sleep(1);
    }

    return 0;
}
