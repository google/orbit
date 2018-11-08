//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "DataView.h"

class GlobalsDataView : public DataView
{
public:
    GlobalsDataView();

    const std::vector<std::wstring>& GetColumnHeaders() override;
    const std::vector<float>& GetColumnHeadersRatios() override;
    std::vector<std::wstring> GetContextMenu(int a_Index) override;
    std::wstring GetValue(int a_Row, int a_Column) override;

    void OnFilter(const std::wstring & a_Filter) override;
    void ParallelFilter();
    void OnSort(int a_Column, bool a_Toggle = true) override;
    void OnContextMenu( const std::wstring & a_Action, int a_MenuIndex, std::vector<int> & a_ItemIndices ) override;
    void OnDataChanged() override;
    void OnAddToWatch( std::vector<int> & a_Items );

protected:
    Variable & GetVariable(unsigned int a_Row) const;
    std::vector< std::wstring > m_FilterTokens;
    static std::vector<int>     s_HeaderMap;
    static std::vector<float>   s_HeaderRatios;
};

