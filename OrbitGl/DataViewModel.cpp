//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "DataViewModel.h"
#include "TypeDataView.h"
#include "FunctionDataView.h"
#include "CallStackDataView.h"
#include "LiveFunctionDataView.h"
#include "GlobalDataView.h"
#include "ProcessDataView.h"
#include "ModuleDataView.h"
#include "SamplingReportDataView.h"
#include "SessionsDataView.h"
#include "LogDataView.h"
#include "ThreadDataViewGl.h"
#include "Pdb.h"
#include "App.h"
#include "OrbitType.h"
#include "Log.h"
#include "Params.h"

//-----------------------------------------------------------------------------
DataViewModel::~DataViewModel()
{
    if( GOrbitApp )
    {
        GOrbitApp->Unregister( this );
    }
}

//-----------------------------------------------------------------------------
DataViewModel* DataViewModel::Create( DataViewType a_Type )
{
    DataViewModel* model = nullptr;
    switch( a_Type )
    {
        case DataViewType::FUNCTIONS:      model = new FunctionsDataView();      break;
        case DataViewType::TYPES:          model = new TypesDataView();          break;
        case DataViewType::LIVEFUNCTIONS:  model = new LiveFunctionsDataView();  break;
        case DataViewType::CALLSTACK:      model = new CallStackDataView();      break;
        case DataViewType::GLOBALS:        model = new GlobalsDataView();        break;
        case DataViewType::MODULES:        model = new ModulesDataView();        break;
        case DataViewType::SAMPLING:       model = new SamplingReportDataView(); break;
        case DataViewType::PROCESSES:      model = new ProcessesDataView();      break;
        case DataViewType::THREADS:        model = new ThreadDataViewGl();       break;
        case DataViewType::SESSIONS:       model = new SessionsDataView();       break;
        case DataViewType::LOG:            model = new LogDataView();            break;
        default:                                                                 break;
    }

    model->m_Type = a_Type;
    return model;
}

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& DataViewModel::GetColumnHeaders()
{
    static std::vector<std::wstring> columns = { L"Invalid Header" };
    return columns;
}

//-----------------------------------------------------------------------------
const std::vector<float>& DataViewModel::GetColumnHeadersRatios()
{
    static std::vector<float> empty;
    return empty;
}

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& DataViewModel::GetContextMenu(int a_Index)
{
    static std::vector<std::wstring> empty;
    return empty;
}
