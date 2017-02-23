/*=============================================================================
 * Date:           2017-02-23
 * Author:         wine93 <wine93.info@gmail.com>
 * Description:    Determine the number of bits that an integer has using the preprocessor.
 * =============================================================================*/

#include <limits.h> /* INT_MAX */

#if INT_MAX == 32767
    #define INTEGER_SIZE 0x0010 /* 16 bits */
#elif INT_MAX == 2147483647
    #define INTEGER_SIZE 0x0100 /* 32 bits */
#elif INT_MAX == 9223372036854775807
    #define INTEGER_SIZE 0x1000 /* 64 bits */
#else
    #error "Failed to determine the size of an integer"
#endif


int
main()
{
#if INTEGER_SIZE & 0x0010   /* 16 bits */
    /* do something */
#elif INTEGER_SIZE & 0x0100 /* 32 bits */
    /* do something */
#elif INTEGER_SIZE & 0x1000 /* 64 bits */
    /* do something */
#endif

    return 0;
}
