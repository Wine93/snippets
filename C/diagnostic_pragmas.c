/*=============================================================================
 * Date:           2017-02-23
 * Author:         wine93 <wine93.info@gmail.com>
 * ComplieFlags:   -Wcast-qual
 * Description:    Using diagnostic pragmas to ignore some complie warming/error,
 *                 See also https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 * =============================================================================*/


int
main()
{
    const char *cstr = "abcdef";

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    char *str = (char*)cstr;  /* Convert const char* to char* */
#pragma GCC diagnostic pop

    return 0;
}

