#include <gtk/gtk.h>

#include "rubberband.h"

#ifdef DEBUG
#define log_area(MSG, RB)                                                  \
  (fprintf (stderr, "%s: %d, %d, %d, %d\n", MSG, RB->start_x, RB->start_y, \
            RB->end_x, RB->end_y))
#else
#define log_area(MSG, RB)
#endif

struct _MbtRubberBand
{
  GtkWidget parent;
  GtkGesture *drag_gesture;
  GtkGesture *click_gesture;

  bool selecting;
  int start_x;
  int start_y;
  int end_x;
  int end_y;
};

static guint mbt_select_signal;

G_DEFINE_TYPE (MbtRubberBand, mbt_rubber_band, GTK_TYPE_WIDGET)

static void
mbt_rb_snapshot (GtkWidget *widget, GtkSnapshot *snapshot)
{
  MbtRubberBand *band = MBT_RUBBER_BAND (widget);
  cairo_t *cr;
  int width, height;

  if (!band->selecting)
    return;

  width = gtk_widget_get_width (widget);
  height = gtk_widget_get_height (widget);

  cr = gtk_snapshot_append_cairo (snapshot,
                                  &GRAPHENE_RECT_INIT (0, 0, width, height));

  cairo_rectangle (cr, band->start_x, band->start_y, band->end_x, band->end_y);

  cairo_stroke (cr);
  cairo_destroy (cr);
}

static void
mbt_rb_click_pressed (GtkGestureClick *gesture __attribute__ ((unused)),
                      int n_count __attribute__ ((unused)),
                      double widget_x,
                      double widget_y,
                      gpointer *data __attribute__ ((unused)))
{
  MbtRubberBand *self = MBT_RUBBER_BAND (gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture)));
  self->start_x = widget_x;
  self->start_y = widget_y;
  self->end_x = widget_x;
  self->end_y = widget_y;
  log_area ("click", self);
}

static void
mbt_rb_drag_begin (GtkGestureDrag *gesture __attribute__ ((unused)),
                   double widget_x __attribute__ ((unused)),
                   double widget_y __attribute__ ((unused)),
                   MbtRubberBand *self)
{
  self->selecting = true;
}

static void
mbt_rb_drag_update (GtkGestureDrag *gesture __attribute__ ((unused)),
                    double widget_x,
                    double widget_y,
                    MbtRubberBand *self)
{
  self->end_x = widget_x;
  self->end_y = widget_y;
  gtk_widget_queue_draw (GTK_WIDGET (self));
  log_area ("update", self);
}

static void
mbt_rb_drag_end (GtkGestureDrag *gesture __attribute__ ((unused)),
                 double widget_x __attribute__ ((unused)),
                 double widget_y __attribute__ ((unused)),
                 MbtRubberBand *self)
{
  self->selecting = false;
  g_signal_emit (self, mbt_select_signal, 0);
  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
mbt_rubber_band_class_init (MbtRubberBandClass *class)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  widget_class->snapshot = mbt_rb_snapshot;
  mbt_select_signal = g_signal_new (
      "selection-complete", G_TYPE_FROM_CLASS (G_OBJECT_CLASS (class)),
      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 0);
}

static void
mbt_rubber_band_init (MbtRubberBand *band)
{
  band->selecting = false;
  band->drag_gesture = gtk_gesture_drag_new ();
  band->click_gesture = gtk_gesture_click_new ();
  g_signal_connect (band->click_gesture, "pressed",
                    G_CALLBACK (mbt_rb_click_pressed), band);
  g_signal_connect (band->drag_gesture, "drag-begin",
                    G_CALLBACK (mbt_rb_drag_begin), band);
  g_signal_connect (band->drag_gesture, "drag-update",
                    G_CALLBACK (mbt_rb_drag_update), band);
  g_signal_connect (band->drag_gesture, "drag-end", G_CALLBACK (mbt_rb_drag_end),
                    band);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (band->drag_gesture),
                                    TRUE);
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (band->click_gesture), 0);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (band->click_gesture),
                                    TRUE);
  gtk_widget_add_controller (GTK_WIDGET (band),
                             GTK_EVENT_CONTROLLER (band->drag_gesture));
  gtk_widget_add_controller (GTK_WIDGET (band),
                             GTK_EVENT_CONTROLLER (band->click_gesture));

  log_area ("init", band);
}

GtkWidget *
mbt_rubber_band_new (void)
{
  return GTK_WIDGET (g_object_new (MBT_TYPE_RUBBER_BAND, NULL));
}

int
mbt_rubber_band_start_x (MbtRubberBand *rb)
{
  return rb->start_x;
}

int
mbt_rubber_band_start_y (MbtRubberBand *rb)
{
  return rb->start_y;
}

int
mbt_rubber_band_width (MbtRubberBand *rb)
{
  return rb->end_x;
}

int
mbt_rubber_band_height (MbtRubberBand *rb)
{
  return rb->end_y;
}
