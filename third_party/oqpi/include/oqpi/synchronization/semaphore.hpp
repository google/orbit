#pragma once

#include "oqpi/platform.hpp"

// Thread interface
#include "oqpi/synchronization/interface/interface_semaphore.hpp"
// Platform specific implementations
#if OQPI_PLATFORM_WIN
#	include "oqpi/synchronization/win/win_semaphore.hpp"
#else
#	error No semaphore implementation defined for the current platform
#endif

namespace oqpi {

    template<template<typename> typename _Layer = empty_layer>
    using semaphore_interface = itfc::semaphore<semaphore_impl, _Layer>;

#ifdef OQPI_USE_DEFAULT
    using semaphore = semaphore_interface<>;
#endif

}