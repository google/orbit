/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#ifdef _WIN32

#include <windows.h>

#ifdef UNICODE
#define DBGHELP_TRANSLATE_TCHAR
#endif

#pragma warning(push)
#pragma warning(disable : 4091)
#include <DbgHelp.h>
#pragma warning(pop)

#endif
