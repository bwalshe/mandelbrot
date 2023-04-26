#ifndef __MBR_RUBBER_BAND_H__
#define __MBR_RUBBER_BAND_H__

#include <gtk/gtk.h>

#define MBT_TYPE_RUBBER_BAND (mbt_rubber_band_get_type())
G_DECLARE_FINAL_TYPE (MbtRubberBand, mbt_rubber_band, MBT, RUBBER_BAND, GtkWidget)


GtkWidget *
mbt_rubber_band_new (void);

int mbt_rubber_band_start_x(MbtRubberBand *rb);
int mbt_rubber_band_start_y(MbtRubberBand *rb);
int mbt_rubber_band_end_x(MbtRubberBand *rb);
int mbt_rubber_band_end_y(MbtRubberBand *rb);

#endif /* __MBR_RUBBER_BAND_H__ */
