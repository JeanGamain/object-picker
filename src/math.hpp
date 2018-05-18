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

#define INT_MIN -2147483648
#define INT_MAX 2147483647


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

inline uint32_t	morton2d(uint32_t x, uint32_t y)
{
  static const uint32_t B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
  static const uint32_t S[] = {1, 2, 4, 8};

  x = (x | (x << S[3])) & B[3];
  x = (x | (x << S[2])) & B[2];
  x = (x | (x << S[1])) & B[1];
  x = (x | (x << S[0])) & B[0];
  
  y = (y | (y << S[3])) & B[3];
  y = (y | (y << S[2])) & B[2];
  y = (y | (y << S[1])) & B[1];
  y = (y | (y << S[0])) & B[0];
  
  return (x | (y << 1));
}

inline uint32_t	morton3d(uint32_t x, uint32_t y, uint32_t z)
{
  static const uint32_t B[] = {0x09249249, 0x030c30c3, 0x0300f00f, 0xff0000ff};
  static const uint32_t S[] = {2, 4, 8, 16};

  x = (x | (x << S[3])) & B[3];
  x = (x | (x << S[2])) & B[2];
  x = (x | (x << S[1])) & B[1];
  x = (x | (x << S[0])) & B[0];
  
  y = (y | (y << S[3])) & B[3];
  y = (y | (y << S[2])) & B[2];
  y = (y | (y << S[1])) & B[1];
  y = (y | (y << S[0])) & B[0];
  
  z = (z | (z << S[3])) & B[3];
  z = (z | (z << S[2])) & B[2];
  z = (z | (z << S[1])) & B[1];
  z = (z | (z << S[0])) & B[0];
  
  return (x | (y << 1) | (z << 2));
}

inline uint32_t morton2d(uint32_t x, uint32_t y);

typedef int_fast32_t	cordinate;

#endif /* MATH_HPP_ */
