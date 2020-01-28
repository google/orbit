//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
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
