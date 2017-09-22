//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataViewModel.h"
#include "OrbitType.h"
#include <vector>

class TypesDataView : public DataViewModel
{
public:
    TypesDataView();

    virtual const std::vector<std::wstring>& GetColumnHeaders() override;
    virtual const std::vector<float>& GetColumnHeadersRatios() override;
    virtual const std::vector<std::wstring>& GetContextMenu(int a_Index);
    virtual std::wstring GetValue(int a_Row, int a_Column);

    void OnFilter(const std::wstring & a_Filter)  override;
    void ParallelFilter(const std::wstring & a_Filter);
    void OnSort(int a_Column, bool a_Toggle)     override;
    void OnContextMenu( int a_Index, std::vector<int> & a_ItemIndices ) override;
    void OnDataChanged() override;

protected:
    Type & GetType(unsigned int a_Row) const;

    void OnProp(std::vector<int> & a_Items);
    void OnView(std::vector<int> & a_Items);
    void OnClip(std::vector<int> & a_Items);

    std::vector< std::wstring > m_FilterTokens;
    static std::vector<int>     s_HeaderMap;
    static std::vector<float>   s_HeaderRatios;
};

