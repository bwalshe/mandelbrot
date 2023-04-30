#include "mandelbrotview.h"
#include "mandelbrotfunction.h"
#include "rubberband.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BYTES_PER_R8G8B8 3

struct _MbtMandelbrotView
{
  GtkWidget parent;
  GtkPicture *picture;
  MbtLevelFunc generator;

  mbt_bounds_t view_bounds;
  int color_range;
  guint8 *colors;
  int xresolution;
  int yresolution;
};

static guint8 default_colors[2 * BYTES_PER_R8G8B8] = { 0, 0, 0, 255, 255, 255 };
static guint mbt_calculation_time_signal;

G_DEFINE_TYPE (MbtMandelbrotView, mbt_mandelbrot_view, GTK_TYPE_WIDGET)

static void
draw_mandelbrot_to_picture (MbtMandelbrotView *self);

static void
mbt_mandelbrot_view_snapshot (GtkWidget *widget, GtkSnapshot *snapshot);

static void
mbt_mandelbrot_view_dispose (GObject *object);

void
mbt_mandelbrot_view_class_init (MbtMandelbrotViewClass *class)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  widget_class->snapshot = mbt_mandelbrot_view_snapshot;
  object_class->dispose = mbt_mandelbrot_view_dispose;
  gtk_widget_class_set_layout_manager_type (widget_class, gtk_bin_layout_get_type ());
  mbt_calculation_time_signal = g_signal_new (
      "calculation-time", G_TYPE_FROM_CLASS (G_OBJECT_CLASS (class)),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 1, G_TYPE_FLOAT);
}

void
mbt_mandelbrot_view_init (MbtMandelbrotView *self)
{
  self->picture = GTK_PICTURE (gtk_picture_new ());
  self->view_bounds.a = -2.0 + I * 2.0;
  self->view_bounds.b = 2.0 - I * 2.0;
  self->xresolution = 0;
  self->yresolution = 0;
  self->color_range = 0;
  self->colors = NULL;
  self->generator = generate_mandelbrot_set;
  mbt_mandelbrot_view_set_colors (self, default_colors, 2);
  gtk_widget_insert_after (GTK_WIDGET (self->picture), GTK_WIDGET (self), NULL);
}

GtkWidget *
mbt_mandelbrot_view_new (void)
{
  return GTK_WIDGET (g_object_new (MBT_TYPE_MANDELBROT_VIEW, NULL));
}

void
mbt_mandelbrot_view_set_level_func (MbtMandelbrotView *self, MbtLevelFunc fn)
{
  self->generator = fn;
}

void
mbt_mandelbrot_view_set_colors (MbtMandelbrotView *self, guint8 *colors, int num_colors)
{
  size_t buffer_size = BYTES_PER_R8G8B8 * num_colors * sizeof (guint8);
  if (self->colors)
    free (self->colors);
  self->colors = malloc (buffer_size);
  memcpy (self->colors, colors, buffer_size);
  self->color_range = num_colors;
}

void
mbt_mandelbrot_view_set_bounds (MbtMandelbrotView *self, double complex a, double complex b)
{
  self->view_bounds.a = a;
  self->view_bounds.b = b;
  draw_mandelbrot_to_picture (self);
  gtk_widget_queue_draw (GTK_WIDGET (self->picture));
}

void
mbt_mandelbrot_view_set_bounds_relative (MbtMandelbrotView *self, int xmin, int ymin, int xwidth, int ywidth)
{
  double xscale, yscale;
  double complex plane_size, cornor;
  cornor = self->view_bounds.a;
  plane_size = self->view_bounds.b - self->view_bounds.a;
  xscale = creal (plane_size) / gtk_widget_get_width (GTK_WIDGET (self));
  yscale = cimag (plane_size) / gtk_widget_get_height (GTK_WIDGET (self));

  mbt_mandelbrot_view_set_bounds (self,
                                  cornor + xmin * xscale + I * ymin * yscale,
                                  cornor + (xmin + xwidth) * xscale + I * (ymin + ywidth) * yscale);
}

void
mbt_mandelbrot_view_set_resolution (MbtMandelbrotView *self, int xresolution, int yresolution)
{
  self->xresolution = xresolution;
  self->yresolution = yresolution;
  draw_mandelbrot_to_picture (self);
  gtk_widget_queue_draw (GTK_WIDGET (self->picture));
}

static void
mbt_mandelbrot_view_snapshot (GtkWidget *widget, GtkSnapshot *snapshot)
{
  MbtMandelbrotView *self = MBT_MANDELBROT_VIEW (widget);
  gtk_widget_snapshot_child (widget, GTK_WIDGET (self->picture), snapshot);
}

static void
mbt_mandelbrot_view_dispose (GObject *object)
{
  MbtMandelbrotView *self = MBT_MANDELBROT_VIEW (object);
  GtkWidget *w = GTK_WIDGET (self->picture);
  g_clear_pointer (&w, gtk_widget_unparent);
  if (self->colors)
    free (self->colors);
  G_OBJECT_CLASS (mbt_mandelbrot_view_parent_class)->dispose (object);
}

static GByteArray *
rgb_from_index (int *values, size_t length, guint8 *table, int color_range)
{
  int clipped_value;
  GByteArray *rgbs;
  rgbs = g_byte_array_sized_new (length * BYTES_PER_R8G8B8);
  for (size_t i = 0; i < length; ++i)
    {
      clipped_value = values[i];
      if (clipped_value >= color_range)
        {
          clipped_value = color_range - 1;
        }
      else if (clipped_value < 0)
        {
          clipped_value = 0;
        }
      g_byte_array_append (rgbs, table + clipped_value * BYTES_PER_R8G8B8,
                           BYTES_PER_R8G8B8);
    }
  return rgbs;
}

static void
draw_mandelbrot_to_picture (MbtMandelbrotView *self)
{
  int width, height;
  GBytes *bytes;
  GdkTexture *texture;
  GByteArray *pixels;
  int *levels;
  clock_t start, end;

  if (self->generator == NULL)
    return;

  width = self->xresolution;
  height = self->yresolution;

  start = clock ();
  levels = self->generator (self->view_bounds, width, height);
  end = clock ();
  g_signal_emit (self, mbt_calculation_time_signal, 0, (1.0 * end - start) / CLOCKS_PER_SEC);

  pixels = rgb_from_index (levels, width * height, self->colors, self->color_range);

  bytes = g_byte_array_free_to_bytes (pixels);
  free (levels);

  texture = gdk_memory_texture_new (width, height, GDK_MEMORY_R8G8B8, bytes,
                                    width * BYTES_PER_R8G8B8);
  gtk_picture_set_paintable (self->picture, GDK_PAINTABLE (texture));
}
