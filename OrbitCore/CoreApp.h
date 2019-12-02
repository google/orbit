//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "BaseTypes.h"

class Timer;
class LinuxPerfData;
struct CallStack;
struct ContextSwitch;
struct HashedLinuxPerfData;

class CoreApp
{
public:
    virtual void SendToUiAsync( const std::wstring & /*a_Msg*/ ){}
    virtual void SendToUiNow( const std::wstring & /*a_Msg*/ ){}
    virtual bool GetUnrealSupportEnabled(){ return false; }
    virtual bool GetUnitySupportEnabled(){ return false; }
    virtual bool GetUnsafeHookingEnabled(){ return false; }
    virtual bool GetSamplingEnabled(){ return false; }
    virtual bool GetOutputDebugStringEnabled(){ return false; }
    virtual void LogMsg( const std::wstring & /*a_Msg*/ ){}
    virtual void UpdateVariable( class Variable * /*a_Variable*/ ){}
    virtual void Disassemble( const std::string & /*a_FunctionName*/, DWORD64 /*a_VirtualAddress*/, const char * /*a_MachineCode*/, size_t /*a_Size*/ ){}
    virtual void ProcessTimer( Timer* /*a_Timer*/, const std::string& /*a_FunctionName*/ ) {}
    virtual void ProcessSamplingCallStack(LinuxPerfData& /*a_CS*/) {}
    virtual void ProcessHashedSamplingCallStack(HashedLinuxPerfData& /*a_CallStack*/) {}
    virtual void ProcessCallStack( CallStack& /*a_CallStack*/ ){}
    virtual void ProcessContextSwitch( const ContextSwitch& /*a_ContextSwithc*/ ){}
    virtual void AddSymbol(uint64_t /*a_Address*/, const std::string& /*a_Module*/, const std::string& /*a_Name*/){}
    virtual const std::unordered_map<DWORD64, std::shared_ptr<class Rule> >* GetRules(){ return nullptr; }
    virtual void SendRemoteProcess(uint32_t a_PID) {}
    virtual void RefreshCaptureView() {}

    virtual void StartRemoteCaptureBufferingThread(){}
    virtual void StopRemoteCaptureBufferingThread(){}

    std::vector< std::wstring > m_SymbolLocations;
};

extern CoreApp* GCoreApp;
