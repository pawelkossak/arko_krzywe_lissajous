#include <gtk/gtk.h>
#include <stdlib.h>

extern void generateLissajousCurves(int a, int b, double beta, char *outputImg);
typedef struct {
    GtkAdjustment *adj1;
    GtkAdjustment *adj2;
    GtkAdjustment *adj3;
    GtkWidget *main_image;
    GtkWidget *scrolled_window;
    GdkPixbuf *original_pixbuf;
    double zoom_level;

    gboolean is_dragging;
    double start_x;
    double start_y;
    double start_h_adj;
    double start_v_adj;
} AppWidgets;

void update_image_zoom(AppWidgets *widgets) {
    if (!widgets->original_pixbuf) return;

    int width = gdk_pixbuf_get_width(widgets->original_pixbuf);
    int height = gdk_pixbuf_get_height(widgets->original_pixbuf);
    int new_width = (int)(width * widgets->zoom_level);
    int new_height = (int)(height * widgets->zoom_level);

    if (new_width < 1) new_width = 1;
    if (new_height < 1) new_height = 1;

    GdkPixbuf *scaled = gdk_pixbuf_scale_simple(widgets->original_pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(widgets->main_image), scaled);
    g_object_unref(scaled);
}

gboolean on_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    double delta_y;
    if (!gdk_event_get_scroll_deltas((GdkEvent *)event, NULL, &delta_y)) {
        if (event->direction == GDK_SCROLL_UP) delta_y = -1.0;
        else if (event->direction == GDK_SCROLL_DOWN) delta_y = 1.0;
        else return FALSE;
    }

    if (delta_y < 0) {
        widgets->zoom_level *= 1.1;
    } else if (delta_y > 0) {
        widgets->zoom_level /= 1.1;
    }

    if (widgets->zoom_level < 0.1) widgets->zoom_level = 0.1;
    if (widgets->zoom_level > 10.0) widgets->zoom_level = 10.0;

    update_image_zoom(widgets);

    return TRUE;
}

void on_submit_button_clicked(GtkButton *button, AppWidgets *widgets) {
    double val1 = gtk_adjustment_get_value(widgets->adj1);
    double val2 = gtk_adjustment_get_value(widgets->adj2);
    double val3 = gtk_adjustment_get_value(widgets->adj3);

    // g_print("--- New parameters have been loaded. ---\n");
    // g_print("Parameter 1: %.2f\n", val1);
    // g_print("Parameter 2: %.2f\n", val2);
    // g_print("Parameter 3: %.2f\n", val3);
    
    generateLissajousCurves((int)val1, (int)val2, val3, "output.bmp");

    if (widgets->original_pixbuf) {
        g_object_unref(widgets->original_pixbuf);
    }
    
    widgets->original_pixbuf = gdk_pixbuf_new_from_file("output.bmp", NULL);
    widgets->zoom_level = 1.0;
    
    update_image_zoom(widgets);
}

gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    if (event->button == 1) {
        widgets->is_dragging = TRUE;
        widgets->start_x = event->x_root;
        widgets->start_y = event->y_root;

        GtkAdjustment *hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(widgets->scrolled_window));
        GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widgets->scrolled_window));
        widgets->start_h_adj = gtk_adjustment_get_value(hadj);
        widgets->start_v_adj = gtk_adjustment_get_value(vadj);

        GdkWindow *window = gtk_widget_get_window(widget);
        GdkDisplay *display = gdk_window_get_display(window);
        GdkCursor *cursor = gdk_cursor_new_from_name(display, "grabbing");
        gdk_window_set_cursor(window, cursor);
        g_object_unref(cursor);

        return TRUE;
    }
    return FALSE;
}

gboolean on_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    if (widgets->is_dragging) {
        double dx = event->x_root - widgets->start_x;
        double dy = event->y_root - widgets->start_y;

        GtkAdjustment *hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(widgets->scrolled_window));
        GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widgets->scrolled_window));
        
        gtk_adjustment_set_value(hadj, widgets->start_h_adj - dx);
        gtk_adjustment_set_value(vadj, widgets->start_v_adj - dy);

        return TRUE;
    }
    return FALSE;
}

gboolean on_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    if (event->button == 1) {
        widgets->is_dragging = FALSE;

        GdkWindow *window = gtk_widget_get_window(widget);
        gdk_window_set_cursor(window, NULL);

        return TRUE;
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *submit_button;
    AppWidgets *widgets = g_slice_new(AppWidgets);
    
    widgets->original_pixbuf = NULL;
    widgets->zoom_level = 1.0;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("interface.ui");

    if (!builder) {
        g_printerr("Error: Couldn't load interface.ui\n");
        return 1;
    }

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_widget_add_events(window, GDK_SCROLL_MASK);
    g_signal_connect(window, "scroll-event", G_CALLBACK(on_scroll_event), widgets);

    widgets->adj1 = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj1"));
    widgets->adj2 = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj2"));
    widgets->adj3 = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adj3"));
    widgets->main_image = GTK_WIDGET(gtk_builder_get_object(builder, "main_image"));
    widgets->scrolled_window = GTK_WIDGET(gtk_builder_get_object(builder, "scrolled_window"));

    GtkWidget *image_event_box = GTK_WIDGET(gtk_builder_get_object(builder, "image_event_box"));

    gtk_widget_add_events(image_event_box, 
        GDK_BUTTON_PRESS_MASK | 
        GDK_BUTTON_RELEASE_MASK | 
        GDK_POINTER_MOTION_MASK
    );

    g_signal_connect(image_event_box, "button-press-event", G_CALLBACK(on_button_press_event), widgets);
    g_signal_connect(image_event_box, "button-release-event", G_CALLBACK(on_button_release_event), widgets);
    g_signal_connect(image_event_box, "motion-notify-event", G_CALLBACK(on_motion_notify_event), widgets);

    submit_button = GTK_WIDGET(gtk_builder_get_object(builder, "submit_button"));
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_button_clicked), widgets);

    gtk_widget_show_all(window);

    gtk_main();

    if (widgets->original_pixbuf) {
        g_object_unref(widgets->original_pixbuf);
    }

    g_object_unref(builder);
    g_slice_free(AppWidgets, widgets);

    return 0;
}