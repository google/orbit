//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#include "Capture.h"
#include "CoreApp.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Path.h"
#include "UnwindStackUtils.h"

#include "unwindstack/MachineX86_64.h"

using namespace unwindstack;

void UnwindStackUtils::ProcessStackFrame(int frameIndex, Unwinder& unwinder, CallStack& CS)
{
    const FrameData& frame = unwinder.frames()[frameIndex];
    std::wstring moduleName = ToLower(Path::GetFileName(s2ws(frame.map_name)));
    std::shared_ptr<Module> moduleFromName = Capture::GTargetProcess->GetModuleFromName( ws2s(moduleName) );
    uint64_t address = frame.pc;
    if( moduleFromName )
    {
        uint64_t new_address = moduleFromName->ValidateAddress(address);
        address = new_address;
    }
    CS.m_Data.push_back(address);
    if( Capture::GTargetProcess && !Capture::GTargetProcess->HasSymbol(address))
    {
        std::stringstream ss;
        ss << LinuxUtils::Demangle(frame.function_name.c_str()) << "+0x" << std::hex << frame.function_offset;
        GCoreApp->AddSymbol(address, frame.map_name, ss.str());
    }
}

RegsX86_64 UnwindStackUtils::LoadRegisters(const LinuxPerfUtils::perf_sample_regs_user& regs_content)
{
    RegsX86_64 regs;  
    regs[X86_64_REG_RAX] = regs_content.AX;
    regs[X86_64_REG_RBX] = regs_content.BX;
    regs[X86_64_REG_RCX] = regs_content.CX;
    regs[X86_64_REG_RDX] = regs_content.DX;
    regs[X86_64_REG_R8] =  regs_content.R8;
    regs[X86_64_REG_R9] =  regs_content.R9;
    regs[X86_64_REG_R10] = regs_content.R10;
    regs[X86_64_REG_R11] = regs_content.R11;
    regs[X86_64_REG_R12] = regs_content.R12;
    regs[X86_64_REG_R13] = regs_content.R13;
    regs[X86_64_REG_R14] = regs_content.R14;
    regs[X86_64_REG_R15] = regs_content.R15; 
    regs[X86_64_REG_RDI] = regs_content.DI;
    regs[X86_64_REG_RSI] = regs_content.SI;
    regs[X86_64_REG_RBP] = regs_content.BP;
    regs[X86_64_REG_RSP] = regs_content.SP;
    regs[X86_64_REG_RIP] = regs_content.IP;

    return regs;
}