/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>

#include <chrono>
#include <codecvt>
#include <ctime>
#include <iostream>
#include <sstream>

#include "Platform.h"
#ifdef _WIN32
#include <tchar.h>
#endif

#include "Path.h"
#include "Threading.h"
#include "Utils.h"
#include "VariableTracing.h"
