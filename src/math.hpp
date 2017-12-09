#ifndef MATH_HPP_
# define MATH_HPP_

#include <math.h>
#include <float.h>
#include <stdint.h>

#ifndef M_E
#define M_E            2.7182818284590452354   /* e */
#endif

#ifndef M_LOG2E
#define M_LOG2E        1.4426950408889634074   /* log_2 e */
#endif

#ifndef M_LOG10E
#define M_LOG10E       0.43429448190325182765  /* log_10 e */
#endif

#ifndef M_LN2
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#endif

#ifndef M_LN10
#define M_LN10         2.30258509299404568402  /* log_e 10 */
#endif

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif

#ifndef M_PI_4
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#endif

#ifndef M_1_PI
#define M_1_PI         0.31830988618379067154  /* 1/pi */
#endif

#ifndef M_2_PI
#define M_2_PI         0.63661977236758134308  /* 2/pi */
#endif

#ifndef M_SQRTPI
#define M_SQRTPI       1.77245385090551602729  /* sqrt(pi) */
#endif

#ifndef M_2_SQRTPI
#define M_2_SQRTPI     1.12837916709551257390  /* 2/sqrt(pi) */
#endif

#ifndef M_LNPI
#define M_LNPI         1.14472988584940017414  /* ln(pi) */
#endif

#ifndef M_EULER
#define M_EULER        0.57721566490153286061 /* Euler constant */
#endif

#ifndef M_SQRT2
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */
#endif

#ifndef M_SQRT3
#define M_SQRT3	       1.73205080756887729352  /* sqrt(3) */
#endif

#define FMAX FLT_MAX

#define NFMAX -FLT_MAX

#ifndef RAD
#define	RAD(a) (M_PI * (a) / 180.0)
#endif

#ifndef ABS
#define	ABS(a) ((a < 0) ? (-a) : (a))
#endif

// safe circular value incrementation / decrementation
#ifndef CIRCULAR_ADD 
#define	CIRCULAR_ADD(nb, max, val) (((max - nb) < val) ? (val - (max - nb)) : (nb + val))
#endif

#ifndef CIRCULAR_SUB
#define	CIRCULAR_SUB(nb, max, val) ((nb < val) ? (max - (val - nb)) : (nb - val))
#endif

#ifndef NORM
#define NORM(nb, max, min) ((nb - min) / (max - min))
#endif

#define D2T1(x, y, xlen) (y * xlen + x)

typedef int32_t	cordinate;

#endif /* MATH_HPP_ */
