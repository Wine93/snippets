/*=============================================================================
 * Date:           2018-12-11
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    Glibc function malloc and memory
 * =============================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MALLOC_SIZE 1 * 1024 * 1024 * 1024


void
get_meminfo()
{
    char           buffer[1024];
    unsigned long  vm_size, vm_rss;

    FILE *f = fopen("/proc/self/status", "r");
    if (f == NULL) {
        printf ("fopen() error\n");
        exit(1);
    }

    while (fscanf(f, "%s", buffer) == 1) {

        if (strcmp(buffer, "VmSize:") == 0) {
            fscanf(f, "%ld", &vm_size);

        } else if (strcmp(buffer, "VmRSS:") == 0) {
            fscanf(f, "%ld", &vm_rss);
        }
    }

    printf ("VmSize(KB): %-10ld VmRSS(KB): %-10ld\n", vm_size, vm_rss);
}


int
main()
{
    get_meminfo();

    char *p = (char *) malloc(MALLOC_SIZE);
    if (p == NULL) {
        printf ("malloc error\n");
        exit(1);
    }

    get_meminfo();

    *p = 'A';
    *(p+4096) = 'B';
    *(p+8192) = 'C';

    get_meminfo();

    return 0;
}
