// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include "Common/WindowSystemInfo.h"

#include <linux/input.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "xdg-shell-client-protocol.h"

#include <memory>
#include <thread>
#include <cstring>

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif


struct geometry {
	int width, height;
};

class EGLWaylandWindow
{
public:
  EGLWaylandWindow(const WindowSystemInfo& wsi, struct wl_display* display, struct wl_egl_window* parent_window, int width, int height);
  ~EGLWaylandWindow();

  struct wl_display *GetDisplay() const { return display; }
  struct wl_egl_window *GetParentWindow() const { return native; }
  struct wl_egl_window *GetWindow() const { return native; }
  int GetWidth() const { return m_width; }
  int GetHeight() const { return m_height; }

  void UpdateDimensions();

  static std::unique_ptr<EGLWaylandWindow> Create(const WindowSystemInfo& wsi, struct wl_display *display, struct wl_egl_window *parent_window);

  struct wl_display *m_display;
  struct wl_egl_window *m_window;

  int m_width;
  int m_height;

  /* Display */
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct wl_subcompositor *subcompositor;
  struct xdg_wm_base *wm_base;
  struct wl_seat *seat;
  struct wl_pointer *pointer;
  struct wl_touch *touch = nullptr;
  struct wl_keyboard *keyboard;
  struct wl_shm *shm;
  struct wl_cursor_theme *cursor_theme;
  struct wl_cursor *default_cursor;
  struct wl_surface *cursor_surface;

  PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC swap_buffers_with_damage;

  /* Window */
  void create_surface(struct wl_surface *parent);
  struct geometry geometry, window_size;
  uint32_t benchmark_time, frames;
  struct wl_egl_window *native;
  struct wl_surface *surface;
  struct wl_subsurface *subsurface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_callback *callback;
  int fullscreen, maximized, running = 1;
  bool wait_for_configure = true;

  const WindowSystemInfo& m_wsi;
};
