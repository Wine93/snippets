/*=============================================================================
 * Date:           2018-10-01
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    Structure memory alignment
 * =============================================================================*/

#include <stdio.h>


struct x {
    int       a;
    unsigned  u1:1;
    int       b;
    unsigned  u2:1;
};


struct y {
    int       a;
    int       b;
    unsigned  u1:1;
    unsigned  u2:1;
};


int
main()
{
    printf ("sizeof(struct x): %d\n", sizeof(struct x));  // 16
    printf ("sizeof(struct y): %d\n", sizeof(struct y));  // 12

    return 0;
}
