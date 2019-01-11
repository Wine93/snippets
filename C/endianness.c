/*=============================================================================
 * Date:           2019-01-08
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    Detect system byte ordering
 * =============================================================================*/

#include <stdio.h>


int
main()
{
    int i = 0x11223344;
    char *p = (char *) &i;

    /*
     *   big  endian >>> high-byte(0x11) stored at low-address(p)
     * little endian >>> low-byte(0x44)  stored at low-address(p)
     */
    if (*p == 0x44) {
        printf ("little endian\n");

    } else {
        printf ("big endian\n");
    }

    return 0;
}
