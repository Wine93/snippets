/*=============================================================================
 * Date:           2017-11-01
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    Memcpy between the two different type values
 * =============================================================================*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>


int
main()
{
    int     dst;
    u_char  src[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    memcpy((u_char *) &dst, src, sizeof(int));

    /*
     * Int:        4         3         2         1
     * Bin: 00000100  00000011  00000010  00000001
     */
    printf ("%zd %zd %d\n", sizeof(u_char), sizeof(int), dst); // 1 4 67305985(100 00000011 00000010 00000001)

    return 0;
}

