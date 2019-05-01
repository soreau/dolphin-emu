// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

//#include <X11/Xlib.h>

#include "Common/GL/GLInterface/EGL.h"
#include "Common/GL/EGLWaylandWindow.h"

class GLContextEGLWayland final : public GLContextEGL
{
public:
  GLContextEGLWayland(const WindowSystemInfo& wsi);
  ~GLContextEGLWayland() override;

  void Update() override;

  const WindowSystemInfo& m_wsi;

protected:
  EGLDisplay OpenEGLDisplay() override;
  EGLNativeWindowType GetEGLNativeWindow(EGLConfig config) override;

  std::unique_ptr<EGLWaylandWindow> m_render_window;
};
