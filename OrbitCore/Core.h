//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <codecvt>
#include <tchar.h>
#include <wchar.h>

#include "external/xxHash-r42/xxhash.h"
#include "external/websocketpp/websocketpp/sha1/sha1.hpp"
#include "external/websocketpp/websocketpp/base64/base64.hpp"

#include "Threading.h"
#include "CoreMath.h"
#include "Utils.h"
#include "VariableTracing.h"
#include "Path.h"
#include "Utils.h"
#include "TimerManager.h"

#include <concurrentqueue.h>

using namespace std;

typedef std::basic_string<TCHAR> TString;
typedef ULONG64                  ptr_type;

