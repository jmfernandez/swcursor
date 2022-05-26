#include "swcursor-window.h"
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

static cairo_surface_t* load_image(const char* path);
static void show_main_window(cairo_surface_t* image;);
static gboolean tick(GtkWidget* widget, GdkFrameClock* frame_clock, gpointer user_data);

int main(int argc, char** argv) {
  gtk_init(&argc, &argv);
  cairo_surface_t* image;
  const char * cursor_filename = "cursor.png";
  
  if(argc > 1) {
    cursor_filename = argv[1];
  }
  image = load_image(cursor_filename);
  show_main_window(image);
  gtk_main();
  return 0;
}

static cairo_surface_t* load_image(const char* path) {
  cairo_surface_t* image = cairo_image_surface_create_from_png(path);
  if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) {
    const char* msg = cairo_status_to_string(cairo_surface_status(image));
    fprintf(stderr, "swcursor: error loading '%s': %s\n", path, msg);
    exit(1);
  }
  return image;
}

static void show_main_window(cairo_surface_t* image) {
  SWCursorWindow* window;
  window = swcursor_window_new();
  swcursor_window_set_image(window, image);
  gtk_widget_add_tick_callback(GTK_WIDGET(window), tick, NULL, NULL);
  gtk_widget_show_all(GTK_WIDGET(window));
}

static gboolean tick(GtkWidget* widget, GdkFrameClock* frame_clock, gpointer user_data) {
  static gboolean show_warning = TRUE;
  GdkWindow* gdk_window;
  Display* xdisplay;
  Window xroot_window;
  Window ret_root;
  Window ret_child;
  int root_x, root_y;
  int win_x, win_y;
  int move_x, move_y;
  unsigned int mask;
  gint scale_factor;
  gboolean mouse_down;
  int w_width;
  int w_height;
  
  gdk_window = gtk_widget_get_window(widget);
  w_width = gdk_window_get_width(gdk_window);
  w_height = gdk_window_get_height(gdk_window);
  xdisplay = GDK_SCREEN_XDISPLAY(gdk_window_get_screen(gdk_window));
  xroot_window = XDefaultRootWindow(xdisplay);
  scale_factor = gdk_window_get_scale_factor(gdk_window);
  if (XQueryPointer(xdisplay, xroot_window, &ret_root, &ret_child, &root_x, &root_y, &win_x, &win_y, &mask)) {
    move_x = root_x / scale_factor - w_width / 2;
    move_y = root_y / scale_factor - w_height / 2;
    gtk_window_move(GTK_WINDOW(widget), move_x, move_y);
    mouse_down = (mask & Button1Mask) || (mask & Button2Mask) || (mask & Button3Mask);
    swcursor_window_set_mouse_down(SWCURSOR_WINDOW(widget), mouse_down);
  } else {
    if (show_warning) {
      fprintf(stderr,
              "swcursor: warning: could not get cursor position from Xserver (further warnings will be suppressed)");
      show_warning = FALSE;
    }
  }
  return G_SOURCE_CONTINUE;
}
