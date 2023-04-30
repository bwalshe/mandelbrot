#include "mandelbrotfunction.h"
#include <memory.h>
#include <stdlib.h>

int *
generate_mandelbrot_set (mbt_bounds_t plane, int width, int height)
{
  double complex c, z, plane_size;
  int *mandelbrot;

  plane_size = plane.b - plane.a;

  mandelbrot = malloc (width * height * sizeof (int));
  memset (mandelbrot, 0, width * height * sizeof (int));

  for (int i = 0; i < width; ++i)
    {
      for (int j = 0; j < height; ++j)
        {
          z = 0;
          c = (creal (plane_size) * i) / width + creal (plane.a) + I * ((cimag (plane_size) * j) / height + cimag (plane.a));
          for (int k = 0; k < 100; ++k)
            {
              z = z * z + c;
              if (cabs (z) > 2.0)
                {
                  mandelbrot[i + j * width] = 1;
                  break;
                }
            }
        }
    }

  return mandelbrot;
}
