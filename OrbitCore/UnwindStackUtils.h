//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include "Callstack.h"
#include "LinuxPerfUtils.h"

#include "unwindstack/RegsX86_64.h"
#include "unwindstack/Unwinder.h"
//-----------------------------------------------------------------------------
namespace UnwindStackUtils
{
    void ProcessStackFrame(int frameIndex, unwindstack::Unwinder& unwinder, CallStack& CS);
    unwindstack::RegsX86_64 LoadRegisters(const LinuxPerfUtils::perf_sample_regs_user& regs_content);
}