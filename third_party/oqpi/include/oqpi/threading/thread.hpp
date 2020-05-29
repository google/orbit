#pragma once

#include "oqpi/platform.hpp"

// Thread interface
#include "oqpi/threading/interface_thread.hpp"
// Platform specific implementations
#if OQPI_PLATFORM_WIN
#	include "oqpi/threading/win_thread.hpp"
#else
#	error No thread implementation defined for the current platform
#endif

namespace oqpi {

    template<template<typename> typename _Layer = empty_layer>
    using thread_interface = itfc::thread<thread_impl, _Layer>;

#ifdef OQPI_USE_DEFAULT
    using thread = thread_interface<>;
#endif

}