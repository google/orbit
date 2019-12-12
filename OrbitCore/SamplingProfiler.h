//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "Pdb.h"
#include "Callstack.h"
#include "BlockChain.h"
#include "SerializationMacros.h"
#include "EventBuffer.h"

class Process;
class Thread;

//-----------------------------------------------------------------------------
struct SampledFunction
{
    SampledFunction() {}
    unsigned long long Hash();
    bool GetSelected() const;
    std::wstring m_Name;
    std::wstring m_Module;
    std::wstring m_File;
    float        m_Exclusive = 0;
    float        m_Inclusive = 0;
    int          m_Line = 0;
    uint64_t     m_Address = 0;
    uint64_t     m_Hash = 0;
    Function*    m_Function = 0;

    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct LineInfo
{
    LineInfo() {}
    std::wstring  m_File;
    uint32_t      m_Line = 0;
    uint64_t      m_Address = 0;
    uint64_t      m_FileNameHash = 0;

    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct ThreadSampleData
{
    ThreadSampleData() { m_ThreadUsage.push_back(0); }
    void ComputeAverageThreadUsage();
    std::multimap< int, CallstackID > SortCallstacks( const std::set< CallstackID > & a_CallStacks, int & o_TotalCallStacks );
    std::unordered_map< CallstackID, unsigned int > m_CallstackCount;
    std::unordered_map< uint64_t, unsigned int >    m_AddressCount;
    std::unordered_map< uint64_t, unsigned int >    m_ExclusiveCount;
    std::multimap< unsigned int, uint64_t >         m_AddressCountSorted;
    unsigned int                                    m_NumSamples = 0;
    std::vector< SampledFunction >                  m_SampleReport;
    std::vector< float >                            m_ThreadUsage;
    float                                           m_AverageThreadUsage = 0;
    ThreadID                                        m_TID = 0;
    
    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct CallstackCount
{
    CallstackCount() {}
    int         m_Count = 0;
    CallstackID m_CallstackId;
    ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
struct SortedCallstackReport
{
    SortedCallstackReport() {}
    int m_NumCallStacksTotal = 0;
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

    void AddCallStack( CallStack & a_CallStack );
    void AddHashedCallStack( CallstackEvent & a_CallStack );
    void AddUniqueCallStack( CallStack & a_CallStack );
  
    const std::shared_ptr<CallStack> GetCallStack( CallstackID a_ID )
    {
        ScopeLock lock(m_Mutex);
        return m_UniqueCallstacks[a_ID];
    }
    
    inline bool HasCallStack( CallstackID a_ID )
    {
        ScopeLock lock(m_Mutex);
        auto it = m_UniqueCallstacks.find( a_ID ); 
        return it != m_UniqueCallstacks.end();
    }

    std::multimap<int, CallstackID> GetCallStacksFromAddress( uint64_t a_Addr, ThreadID a_TID, int & o_NumCallstacks );
    std::shared_ptr< SortedCallstackReport > GetSortedCallstacksFromAddress( uint64_t a_Addr, ThreadID a_TID );
    
    enum SamplingState { Idle, Sampling, PendingStop, Processing, DoneProcessing };
    SamplingState GetState() const { return m_State; }
    void SetState( SamplingState a_State ){ m_State = a_State; }
    const std::vector< ThreadSampleData* > & GetThreadSampleData() const { return m_SortedThreadSampleData; }
    void SetLoadedFromFile(bool a_Value = true) { m_LoadedFromFile = a_Value; }
    void SetIsLinuxPerf( bool a_Value = true ) { m_IsLinuxPerf = a_Value; }

    typedef std::function< void() > ProcessingDoneCallback;
    void AddCallback( ProcessingDoneCallback a_Callback ) { m_Callbacks.push_back( a_Callback ); }
    void SetSelectedFunctions( std::unordered_map<uint64_t, SampledFunction> & a_SelectedFunctions );
    void SetGenerateSummary( bool a_Value ) { m_GenerateSummary = a_Value; }
    bool GetGenerateSummary() const { return m_GenerateSummary; }
    void SortByThreadUsage();
    void SortByThreadID();
    bool GetLineInfo( uint64_t a_Address, LineInfo & a_LineInfo );
    void Resolve(){ ProcessSamples(); }
    void Print();
    void ProcessSamples();
    void ProcessSamplesAsync();
    void AddAddress( uint64_t a_Address );

    std::wstring GetSymbolFromAddress( uint64_t a_Address );
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
    std::shared_ptr<Process>                m_Process;
    std::unique_ptr<std::thread>            m_SamplingThread;
    std::atomic<SamplingState>              m_State;
    BlockChain<CallstackEvent, 16*1024 >    m_Callstacks;
    Timer                                   m_SamplingTimer;
    Timer                                   m_ThreadUsageTimer;
    int                                     m_PeriodMs = 1;
    float                                   m_SampleTimeSeconds = FLT_MAX;
    bool                                    m_GenerateSummary = true;
    Mutex                                   m_Mutex;
    int                                     m_NumSamples = 0;
    bool                                    m_LoadedFromFile = false;
    bool                                    m_IsLinuxPerf = false;

    std::unordered_map<ThreadID, ThreadSampleData>              m_ThreadSampleData;
    std::unordered_map<CallstackID, std::shared_ptr<CallStack>> m_UniqueCallstacks;
    std::unordered_map<CallstackID, std::shared_ptr<CallStack>> m_UniqueResolvedCallstacks;
    std::unordered_map<CallstackID, CallstackID>                m_RawToResolvedMap;
    std::unordered_map<uint64_t, std::set<CallstackID>>         m_FunctionToCallstacks;
    std::unordered_map<uint64_t, uint64_t>                      m_ExactAddresses;
    std::unordered_map<uint64_t, std::wstring>                  m_AddressToSymbol;
    std::unordered_map<uint64_t, LineInfo>                      m_AddressToLineInfo;
    std::unordered_map<uint64_t, std::wstring>                  m_FileNames;
    std::vector< ProcessingDoneCallback >                       m_Callbacks;
    std::vector< ThreadSampleData* >                            m_SortedThreadSampleData;
};

