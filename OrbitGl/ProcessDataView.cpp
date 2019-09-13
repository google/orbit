//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ProcessDataView.h"
#include "ModuleDataView.h"
#include "Pdb.h"
#include "OrbitType.h"
#include "Capture.h"
#include "App.h"
#include "Callstack.h"
#include "Params.h"
#include "TcpServer.h"

//-----------------------------------------------------------------------------
ProcessesDataView::ProcessesDataView()
{
    m_SortingToggles.resize(PDV_NumColumns, false);
    UpdateProcessList();
    m_UpdatePeriodMs = 1000;
    m_IsRemote = false;

    GOrbitApp->RegisterProcessesDataView(this);
}

//-----------------------------------------------------------------------------
std::vector<float> ProcessesDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& ProcessesDataView::GetColumnHeaders()
{
	static std::vector<std::wstring> Columns;
    if( Columns.size() == 0 )
    { 
        Columns.push_back( L"PID" );   s_HeaderRatios.push_back( 0 );
        Columns.push_back( L"Name" );  s_HeaderRatios.push_back( 0.5f );
        Columns.push_back( L"CPU" );   s_HeaderRatios.push_back( 0 );
        Columns.push_back( L"Type" );  s_HeaderRatios.push_back( 0 );
    };

	return Columns;
}

//-----------------------------------------------------------------------------
const std::vector<float>& ProcessesDataView::GetColumnHeadersRatios()
{
    return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
std::wstring ProcessesDataView::GetValue( int row, int col )
{
    const Process & process = *GetProcess(row);
	std::wstring value;

    switch (col)
    {
    case PDV_ProcessID:
        value = std::to_wstring((long)process.GetID());     break;
    case PDV_ProcessName:
        value = process.GetName(); 
        if( process.IsElevated() ) { value+=L"*"; }
        if( process.GetIsRemote() ){ value += L"[REMOTE]"; }
        break;
    case PDV_CPU:
        value = Format( L"%.1f", process.GetCpuUsage() );    break;
    case PDV_Type:
        value = process.GetIs64Bit() ? L"64 bit" : L"32 bit"; break;
    default:                                                break;
    }

	return value;
}

//-----------------------------------------------------------------------------
std::wstring ProcessesDataView::GetToolTip( int a_Row, int a_Column )
{
    const Process & process = *GetProcess(a_Row);
    return process.GetFullName();
}

//-----------------------------------------------------------------------------
#define ORBIT_PROC_SORT( Member ) [&](int a, int b) { return OrbitUtils::Compare(processes[a]->Member, processes[b]->Member, ascending); }

//-----------------------------------------------------------------------------
void ProcessesDataView::OnSort(int a_Column, bool a_Toggle)
{
    if( a_Column == -1 )
    {
        a_Column = PdvColumn::PDV_CPU;
    }

    const std::vector< std::shared_ptr<Process> > & processes = m_ProcessList.m_Processes;
    PdvColumn pdvColumn = PdvColumn(a_Column);
    
    if (a_Toggle)
    {
        m_SortingToggles[pdvColumn] = !m_SortingToggles[pdvColumn];
    }

    bool ascending = m_SortingToggles[pdvColumn];
    std::function<bool(int a, int b)> sorter = nullptr;

    switch (pdvColumn)
    {
    case PDV_ProcessID:   sorter = ORBIT_PROC_SORT(GetID());         break;
    case PDV_ProcessName: sorter = ORBIT_PROC_SORT(GetName());       break;
    case PDV_CPU: ascending = false; sorter = ORBIT_PROC_SORT(GetCpuUsage()); break;
    case PDV_Type:        sorter = ORBIT_PROC_SORT(GetIs64Bit());    break;
    default:                                                         break;
    }

    if (sorter)
    {
        std::sort(m_Indices.begin(), m_Indices.end(), sorter);
    }

    m_LastSortedColumn = a_Column;
    SetSelectedItem();
}

//-----------------------------------------------------------------------------
void ProcessesDataView::OnSelect( int a_Index )
{
	m_SelectedProcess = GetProcess( a_Index );
    UpdateModuleDataView(m_SelectedProcess);
}

void ProcessesDataView::UpdateModuleDataView(std::shared_ptr<Process> a_Process)
{
    if (m_ModulesDataView)
    {
        if (!a_Process->GetIsRemote())
        {
            a_Process->ListModules();
        }
        else
        {
            Message msg(Msg_RemoteProcessRequest);
            msg.m_Header.m_GenericHeader.m_Address = a_Process->GetID();
            GTcpServer->Send(msg);
        }

        m_ModulesDataView->SetProcess(a_Process);
        Capture::SetTargetProcess(a_Process);
        GOrbitApp->FireRefreshCallbacks();
    }
}

//-----------------------------------------------------------------------------
void ProcessesDataView::OnTimer()
{
    Refresh();
}

//-----------------------------------------------------------------------------
void ProcessesDataView::Refresh()
{
    if( Capture::IsCapturing() )
    {
        return;
    }

    if( m_RemoteProcess )
    {
        std::shared_ptr< Process > CurrentRemoteProcess = m_ProcessList.m_Processes.size() == 1 ? m_ProcessList.m_Processes[0] : nullptr;

        if( m_RemoteProcess != CurrentRemoteProcess )
        {
            m_ProcessList.Clear();
            m_ProcessList.m_Processes.push_back( m_RemoteProcess );
            UpdateProcessList();
            SetFilter( L"" );
            SelectProcess(m_RemoteProcess->GetID());
            SetSelectedItem();
        }
    }
    else
    {
        if (!m_IsRemote)
        {
            m_ProcessList.Refresh();
            m_ProcessList.UpdateCpuTimes();
        }
        UpdateProcessList();
        OnSort( m_LastSortedColumn, false );
        OnFilter( m_Filter );
        SetSelectedItem();

        if( Capture::GTargetProcess && !Capture::IsCapturing() )
        {
            Capture::GTargetProcess->UpdateThreadUsage();
        }
    }

    GParams.m_ProcessFilter = ws2s( m_Filter );
}

//-----------------------------------------------------------------------------
void ProcessesDataView::SetSelectedItem()
{
	int initialIndex = m_SelectedIndex;
    m_SelectedIndex = -1;

    for( uint32_t i = 0; i < (uint32_t)GetNumElements(); ++i )
    {
        if(m_SelectedProcess && GetProcess( i )->GetID() == m_SelectedProcess->GetID() )
        {
            m_SelectedIndex = i;
            return;
        }
    }

	if( GParams.m_AutoReleasePdb && initialIndex != -1 )
	{
		ClearSelectedProcess();
	}
}

//-----------------------------------------------------------------------------
void ProcessesDataView::ClearSelectedProcess()
{
	std::shared_ptr<Process> process = std::make_shared<Process>();
	Capture::SetTargetProcess( process );
	m_ModulesDataView->SetProcess( process );
	m_SelectedProcess = process;
	GPdbDbg = nullptr;
	GOrbitApp->FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
bool ProcessesDataView::SelectProcess( const std::wstring & a_ProcessName )
{
	for( uint32_t i = 0; i < GetNumElements(); ++i )
	{
		Process & process = *GetProcess(i);
		if ( process.GetFullName().find( a_ProcessName ) != std::string::npos )
		{
			OnSelect(i);
            Capture::GPresetToLoad = L"";
            return true;
		}
	}

    return false;
}

//-----------------------------------------------------------------------------
std::shared_ptr<Process> ProcessesDataView::SelectProcess( DWORD a_ProcessId )
{
    Refresh();

    for( uint32_t i = 0; i < GetNumElements(); ++i )
    {       
        Process & process = *GetProcess( i );
        if( process.GetID() == a_ProcessId )
        {
            OnSelect( i );
            Capture::GPresetToLoad = L"";
            return m_SelectedProcess;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
void ProcessesDataView::OnFilter( const std::wstring & a_Filter )
{
    std::vector<uint32_t> indices;
    const std::vector<std::shared_ptr<Process>> & processes = m_ProcessList.m_Processes;

    std::vector< std::wstring > tokens = Tokenize( ToLower( a_Filter ) );

    for (uint32_t i = 0; i < processes.size(); ++i)
    {
        const Process & process = *processes[i];
        std::wstring name = ToLower( process.GetName() );
        std::wstring type = process.GetIs64Bit() ? L"64" : L"32";

        bool match = true;

        for( std::wstring & filterToken : tokens )
        {
            if (!(name.find(filterToken) != std::wstring::npos ||
                type.find(filterToken) != std::wstring::npos))
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            indices.push_back(i);
        }
    }

    m_Indices = indices;

    if (m_LastSortedColumn != -1)
    {
        OnSort(m_LastSortedColumn, false);
    }
}

//-----------------------------------------------------------------------------
void ProcessesDataView::UpdateProcessList()
{
    size_t numProcesses = m_ProcessList.m_Processes.size();
    m_Indices.resize(numProcesses);
    for (uint32_t i = 0; i < numProcesses; ++i)
    {
        m_Indices[i] = i;
    }
}

//-----------------------------------------------------------------------------
void ProcessesDataView::SetRemoteProcessList(std::shared_ptr<ProcessList> a_RemoteProcessList)
{
    m_IsRemote = true;
    m_ProcessList = *a_RemoteProcessList;
    UpdateProcessList();
    OnSort(m_LastSortedColumn, false);
    OnFilter(m_Filter);
    SetSelectedItem();
}

//-----------------------------------------------------------------------------
void ProcessesDataView::SetRemoteProcess( std::shared_ptr<Process> a_Process )
{
    std::shared_ptr<Process> targetProcess = m_ProcessList.GetProcess(a_Process->GetID());
    if (targetProcess)
    {
        m_SelectedProcess = a_Process;
        UpdateModuleDataView(m_SelectedProcess);
    }
    else
    {
        m_RemoteProcess = a_Process;
    }
}

//-----------------------------------------------------------------------------
std::shared_ptr<Process> ProcessesDataView::GetProcess(unsigned int a_Row) const
{
    return m_ProcessList.m_Processes[m_Indices[a_Row]];
}
