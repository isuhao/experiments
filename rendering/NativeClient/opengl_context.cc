// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "opengl_context.h"

#include <pthread.h>

#include "ppapi/gles2/gl2ext_ppapi.h"

namespace {
// This is called by the brower when the 3D context has been flushed to the
// browser window.
void FlushCallback(void* data, int32_t result) {
  static_cast<tumbler::OpenGLContext*>(data)->set_flush_pending(false);
}
}  // namespace

namespace tumbler {

OpenGLContext::OpenGLContext(pp::Instance* instance)
    : pp::Graphics3DClient(instance),
      flush_pending_(false) {
  pp::Module* module = pp::Module::Get();
  assert(module);
  gles2_interface_ = static_cast<const struct PPB_OpenGLES2*>(
      module->GetBrowserInterface(PPB_OPENGLES2_INTERFACE));
  assert(gles2_interface_);
}

OpenGLContext::~OpenGLContext() {
  glSetCurrentContextPPAPI(0);
}

bool OpenGLContext::MakeContextCurrent(pp::Instance* instance) {
  if (instance == NULL) {
    glSetCurrentContextPPAPI(0);
    return false;
  }
  // Lazily create the Pepper context.
  if (context_.is_null()) {
    int32_t attribs[] = {
        PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
        PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
        PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
        PP_GRAPHICS3DATTRIB_SAMPLES, 0,
        PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
        PP_GRAPHICS3DATTRIB_WIDTH, size_.width(),
        PP_GRAPHICS3DATTRIB_HEIGHT, size_.height(),
        PP_GRAPHICS3DATTRIB_NONE
    };
    context_ = pp::Graphics3D(*instance, pp::Graphics3D(), attribs);
    if (context_.is_null()) {
      glSetCurrentContextPPAPI(0);
      return false;
    }
    instance->BindGraphics(context_);
  }
  glSetCurrentContextPPAPI(context_.pp_resource());
  return true;
}

void OpenGLContext::InvalidateContext(pp::Instance* instance) {
  glSetCurrentContextPPAPI(0);
}

void OpenGLContext::ResizeContext(const pp::Size& size) {
  size_ = size;
  if (!context_.is_null()) {
    context_.ResizeBuffers(size.width(), size.height());
  }
}


void OpenGLContext::FlushContext(PP_CompletionCallback_Func func, void* data) 
{
  if (flush_pending()) {
    // A flush is pending so do nothing; just drop this flush on the floor.
    return;
  }
  set_flush_pending(true);
//  context_.SwapBuffers(pp::CompletionCallback(&FlushCallback, this));
  context_.SwapBuffers(pp::CompletionCallback(func, data));
}
}  // namespace tumbler
