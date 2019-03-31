//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "Pdb.h"
#include "Callstack.h"
#include "BlockChain.h"
#include "SerializationMacros.h"

class Process;
class Thread;

//-----------------------------------------------------------------------------
struct SampledFunction
{
    SampledFunction() : m_Inclusive(0), m_Exclusive(0), m_Line(0), m_Address(0), m_Hash(0), m_Function(0) {}
    unsigned long long Hash();
    bool GetSelected() const;
    std::wstring m_Name;
    std::wstring m_Module;
    std::wstring m_File;
    float        m_Exclusive;
    float        m_Inclusive;
    int          m_Line;
    DWORD64      m_Address;
    unsigned long long m_Hash;
    Function*   m_Function;

    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct LineInfo
{
    LineInfo() : m_Line(0), m_Address(0) {}
    std::wstring  m_File;
    DWORD         m_Line;
    DWORD64       m_Address;
    DWORD64       m_FileNameHash;

    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct ThreadSampleData
{
    ThreadSampleData() : m_NumSamples(0), m_AverageThreadUsage(0), m_TID(0) { m_ThreadUsage.push_back(0); }
    void ComputeAverageThreadUsage();
    std::multimap< int, CallstackID > SortCallstacks( const std::set< CallstackID > & a_CallStacks, int & o_TotalCallStacks );
    std::unordered_map< CallstackID, unsigned int > m_CallstackCount;
    std::unordered_map< DWORD64, unsigned int >     m_AddressCount;
    std::unordered_map< DWORD64, unsigned int >     m_ExclusiveCount;
    std::multimap< unsigned int, DWORD64 >          m_AddressCountSorted;
    unsigned int                                    m_NumSamples;
    std::vector< SampledFunction >                  m_SampleReport;
    std::vector< float >                            m_ThreadUsage;
    float                                           m_AverageThreadUsage;
    ThreadID                                        m_TID;
    
    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct CallstackCount
{
    CallstackCount() : m_Count(0){}
    int                m_Count;
    CallstackID        m_CallstackId;

    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct SortedCallstackReport
{
    SortedCallstackReport() : m_NumCallStacksTotal(0){}
    int m_NumCallStacksTotal;
    std::vector< CallstackCount > m_CallStacks;

    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
class SamplingProfiler
{
public: 
    SamplingProfiler( const std::shared_ptr<Process> & a_Process, bool a_ETW = false );
    SamplingProfiler();
    ~SamplingProfiler();

    void StartCapture();
    void StopCapture();
    int  GetNumSamples() const { return m_NumSamples; }
    float GetSampleTime();
    float GetSampleTimeTotal() const { return m_SampleTimeSeconds; }
    bool  ShouldStop();
    void FireDoneProcessingCallbacks();
    void AddCallStack( CallStack & a_CallStack ) { if( m_State == Sampling ) m_Callstacks.push_back( a_CallStack ); }
    const std::shared_ptr<CallStack> GetCallStack( CallstackID a_ID ) { return m_UniqueCallstacks[a_ID]; }
    std::multimap<int, CallstackID> GetCallStacksFromAddress( DWORD64 a_Addr, ThreadID a_TID, int & o_NumCallstacks );
    std::shared_ptr< SortedCallstackReport > GetSortedCallstacksFromAddress( DWORD64 a_Addr, ThreadID a_TID );
    
    enum SamplingState { Idle, Sampling, PendingStop, Processing, DoneProcessing };
    SamplingState GetState() const { return m_State; }
    void SetState( SamplingState a_State ){ m_State = a_State; }
    const std::vector< ThreadSampleData* > & GetThreadSampleData() const { return m_SortedThreadSampleData; }
    void SetLoadedFromFile( bool a_Value = true ) { m_LoadedFromFile = a_Value; }

    typedef std::function< void() > ProcessingDoneCallback;
    void AddCallback( ProcessingDoneCallback a_Callback ) { m_Callbacks.push_back( a_Callback ); }
    void SetSelectedFunctions( std::unordered_map<DWORD64, SampledFunction> & a_SelectedFunctions );
    void SetGenerateSummary( bool a_Value ) { m_GenerateSummary = a_Value; }
    bool GetGenerateSummary() const { return m_GenerateSummary; }
    void SortByThreadUsage();
    void SortByThreadID();
    bool GetLineInfo( DWORD64 a_Address, LineInfo & a_LineInfo );
    void Resolve(){ ProcessSamples(); }
    void Print();
    void ProcessSamples();
    void ProcessSamplesAsync();
    void AddAddress( DWORD64 a_Address );

    std::wstring GetSymbolFromAddress( DWORD64 a_Address );
    const ThreadSampleData & GetSummary(){ return m_ThreadSampleData[0]; }

    ORBIT_SERIALIZABLE;

protected:
    void ReserveThreadData();
    void SampleThreadsAsync();
    void GetThreadCallstack( Thread * a_Thread );
    void GetThreadsUsage();
    void ProcessAddresses();
    void OutputStats();

protected:
    std::shared_ptr<Process>        m_Process;
    std::unique_ptr<std::thread>    m_SamplingThread;
    std::atomic<SamplingState>      m_State;
    BlockChain<CallStack, 16*1024 > m_Callstacks;
    Timer                           m_SamplingTimer;
    Timer                           m_ThreadUsageTimer;
    int                             m_PeriodMs;
    float                           m_SampleTimeSeconds;
    bool                            m_GenerateSummary;
    Mutex                           m_SymbolMutex;
    int                             m_NumSamples;
    bool                            m_LoadedFromFile;

    std::unordered_map<ThreadID, ThreadSampleData>              m_ThreadSampleData;
    std::unordered_map<CallstackID, std::shared_ptr<CallStack>> m_UniqueCallstacks;
    std::unordered_map<CallstackID, std::shared_ptr<CallStack>> m_UniqueResolvedCallstacks;
    std::unordered_map<CallstackID, CallstackID>                m_RawToResolvedMap;
    std::unordered_map<DWORD64, std::set<CallstackID>>          m_FunctionToCallstacks;
    std::unordered_map<DWORD64, DWORD64>                        m_ExactAddresses;
    std::unordered_map<DWORD64, std::wstring>                   m_AddressToSymbol;
    std::unordered_map<DWORD64, LineInfo>                       m_AddressToLineInfo;
    std::unordered_map<DWORD64, std::wstring>                   m_FileNames;
    std::vector< ProcessingDoneCallback >                       m_Callbacks;
    std::vector< ThreadSampleData* >                            m_SortedThreadSampleData;
};

