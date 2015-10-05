#ifndef MATH040_H
#define MATH040_H

#ifndef _MATH_H
#include <math.h>
#endif

#ifdef _M68881
#ifdef _M68040
#include <m68881.h>

/*
 *
 * Undefine the definitions made above in m68881.h for the functions
 * to be used from math040.lib instead of generated as in-line code.
 * Note that m68881.h should NOT be included more than once, and not
 * again after this file is included.  (The former restriction is because
 * the compiler treats definitions as if being on a stack.  Thus, if
 * sin is defined twice, and undefined once here..you will still end up
 * using the definition from m68881.h)
 *
 */

#undef acos
#undef asin
#undef atan
#undef cos
#undef sin
#undef tan
#undef cosh
#undef sinh
#undef tanh
#undef log
#undef log10
#undef atan
#endif

#endif
#endif

