//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

// clang-format off
#ifdef WIN32
// windows.h must be included BEFORE GL/glew.h
// otherwise wglew.h undefs WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
// clang-format on

#include <GL/glew.h>
#include <GL/glu.h>
#include <freetype-gl/freetype-gl.h>

// clang-format off
#include <GL/gl.h>
// clang-format on
