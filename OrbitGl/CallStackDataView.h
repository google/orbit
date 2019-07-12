//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "FunctionDataView.h"

struct CallStack;

//-----------------------------------------------------------------------------
class CallStackDataView : public FunctionsDataView
{
public:
    CallStackDataView();
    void SetAsMainInstance() override;
    size_t GetNumElements() override;
    bool SortAllowed() override { return false; }
    void OnDataChanged() override;
    std::wstring GetValue( int a_Row, int a_Column ) override;
    void OnFilter( const std::wstring & a_Filter ) override;
    void SetCallStack( std::shared_ptr<CallStack> a_CallStack ){ m_CallStack = a_CallStack; OnDataChanged(); }

protected:
    Function & GetFunction( unsigned int a_Row ) override;

    std::shared_ptr<CallStack> m_CallStack;
};

