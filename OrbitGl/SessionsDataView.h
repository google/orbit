//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataViewModel.h"
#include <memory>

class Session;

class SessionsDataView : public DataViewModel
{
public:
    SessionsDataView();
    virtual const std::vector<std::wstring>& GetColumnHeaders() override;
    virtual const std::vector<float>& GetColumnHeadersRatios() override;
    virtual std::vector<std::wstring> GetContextMenu( int a_Index ) override;
    virtual std::wstring GetValue( int a_Row, int a_Column ) override;
    virtual std::wstring GetToolTip( int a_Row, int a_Column ) override;
    virtual std::wstring GetLabel() override { return L"Sessions"; }
    
    void OnDataChanged() override;
    void OnFilter( const std::wstring & a_Filter ) override;
    void OnSort( int a_Column, bool a_Toggle = true ) override;
    void OnContextMenu( const std::wstring & a_Action, int a_MenuIndex, std::vector<int> & a_ItemIndices ) override;
    
    void SetSessions( const std::vector< std::shared_ptr< Session > > & a_Sessions );

    enum SdvColumn
    {
        SDV_SessionName,
        SDV_ProcessName,
        //SDV_LastUsed,
        SDV_NumColumns
    };

protected:
    const std::shared_ptr<Session> & GetSession( unsigned int a_Row ) const;

protected:
    std::vector< std::shared_ptr<Session> > m_Sessions;
    static std::vector<float>               s_HeaderRatios;
};

