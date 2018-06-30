#pragma once

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#undef min
#undef max
#endif
