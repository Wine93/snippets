/*=============================================================================
 * Date:           2019-01-15
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    An example to solve the interdependencies of header files
 * =============================================================================*/

/*
 * sub1.h ---
 *   ^       |
 *   |        ---> main.h
 *   v       |
 * sub2.h ---
 */


#include "main.h"


int
main()
{
    sub1_t  s1 = { 1 };
    sub2_t  s2 = { 2 };

    func_sub1(s2);
    func_sub2(s1);

    return 0;
}
