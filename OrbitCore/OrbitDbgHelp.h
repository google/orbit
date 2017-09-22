//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#ifdef UNICODE
#define DBGHELP_TRANSLATE_TCHAR
#endif

#pragma warning( push )
#pragma warning( disable : 4091 )
#include <DbgHelp.h>
#pragma warning( pop )

