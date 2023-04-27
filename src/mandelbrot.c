#include "rubberband.h"
#include <complex.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>

#define WIDTH 800
#define BYTES_PER_R8G8B8 3

typedef struct
{
  float rmin;
  float rmax;
  float imin;
  float imax;
} plane_t;

plane_t plane = { -2.0, 2.0, -2.0, 2.0 };

static void add_pixel_picture (GtkPicture *picture, plane_t plane);

static void
scale_view (MbtRubberBand *band,
            gpointer picture)
{
  float plane_width, plane_height, selection_width, selection_height;

  plane_width = plane.rmax - plane.rmin;
  plane_height = plane.imax - plane.imin;
  selection_width = gtk_widget_get_width (GTK_WIDGET (band));
  selection_height = gtk_widget_get_height (GTK_WIDGET (band));

  plane.rmin = plane.rmin + plane_width * mbt_rubber_band_start_x (band) / selection_width;
  plane.imin = plane.imin + plane_height * mbt_rubber_band_start_y (band) / selection_height;
  plane.rmax = plane.rmin + plane_width * mbt_rubber_band_end_x (band) / selection_width;
  plane.imax = plane.imin + plane_height * mbt_rubber_band_end_y (band) / selection_height;

  add_pixel_picture (GTK_PICTURE (picture), plane);
  gtk_widget_queue_draw (GTK_WIDGET (picture));
}

static int *
mandelbrot_set (plane_t plane, int width, int height)
{
  float complex c, z;
  int *mandelbrot;
  clock_t start, end;

  start = clock ();

  mandelbrot = malloc (width * height * sizeof (int));
  memset (mandelbrot, 0, width * height * sizeof (int));

  for (int i = 0; i < width; ++i)
    {
      for (int j = 0; j < height; ++j)
        {
          z = 0;
          c = ((plane.rmax - plane.rmin) * i) / width + plane.rmin + I * (((plane.imax - plane.imin) * j) / height + plane.imin);
          for (int k = 0; k < 100; ++k)
            {
              z = z * z + c;
              if (cabsf (z) > 2.0)
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
add_pixel_picture (GtkPicture *picture, plane_t plane)
{
  GBytes *bytes;
  GdkTexture *texture;
  GByteArray *pixels;
  int *levels;
  guint8 table[6] = { 0, 0, 0, 255, 255, 255 };

  levels = mandelbrot_set (plane, WIDTH, WIDTH);
  pixels = rgb_from_index (levels, WIDTH * WIDTH, table);

  bytes = g_byte_array_free_to_bytes (pixels);
  free (levels);

  texture = gdk_memory_texture_new (WIDTH, WIDTH, GDK_MEMORY_R8G8B8, bytes,
                                    WIDTH * BYTES_PER_R8G8B8);
  gtk_picture_set_paintable (picture, GDK_PAINTABLE (texture));
}

static void
activate (GtkApplication *app, gpointer *userdata __attribute__ ((unused)))
{
  GtkWidget *window;
  GtkWidget *picture, *overlay, *rubber_band;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Mandelbrot Set");
  gtk_window_set_default_size (GTK_WINDOW (window), WIDTH, WIDTH);
  picture = gtk_picture_new ();
  overlay = gtk_overlay_new ();
  rubber_band = mbt_rubber_band_new ();
  g_signal_connect (rubber_band, "selection-complete", G_CALLBACK (scale_view), picture);
  add_pixel_picture (GTK_PICTURE (picture), plane);
  gtk_overlay_set_child (GTK_OVERLAY (overlay), picture);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), rubber_band);
  gtk_window_set_child (GTK_WINDOW (window), overlay);
  gtk_widget_show (window);
}

int
main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.example.mandelbrot",
                             G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
