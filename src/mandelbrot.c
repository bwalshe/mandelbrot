#include "mandelbrot.h"
#include "gtk/gtkbinlayout.h"
#include "rubberband.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>

#define BYTES_PER_R8G8B8 3

typedef struct
{
  double complex a;
  double complex b;
} mbt_bounds_t;

struct _MbtMandelbrotView
{
  GtkWidget parent;
  GtkPicture *picture;

  mbt_bounds_t view_bounds;
  int xresolution;
  int yresolution;
};

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
}

void
mbt_mandelbrot_view_init (MbtMandelbrotView *self)
{
  self->picture = GTK_PICTURE (gtk_picture_new ());
  self->view_bounds.a = -2.0 + I * 2.0;
  self->view_bounds.b = 2.0 - I * 2.0;
  self->xresolution = 0;
  self->yresolution = 0;
  gtk_widget_insert_after (GTK_WIDGET (self->picture), GTK_WIDGET (self), NULL);
}

GtkWidget *
mbt_mandelbrot_view_new (void)
{
  return GTK_WIDGET (g_object_new (MBT_TYPE_MANDELBROT_VIEW, NULL));
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

  G_OBJECT_CLASS (mbt_mandelbrot_view_parent_class)->dispose (object);
}

static int *
mandelbrot_set (mbt_bounds_t plane, int width, int height)
{
  double complex c, z, plane_size;
  int *mandelbrot;
  clock_t start, end;

  start = clock ();
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

  end = clock ();
  fprintf (stderr, "Set calculation took %f seconds.\n",
           (double) (end - start) / CLOCKS_PER_SEC);

  return mandelbrot;
}

static GByteArray *
rgb_from_index (int *values, size_t length, guint8 *table)
{
  GByteArray *rgbs;
  rgbs = g_byte_array_sized_new (length * BYTES_PER_R8G8B8);
  for (size_t i = 0; i < length; ++i)
    {
      g_byte_array_append (rgbs, table + values[i] * BYTES_PER_R8G8B8,
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
  guint8 table[6] = { 0, 0, 0, 255, 255, 255 };

  width = self->xresolution;
  height = self->yresolution;

  levels = mandelbrot_set (self->view_bounds, width, height);
  pixels = rgb_from_index (levels, width * height, table);

  bytes = g_byte_array_free_to_bytes (pixels);
  free (levels);

  texture = gdk_memory_texture_new (width, height, GDK_MEMORY_R8G8B8, bytes,
                                    width * BYTES_PER_R8G8B8);
  gtk_picture_set_paintable (self->picture, GDK_PAINTABLE (texture));
}
