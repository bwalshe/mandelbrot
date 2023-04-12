#include <complex.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>

#define WIDTH 800
#define BYTES_PER_R8G8B8 3

static int *mandelbrot_set(int width, int height) {
  float complex c, z;
  int *mandelbrot;
  clock_t start, end;

  start = clock();

  mandelbrot = malloc(width * height * sizeof(int));
  memset(mandelbrot, 0, width * height * sizeof(int));

  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < height; ++j) {
      z = 0;
      c = (4.0 * i) / width - 2.0 + I * ((4.0 * j) / height - 2.0);
      for (int k = 0; k < 100; ++k) {
        z = z * z + c;
        if (cabsf(z) > 4.0) {
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

static GByteArray *rgb_from_index(int *values, size_t length, guint8 *table) {
  int i;
  GByteArray *rgbs;
  rgbs = g_byte_array_sized_new(length * BYTES_PER_R8G8B8);
  for (i = 0; i < length; ++i) {
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

  levels = mandelbrot_set(WIDTH, WIDTH);
  pixels = rgb_from_index(levels, WIDTH * WIDTH, table);

  bytes = g_byte_array_free_to_bytes(pixels);
  free(levels);

  texture = gdk_memory_texture_new(WIDTH, WIDTH, GDK_MEMORY_R8G8B8, bytes,
                                   WIDTH * BYTES_PER_R8G8B8);
  gtk_picture_set_paintable(picture, GDK_PAINTABLE(texture));
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *picture;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Mandelbrot Set");
  gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, WIDTH);
  picture = gtk_picture_new();
  add_pixel_picture(GTK_PICTURE(picture));
  gtk_window_set_child(GTK_WINDOW(window), picture);
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
