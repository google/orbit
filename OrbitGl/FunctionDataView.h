//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "DataViewModel.h"

class FunctionsDataView : public DataViewModel
{
public:
    FunctionsDataView();

    virtual const std::vector<std::wstring>& GetColumnHeaders() override;
    virtual const std::vector<float>& GetColumnHeadersRatios() override;
    virtual const std::vector<std::wstring>& GetContextMenu(int a_Index);
    virtual std::wstring GetValue( int a_Row, int a_Column );

    void OnFilter( const std::wstring & a_Filter ) override;
    void ParallelFilter();
    void OnSort(int a_Column, bool a_Toggle = true) override;
    void OnContextMenu( int a_Index, std::vector<int> & a_ItemIndices ) override;
    void OnDataChanged() override;

    virtual bool SortAllowed() { return true; }

protected:
    virtual Function & GetFunction( unsigned int a_Row );

    std::vector< std::wstring > m_FilterTokens;
    static std::vector<int>     s_HeaderMap;
    static std::vector<float>   s_HeaderRatios;
};

