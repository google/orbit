// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ole32.lib")


//
//                              /\
//                             ( /   @ @    ()
//                              \  __| |__  /
//                               -/   "   \-
//                              /-|       |-\
//                             / /-\     /-\ \
//                              / /-`---'-\ \
//                               /         \
//
// Summary
// ----------------------------------------------------------------------------
// Krabs is a wrapper around ETW because ETW is the worst API ever made.

#pragma warning(push)
#pragma warning(disable: 4512) // stupid spurious "can't generate assignment error" warning
#pragma warning(disable: 4634) // DocXml comment warnings in native C++
#pragma warning(disable: 4635) // DocXml comment warnings in native C++

#include "krabs/compiler_check.hpp"
#include "krabs/ut.hpp"
#include "krabs/kt.hpp"
#include "krabs/guid.hpp"
#include "krabs/trace.hpp"
#include "krabs/trace_context.hpp"
#include "krabs/client.hpp"
#include "krabs/errors.hpp"
#include "krabs/schema.hpp"
#include "krabs/schema_locator.hpp"
#include "krabs/parse_types.hpp"
#include "krabs/collection_view.hpp"
#include "krabs/size_provider.hpp"
#include "krabs/parser.hpp"
#include "krabs/property.hpp"
#include "krabs/provider.hpp"
#include "krabs/etw.hpp"
#include "krabs/tdh_helpers.hpp"
#include "krabs/kernel_providers.hpp"

#include "krabs/testing/proxy.hpp"
#include "krabs/testing/filler.hpp"
#include "krabs/testing/synth_record.hpp"
#include "krabs/testing/record_builder.hpp"
#include "krabs/testing/event_filter_proxy.hpp"
#include "krabs/testing/record_property_thunk.hpp"

#include "krabs/filtering/view_adapters.hpp"
#include "krabs/filtering/comparers.hpp"
#include "krabs/filtering/predicates.hpp"
#include "krabs/filtering/event_filter.hpp"

#pragma warning(pop)
