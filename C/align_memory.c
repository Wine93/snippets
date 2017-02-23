/*=============================================================================
 * Date:           2017-07-21
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    Align memory (pointer address or size)
 * =============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>  /* uintptr_t */

#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


int
main()
{
    u_char *p = (u_char *) malloc(1);
    printf ("%p -> %p\n", p, ngx_align_ptr(p, 16));

    printf ("4000 -> %d\n", ngx_align(4000, 4096));  /* 4000 -> 4096 */
    printf ("5000 -> %d\n", ngx_align(5000, 4096));  /* 5000 -> 8192 */

    return 0;
}
