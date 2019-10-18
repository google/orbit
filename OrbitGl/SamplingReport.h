//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

//-----------------------------------------------------------------------------
class SamplingReport
{
public:
    SamplingReport( std::shared_ptr< class SamplingProfiler > a_SamplingProfiler );
    ~SamplingReport();

    void FillReport();
    std::shared_ptr< class SamplingProfiler > GetProfiler() const { return m_Profiler; }
    const std::vector< std::shared_ptr<class DataView> > & GetThreadReports() { return m_ThreadReports; }
    void SetCallstackDataView( class CallStackDataView* a_DataView ){ m_CallstackDataView = a_DataView; }
    void OnSelectAddress( uint64_t a_Address, uint32_t a_ThreadId );
    void OnCallstackIndexChanged( int a_Index );
    void IncrementCallstackIndex();
    void DecrementCallstackIndex();
    std::wstring GetSelectedCallstackString();
    void SetUiRefreshFunc( std::function<void()> a_Func ){ m_UiRefreshFunc = a_Func; }
    std::shared_ptr< struct SortedCallstackReport >     m_SelectedSortedCallstackReport;

protected:
    std::shared_ptr< class SamplingProfiler >           m_Profiler;
    std::vector< std::shared_ptr<class DataView> >      m_ThreadReports;
    CallStackDataView*                                  m_CallstackDataView;
    
    unsigned long long                                  m_SelectedAddress;
    int                                                 m_SelectedAddressCallstackIndex;
    std::function<void()>                               m_UiRefreshFunc;
};

