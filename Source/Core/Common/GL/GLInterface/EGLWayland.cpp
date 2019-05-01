// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#define WL_EGL_PLATFORM 1
#include "Common/GL/GLInterface/EGLWayland.h"

GLContextEGLWayland::GLContextEGLWayland(const WindowSystemInfo& wsi)
  : m_wsi(wsi)
{
}

GLContextEGLWayland::~GLContextEGLWayland()
{
  // The context must be destroyed before the window.
  DestroyWindowSurface();
  DestroyContext();
  m_render_window.reset();
}

void GLContextEGLWayland::Update()
{
  m_render_window->UpdateDimensions();
  m_backbuffer_width = m_render_window->GetWidth();
  m_backbuffer_height = m_render_window->GetHeight();
}

EGLDisplay GLContextEGLWayland::OpenEGLDisplay()
{
  return eglGetDisplay(static_cast<struct wl_display *>(m_host_display));
}

EGLNativeWindowType GLContextEGLWayland::GetEGLNativeWindow(EGLConfig config)
{
  if (m_render_window)
    m_render_window.reset();

  m_render_window = EGLWaylandWindow::Create(m_wsi,
    static_cast<struct wl_display *>(m_host_display),
    reinterpret_cast<struct wl_egl_window*>(m_host_window));
  m_backbuffer_width = m_render_window->GetWidth();
  m_backbuffer_height = m_render_window->GetHeight();

  return reinterpret_cast<EGLNativeWindowType>(m_render_window->GetWindow());
}
