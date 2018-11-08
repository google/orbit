//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "DataView.h"

class ThreadDataViewGl : public DataView
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
