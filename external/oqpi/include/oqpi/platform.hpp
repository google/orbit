#pragma once

// Define every supported platform to 0 so it can be used in #if statements
#define OQPI_PLATFORM_WIN	(0)
#define OQPI_PLATFORM_POSIX	(0)

// Define only the current platform to 1
#if defined(_WIN32)
#	undef  OQPI_PLATFORM_WIN
#	define OQPI_PLATFORM_WIN	(1)
#   ifndef WIN32_LEAN_AND_MEAN
#     error oqpi requires WIN32_LEAN_AND_MEAN
#   endif
#   ifndef VC_EXTRALEAN
#     error oqpi requires VC_EXTRALEAN
#   endif

#   include <windows.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#	undef  OQPI_PLATFORM_POSIX
#	define OQPI_PLATFORM_POSIX	(1)
#endif
