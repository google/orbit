// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlUtils.h"

#include "OpenGl.h"
#include "OrbitBase/Logging.h"
#include "absl/strings/str_format.h"
#include "freetype-gl/freetype-gl.h"

static const char kErrorInvalidEnum[] =
    "An unacceptable value is specified for an enumerated argument";
static const char kErrorInvalidValue[] = "A numeric argument is out of range";
static const char kErrorInvalidOperation[] =
    "The specified operation is not allowed in the current state";
static const char kErrorInvalidFramebufferOperation[] = "The framebuffer object is not complete";
static const char kErrorOutOfMemory[] = "There is not enough memory left to execute the command";
static const char kErrorStackUnderflow[] =
    "An attempt has been made to perform an operation that would cause an internal stack to "
    "underflow";
static const char kErrorStackOverflow[] =
    "An attempt has been made to perform an operation that would cause an internal stack to "
    "overflow";
static const char kUnknownError[] = "Unknown error";

static const char* GetGlError(GLenum error_code) {
  switch (error_code) {
    case GL_INVALID_ENUM:
      return kErrorInvalidEnum;
    case GL_INVALID_VALUE:
      return kErrorInvalidValue;
    case GL_INVALID_OPERATION:
      return kErrorInvalidOperation;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return kErrorInvalidFramebufferOperation;
    case GL_OUT_OF_MEMORY:
      return kErrorOutOfMemory;
    case GL_STACK_UNDERFLOW:
      return kErrorStackUnderflow;
    case GL_STACK_OVERFLOW:
      return kErrorStackOverflow;
    default:
      return kUnknownError;
  }
}

void CheckGlError() {
  GLenum error_code = glGetError();
  if (error_code != GL_NO_ERROR) {
    const char* error_string = GetGlError(error_code);
    LOG("OpenGL ERROR : %s", error_string);
  }
}
