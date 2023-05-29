#include "rubberband.h"
#include <gtk/gtk.h>

static void
print_selection (MbtRubberBand *band,
                 gpointer user_data __attribute__ ((unused)))
{
  printf ("You selected (%d, %d) to  (%d, %d)\n",
          mbt_rubber_band_start_x (band),
          mbt_rubber_band_start_y (band),
          mbt_rubber_band_width (band),
          mbt_rubber_band_height (band));
}

static void
activate (GtkApplication *app,
          gpointer user_data __attribute__ ((unused)))
{
  GtkWidget *window, *band;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

  band = mbt_rubber_band_new ();
  g_signal_connect (band, "selection-complete", G_CALLBACK (print_selection), NULL);
  gtk_window_set_child (GTK_WINDOW (window), band);

  gtk_widget_show (window);
}

int
main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
