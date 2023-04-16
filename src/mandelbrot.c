#include <complex.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>

#define WIDTH 800
#define BYTES_PER_R8G8B8 3

static cairo_surface_t *surface = NULL;
static int start_x, start_y;


static void
clear_surface ()
{
  cairo_t *cr;

  cr = cairo_create (surface);
  cairo_set_source_rgba (cr, 0, 0, 0, 0);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  cairo_destroy (cr);
}

static void
draw_selection (GtkWidget *widget,
            double     x,
            double     y)
{
  cairo_t *cr;

  cr = cairo_create (surface);

  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_rectangle (cr, start_x, start_y, x, y);
  cairo_stroke(cr);
  cairo_destroy (cr);

  gtk_widget_queue_draw (widget);
}

static void
drag_begin (GtkGestureDrag *gesture __attribute__((unused)),
            double          x __attribute__((unused)),
            double          y __attribute__((unused)),
            GtkWidget      *area __attribute__((unused)))
{
  start_x = x;
  start_y = y;

  clear_surface ();
}

static void
drag_update (GtkGestureDrag *gesture __attribute__((unused)),
             double          x,
             double          y,
             GtkWidget      *area)
{
  clear_surface ();
  draw_selection (area, x, y);
}

static void
drag_end (GtkGestureDrag *gesture __attribute__((unused)),
          double          x __attribute__((unused)),
          double          y __attribute__((unused)),
          GtkWidget      *area)
{
  clear_surface ();
  gtk_widget_queue_draw (area);
}

static int *mandelbrot_set(float rmin, float rmax, float imin, float imax, int width, int height) {
  float complex c, z;
  int *mandelbrot;
  clock_t start, end;

  start = clock();

  mandelbrot = malloc(width * height * sizeof(int));
  memset(mandelbrot, 0, width * height * sizeof(int));

  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < height; ++j) {
      z = 0;
      c = ((rmax - rmin) * i) / width + rmin + I * (((imax - imin) * j) / height + imin);
      for (int k = 0; k < 100; ++k) {
        z = z * z + c;
        if (cabsf(z) > 2.0) {
          mandelbrot[i + j * width] = 1;
          break;
        }
      }
    }
  }

  end = clock();
  fprintf(stderr, "Set calculation took %f seconds.\n",
          (double)(end - start) / CLOCKS_PER_SEC);

  return mandelbrot;
}

/* Create a new surface of the appropriate size to store our scribbles */
static void
resize_cb (GtkWidget *widget,
           int        width __attribute__((unused)),
           int        height __attribute__((unused)),
           gpointer   data __attribute__((unused)))
{
  if (surface)
    {
      cairo_surface_destroy (surface);
      surface = NULL;
    }

  if (gtk_native_get_surface (gtk_widget_get_native (widget)))
    {
      surface = gdk_surface_create_similar_surface (gtk_native_get_surface (gtk_widget_get_native (widget)),
                                                    CAIRO_CONTENT_COLOR_ALPHA,
                                                    gtk_widget_get_width (widget),
                                                    gtk_widget_get_height (widget));

      clear_surface ();
    }
}


static void
draw_cb (GtkDrawingArea *drawing_area __attribute__((unused)),
         cairo_t        *cr,
         int             width __attribute__((unused)),
         int             height __attribute__((unused)),
         gpointer        data __attribute__((unused)))
{
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);
}


static GByteArray *rgb_from_index(int *values, size_t length, guint8 *table) {
  GByteArray *rgbs;
  rgbs = g_byte_array_sized_new(length * BYTES_PER_R8G8B8);
  for (size_t i = 0; i < length; ++i) {
    g_byte_array_append(rgbs, table + values[i] * BYTES_PER_R8G8B8,
                        BYTES_PER_R8G8B8);
  }
  return rgbs;
}

static void add_pixel_picture(GtkPicture *picture) {
  GBytes *bytes;
  GdkTexture *texture;
  GByteArray *pixels;
  int *levels;
  guint8 table[6] = {0, 0, 0, 255, 255, 255};

  levels = mandelbrot_set(-2.0, 2.0, -2.0, 2.0, WIDTH, WIDTH);
  pixels = rgb_from_index(levels, WIDTH * WIDTH, table);

  bytes = g_byte_array_free_to_bytes(pixels);
  free(levels);

  texture = gdk_memory_texture_new(WIDTH, WIDTH, GDK_MEMORY_R8G8B8, bytes,
                                   WIDTH * BYTES_PER_R8G8B8);
  gtk_picture_set_paintable(picture, GDK_PAINTABLE(texture));
}
 
static void
close_window (void)
{
  if (surface)
    cairo_surface_destroy (surface);
}


static void activate(GtkApplication *app, gpointer *userdata __attribute__((unused))) {
  GtkWidget *window;
  GtkWidget *picture, *overlay, *drawing_area;
  GtkGesture *drag;

  window = gtk_application_window_new(app);
  g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);
  gtk_window_set_title(GTK_WINDOW(window), "Mandelbrot Set");
  gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, WIDTH);
  picture = gtk_picture_new();
  overlay = gtk_overlay_new();
  drawing_area = gtk_drawing_area_new();
  gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (drawing_area), draw_cb, NULL, NULL);
  g_signal_connect_after (drawing_area, "resize", G_CALLBACK (resize_cb), NULL);
  gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (drawing_area), draw_cb, NULL, NULL);
  add_pixel_picture(GTK_PICTURE(picture));
  gtk_overlay_set_child(GTK_OVERLAY(overlay), picture);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), drawing_area);
  drag = gtk_gesture_drag_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (drag), GDK_BUTTON_PRIMARY);
  gtk_widget_add_controller (drawing_area, GTK_EVENT_CONTROLLER (drag));
  g_signal_connect (drag, "drag-begin", G_CALLBACK (drag_begin), drawing_area);
  g_signal_connect (drag, "drag-update", G_CALLBACK (drag_update), drawing_area);
  g_signal_connect (drag, "drag-end", G_CALLBACK (drag_end), drawing_area);
  gtk_window_set_child(GTK_WINDOW(window), overlay);
  gtk_widget_show(window);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.example.mandelbrot",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
