//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "DataViewModel.h"

class GlobalsDataView : public DataViewModel
{
public:
    GlobalsDataView();

    virtual const std::vector<std::wstring>& GetColumnHeaders() override;
    virtual const std::vector<float>& GetColumnHeadersRatios() override;
    virtual const std::vector<std::wstring>& GetContextMenu(int a_Index) override;
    virtual std::wstring GetValue(int a_Row, int a_Column) override;

    virtual void OnFilter(const std::wstring & a_Filter) override;
    void ParallelFilter();
    virtual void OnSort(int a_Column, bool a_Toggle = true) override;
    virtual void OnContextMenu( int a_Index, std::vector<int> & a_ItemIndices ) override;
    void OnDataChanged() override;
    void OnAddToWatch( std::vector<int> & a_Items );

protected:
    Variable & GetVariable(unsigned int a_Row) const;
    std::vector< std::wstring > m_FilterTokens;
    static std::vector<int>     s_HeaderMap;
    static std::vector<float>   s_HeaderRatios;
};

