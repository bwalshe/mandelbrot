#include "mandelbrotview.h"
#include "rubberband.h"
#include <gtk/gtk.h>

#define WIDTH 800
#define HEIGHT 800

static void
print_calc_time (float seconds)
{
  printf ("Calculation took %f seconds\n", seconds);
}

static void
scale_view (MbtRubberBand *band,
            gpointer mandelbrot)
{
  int xmin, ymin, width, height;

  width = mbt_rubber_band_width (band);
  height = mbt_rubber_band_height (band);
  xmin = mbt_rubber_band_start_x (band);
  ymin = mbt_rubber_band_start_y (band);
  if (width < 0)
    {
      width = -width;
      xmin = xmin - width;
    }
  if (height < 0)
    {
      height = -height;
      ymin = ymin - height;
    }
  mbt_mandelbrot_view_set_bounds_relative (MBT_MANDELBROT_VIEW (mandelbrot),
                                           xmin, ymin, width, height);

  gtk_widget_queue_draw (GTK_WIDGET (mandelbrot));
}

static void
activate (GtkApplication *app, gpointer *userdata __attribute__ ((unused)))
{
  GtkWidget *window;
  GtkWidget *picture, *overlay, *rubber_band;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Mandelbrot Set");
  gtk_window_set_default_size (GTK_WINDOW (window), WIDTH, HEIGHT);
  picture = mbt_mandelbrot_view_new ();
  mbt_mandelbrot_view_set_resolution (MBT_MANDELBROT_VIEW (picture), WIDTH, HEIGHT);
  overlay = gtk_overlay_new ();
  rubber_band = mbt_rubber_band_new ();
  g_signal_connect (rubber_band, "selection-complete", G_CALLBACK (scale_view), picture);
  g_signal_connect (picture, "calculation-time", G_CALLBACK (print_calc_time), NULL);
  gtk_overlay_set_child (GTK_OVERLAY (overlay), picture);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), rubber_band);
  gtk_window_set_child (GTK_WINDOW (window), overlay);
  gtk_window_present (GTK_WINDOW (window));
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
