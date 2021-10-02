#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gdk/gdkx.h>

#include "swcursor-window.h"

#define SECOND 1000000
typedef struct {
  cairo_surface_t* image;
  guint64 timestamp;
  guint64 framerate;
} State_t;

static cairo_surface_t *load_image(const char *path);
static void show_main_window(State_t *state);
static gboolean tick(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer user_data);

static State_t* state;

static void
cleanup() {
  free(state);
  return;
}

int main(int argc, char **argv)
{
  int opt;
	cairo_surface_t *image;

  signal(SIGABRT, cleanup);
  state = (State_t*)malloc(sizeof(State_t));

  state->framerate = SECOND/60;
  state->timestamp = 0;

  while ((opt = getopt(argc, argv, "irh")) != -1) {
      switch (opt) {
      case 'i':
        if (optind < argc) {
          char *file = argv[optind];
		      image = load_image(file);
        }
        break;
      case 'r':
      {
        int rate = atoi(argv[optind]);
        if(rate == 0 || rate > SECOND) {
          fprintf(stdout, "Could not parse: %s, setting default rate to 60", argv[optind]);
        } else {
          state->framerate = SECOND/rate;
        }
        break;
      }
      case 'h':
          fprintf(stdout, "Usage: %s [-ir] [file...]\n\n-i [file.png]\n load the png file as mouse cursor\n\n-r [number]\nset the refresh rate per seconds\n\nExample:\n./swcursor -i cursors/cursor-large.png -r 120", argv[0]);
          exit(EXIT_SUCCESS);
      default:
          fprintf(stderr, "Type -h to see help\n", argv[0]);
          exit(EXIT_FAILURE);
      }
  }
  if(image != NULL) {
    state->image = image;
  } else {
	  state->image = load_image("cursors/8.png");
  }
  fprintf(stdout, "Refreshing cursor with %d fps", state->framerate);
	gtk_init (&argc, &argv);

	show_main_window(state);

	gtk_main();

	return 0;
}

static cairo_surface_t*
load_image(const char *path)
{
	cairo_surface_t *image = cairo_image_surface_create_from_png(path);
	if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) {
		const char *msg = cairo_status_to_string(cairo_surface_status(image));
		fprintf(stderr, "Error loading '%s': %s\n", path, msg);
    return NULL;
	}

	return image;
}

static void show_main_window(State_t* state)
{
	SWCursorWindow *window;

	window = swcursor_window_new();
	swcursor_window_set_image(window, state->image);

	gtk_widget_add_tick_callback(GTK_WIDGET (window), tick, state, free);
	gtk_widget_show_all(GTK_WIDGET (window));
}

static gboolean
tick(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer user_data)
{
	static gboolean show_warning = TRUE;

	GdkWindow *gdk_window;
	Display *xdisplay;
	Window xroot_window;
	Window ret_root;
	Window ret_child;
	int root_x, root_y;
	int win_x, win_y;
	int move_x, move_y;
	unsigned int mask;
	gint scale_factor;
	gboolean mouse_down;
  int w_width, w_height;

  State_t *state = (State_t*)user_data;
  guint64 timestamp = gdk_frame_clock_get_frame_time(frame_clock);

  if((timestamp-state->timestamp) >= state->framerate) {
    state->timestamp=gdk_frame_clock_get_frame_time(frame_clock);

    gtk_window_get_size(GTK_WINDOW (widget), &w_width, &w_height);
    gdk_window = gtk_widget_get_window(widget);
    xdisplay = GDK_SCREEN_XDISPLAY(gdk_window_get_screen(gdk_window));
    xroot_window = XDefaultRootWindow(xdisplay);

    scale_factor = gdk_window_get_scale_factor(gdk_window);

    if(XQueryPointer(xdisplay, xroot_window,
                     &ret_root, &ret_child,
                     &root_x, &root_y,
                     &win_x, &win_y, &mask))
    {
      move_x = root_x / scale_factor - w_height / 2;
      move_y = root_y / scale_factor - w_width / 2;

      gtk_window_move(GTK_WINDOW (widget), move_x, move_y);

      mouse_down = (mask & Button1Mask) ||
                   (mask & Button2Mask) ||
                   (mask & Button3Mask);

      swcursor_window_set_mouse_down(SWCURSOR_WINDOW (widget), mouse_down);
    } else {
      if (show_warning) {
        fprintf(stderr, "swcursor: warning: could not get cursor position from Xserver (further warnings will be suppressed)");
        show_warning = FALSE;
      }
    }
  }

	return G_SOURCE_CONTINUE;
}
