// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include "compiler_check.hpp"
#include "ut.hpp"
#include "kt.hpp"
#include "trace.hpp"

namespace krabs {

    /**
     * <summary>
     * Specialization of the base trace class for user traces.
     * </summary>
     */
    typedef krabs::trace<krabs::details::ut> user_trace;

    /**
     * <summary>
     * Specialization of the base trace class for kernel traces.
     * </summary>
     */
    typedef krabs::trace<krabs::details::kt> kernel_trace;
}
