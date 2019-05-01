// Copyright 2012 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Common/GL/EGLWaylandWindow.h"
#include "Common/GL/GLContext.h"


void
EGLWaylandWindow::create_surface(struct wl_surface *parent)
{
  surface = wl_compositor_create_surface(compositor);

  subsurface = wl_subcompositor_get_subsurface(subcompositor, surface, parent);
  wl_subsurface_set_desync(subsurface);
  wl_subsurface_set_position(subsurface, 3, 25);

  native = wl_egl_window_create(surface, m_width, m_height);
}

static void
pointer_handle_enter(void *data, struct wl_pointer *pointer,
		     uint32_t serial, struct wl_surface *surface,
		     wl_fixed_t sx, wl_fixed_t sy)
{
  EGLWaylandWindow *window = (EGLWaylandWindow *) data;
  struct wl_buffer *buffer;
  struct wl_cursor *cursor = window->default_cursor;
  struct wl_cursor_image *image;

  if (window->fullscreen)
    wl_pointer_set_cursor(pointer, serial, NULL, 0, 0);
  else if (cursor) {
    image = window->default_cursor->images[0];
    buffer = wl_cursor_image_get_buffer(image);
    if (!buffer)
      return;
    wl_pointer_set_cursor(pointer, serial,
      window->cursor_surface,
      image->hotspot_x,
      image->hotspot_y);
    wl_surface_attach(window->cursor_surface, buffer, 0, 0);
    wl_surface_damage(window->cursor_surface, 0, 0,
      image->width, image->height);
    wl_surface_commit(window->cursor_surface);
  }
}

static void
pointer_handle_leave(void *data, struct wl_pointer *pointer,
		     uint32_t serial, struct wl_surface *surface)
{
}

static void
pointer_handle_motion(void *data, struct wl_pointer *pointer,
		      uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
}

static void
pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
		      uint32_t serial, uint32_t time, uint32_t button,
		      uint32_t state)
{
  EGLWaylandWindow *window = (EGLWaylandWindow *) data;

  if (!window->xdg_toplevel)
    return;

  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED)
    xdg_toplevel_move(window->xdg_toplevel, window->seat, serial);
}

static void
pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		    uint32_t time, uint32_t axis, wl_fixed_t value)
{
}

static const struct wl_pointer_listener pointer_listener = {
  pointer_handle_enter,
  pointer_handle_leave,
  pointer_handle_motion,
  pointer_handle_button,
  pointer_handle_axis,
};

static void
touch_handle_down(void *data, struct wl_touch *wl_touch,
		  uint32_t serial, uint32_t time, struct wl_surface *surface,
		  int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
  EGLWaylandWindow *window = (EGLWaylandWindow *) data;

  if (!window->wm_base)
    return;

  xdg_toplevel_move(window->xdg_toplevel, window->seat, serial);
}

static void
touch_handle_up(void *data, struct wl_touch *wl_touch,
		uint32_t serial, uint32_t time, int32_t id)
{
}

static void
touch_handle_motion(void *data, struct wl_touch *wl_touch,
		    uint32_t time, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
}

static void
touch_handle_frame(void *data, struct wl_touch *wl_touch)
{
}

static void
touch_handle_cancel(void *data, struct wl_touch *wl_touch)
{
}

static const struct wl_touch_listener touch_listener = {
  touch_handle_down,
  touch_handle_up,
  touch_handle_motion,
  touch_handle_frame,
  touch_handle_cancel,
};

static void
keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
		       uint32_t format, int fd, uint32_t size)
{
}

static void
keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
		      uint32_t serial, struct wl_surface *surface,
		      struct wl_array *keys)
{
}

static void
keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
		      uint32_t serial, struct wl_surface *surface)
{
}

static void
keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
		    uint32_t serial, uint32_t time, uint32_t key,
		    uint32_t state)
{
  EGLWaylandWindow *window = (EGLWaylandWindow *) data;

  if (!window->wm_base)
    return;

  if (key == KEY_F11 && state) {
    if (window->fullscreen)
      xdg_toplevel_unset_fullscreen(window->xdg_toplevel);
    else
      xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);
  } else if (key == KEY_ESC && state)
    window->running = 0;
}

static void
keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
			  uint32_t serial, uint32_t mods_depressed,
			  uint32_t mods_latched, uint32_t mods_locked,
			  uint32_t group)
{
}

static const struct wl_keyboard_listener keyboard_listener = {
  keyboard_handle_keymap,
  keyboard_handle_enter,
  keyboard_handle_leave,
  keyboard_handle_key,
  keyboard_handle_modifiers,
};

static void
seat_handle_capabilities(void *data, struct wl_seat *seat,
			 uint32_t caps)
{
  EGLWaylandWindow *window = (EGLWaylandWindow *) data;

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !window->pointer) {
    window->pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(window->pointer, &pointer_listener, window);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && window->pointer) {
    wl_pointer_destroy(window->pointer);
    window->pointer = NULL;
  }
  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !window->keyboard) {
    window->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(window->keyboard, &keyboard_listener, window);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && window->keyboard) {
    wl_keyboard_destroy(window->keyboard);
    window->keyboard = NULL;
  }
  if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !window->touch) {
    window->touch = wl_seat_get_touch(seat);
    wl_touch_set_user_data(window->touch, window);
    wl_touch_add_listener(window->touch, &touch_listener, window);
  } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && window->touch) {
    wl_touch_destroy(window->touch);
    window->touch = NULL;
  }
}

static const struct wl_seat_listener seat_listener = {
  seat_handle_capabilities,
};

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
  xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
  xdg_wm_base_ping,
};

static void
registry_handle_global(void *data, struct wl_registry *registry,
		       uint32_t id, const char *interface, uint32_t version)
{
  EGLWaylandWindow *window = (EGLWaylandWindow *) data;

  if (strcmp(interface, "wl_compositor") == 0) {
    window->compositor = (wl_compositor *)
      wl_registry_bind(registry, id,
        &wl_compositor_interface,
        MIN(version, 4));
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    window->wm_base = (xdg_wm_base *) wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(window->wm_base, &wm_base_listener, window);
  } else if (strcmp(interface, "wl_seat") == 0) {
    window->seat = (wl_seat *) wl_registry_bind(registry, id, &wl_seat_interface, 1);
      wl_seat_add_listener(window->seat, &seat_listener, window);
  } else if (strcmp(interface, "wl_shm") == 0) {
    window->shm = (wl_shm *) wl_registry_bind(registry, id, &wl_shm_interface, 1);
    window->cursor_theme = wl_cursor_theme_load(NULL, 32, window->shm);
    if (!window->cursor_theme) {
      fprintf(stderr, "unable to load default theme\n");
      return;
    }
    window->default_cursor =
      wl_cursor_theme_get_cursor(window->cursor_theme, "left_ptr");
    if (!window->default_cursor) {
      fprintf(stderr, "unable to load default left pointer\n");
    }
  } else if (strcmp(interface, "wl_subcompositor") == 0) {
    window->subcompositor = (wl_subcompositor *) wl_registry_bind(registry, id,
      &wl_subcompositor_interface, 1);
  }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
			      uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove
};

EGLWaylandWindow::EGLWaylandWindow(const WindowSystemInfo& wsi, struct wl_display* _display, struct wl_egl_window* parent_window, int width, int height)
    : m_width(width), m_height(height), display(_display), m_wsi(wsi)
{
}

EGLWaylandWindow::~EGLWaylandWindow()
{
}

void EGLWaylandWindow::UpdateDimensions()
{
  //here we need to determine the inner size of the qwidget render window
  //resize(width, height)
  //m_width = width;
  //m_height = height;
}

std::unique_ptr<EGLWaylandWindow> EGLWaylandWindow::Create(const WindowSystemInfo& wsi, struct wl_display *_display, struct wl_egl_window* parent_window)
{
  int width = 640;
  int height = 480;

  std::unique_ptr<EGLWaylandWindow> window = std::make_unique<EGLWaylandWindow>(wsi, _display, parent_window, width, height);

  window->window_size.width = width;
  window->window_size.height = height;

  struct wl_registry *registry = wl_display_get_registry(_display);
  wl_registry_add_listener(registry, &registry_listener, (void *) window.get());
  wl_display_roundtrip(_display);
  window->create_surface((struct wl_surface *) wsi.render_surface);
  window->cursor_surface = wl_compositor_create_surface(window->compositor);

  return window;
}
