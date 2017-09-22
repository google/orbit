//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "DataViewModel.h"

class ThreadDataViewGl : public DataViewModel
{
public:
    ThreadDataViewGl();

    enum ColumnType { THREAD_ID, HISTORY, USAGE };

    virtual const std::vector<std::wstring>& GetColumnHeaders();
    void OnSort(int a_Column, bool a_Toggle = true) override;
    void SetGlPanel( class GlPanel* a_GlPanel ) override;

protected:
    class ThreadView* m_ThreadView;
};
