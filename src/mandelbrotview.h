#ifndef __MBT_MANDELBROTVIEW_H__
#define __MBT_MANDELBROTVIEW_H__

#include "mandelbrotfunction.h"
#include <gtk/gtk.h>
#include <complex.h>


#define MBT_TYPE_MANDELBROT_VIEW (mbt_mandelbrot_view_get_type ())
G_DECLARE_FINAL_TYPE (MbtMandelbrotView, mbt_mandelbrot_view, MBT, MANDELBROT_VIEW, GtkWidget)

GtkWidget *
mbt_mandelbrot_view_new (void);

void
mbt_mandelbrot_view_set_level_func(MbtMandelbrotView *self, MbtLevelFunc fn);

void
mbt_mandelbrot_view_set_colors(MbtMandelbrotView *self, guint8 *colors, int num_colors);

void
mbt_mandelbrot_view_set_resolution (MbtMandelbrotView *self, int xresolution, int yresolution);

void
mbt_mandelbrot_view_set_bounds (MbtMandelbrotView *self, double complex a, double complex b);

void
mbt_mandelbrot_view_set_bounds_relative(MbtMandelbrotView *self, int xmin, int xmax, int ymin, int ymax);
/*
 * TODO: The view should provide some way of zooming back out.
 * For example, assuming that most `set_bounds` opperations
 * are "zoom in" opperations, if the bounds were put on a stack
 * then the following function could pop that stack to zoom
 * back out to the previous view.

mbt_bounds_t
mbt_mandelbrot_view_pop_bounds (void);

*/

#endif /* __MBT_MANDELBROTVIEW_H__ */
