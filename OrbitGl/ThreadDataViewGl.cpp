//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "ThreadDataViewGl.h"
#include "ThreadView.h"
#include "Capture.h"

//-----------------------------------------------------------------------------
ThreadDataViewGl::ThreadDataViewGl()
{

}

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& ThreadDataViewGl::GetColumnHeaders()
{
    static std::vector< std::wstring > columns = { L"ThreadId", L"History", L"Usage" };
    return columns;
}

//-----------------------------------------------------------------------------
void ThreadDataViewGl::OnSort( int a_Column, bool a_Toggle )
{
    switch ( (ColumnType)a_Column )
    {
    case ColumnType::THREAD_ID:
        Capture::GTargetProcess->SortThreadsById();
        break;
    case ColumnType::HISTORY:
    case ColumnType::USAGE:
        Capture::GTargetProcess->SortThreadsByUsage();
        break;
    }
}

//-----------------------------------------------------------------------------
void ThreadDataViewGl::SetGlPanel( class GlPanel* a_GlPanel )
{
    if( a_GlPanel->GetType() == GlPanel::THREADS )
    {
        m_ThreadView = static_cast<ThreadView*>( a_GlPanel );
    }
}
