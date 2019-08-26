/*=============================================================================
 * Date:           2019-08-26
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    Flexible array members in a structure in C
 * =============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    int   n1;
    int   n2;
} s1;


typedef struct{
    int   n1;
    int   n2;
    char  buf[];
} s2;


int
main()
{
    printf ("sizeof(s1) = %zu\n", sizeof(s1)); // sizeof(s1) = 8
    printf ("sizeof(s2) = %zu\n", sizeof(s2)); // sizeof(s2) = 8

    // Usage of FAM (Flexible Array Members)
    s2 *s = malloc(sizeof(s2) + sizeof("hello") + 1);

    s->n1 = 1;
    s->n2 = 2;
    memcpy(s->buf, "hello", sizeof("hello"));
    s->buf[5] = '\0';

    // n1 = 1, n2 = 2, buf = "hello"
    printf ("n1 = %d, n2 = %d, buf = \"%s\"\n", s->n1, s->n2, s->buf);

    return 0;
}
