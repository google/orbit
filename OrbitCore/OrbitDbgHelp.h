//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
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
