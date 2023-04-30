#ifndef __MBT_MANDELBROT_FUNCTION_H__

#define __MBT_MANDELBROT_FUNCTION_H__

#include <complex.h>

typedef struct
{
  double complex a;
  double complex b;
} mbt_bounds_t;

typedef int * (*MbtLevelFunc) (mbt_bounds_t plane, int width, int height);

int *
generate_mandelbrot_set (mbt_bounds_t plane, int width, int height);

#endif /* __MBT_MANDELBROT_FUNCTION_H__ */
