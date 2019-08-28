//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "LiveFunctionDataView.h"
#include "OrbitType.h"
#include "Capture.h"
#include "Log.h"
#include "App.h"
#include "Pdb.h"
#include "FunctionStats.h"

//-----------------------------------------------------------------------------
namespace LiveFunction
{
    enum Columns
    {
        SELECTED,
        NAME,
        COUNT,
        TIME_TOTAL,
        TIME_AVG,
        TIME_MIN,
        TIME_MAX,
        ADDRESS,
        MODULE,
        INDEX,
        NUM_EXPOSED_MEMBERS
    };
}

//-----------------------------------------------------------------------------
LiveFunctionsDataView::LiveFunctionsDataView()
{
    GOrbitApp->RegisterLiveFunctionsDataView(this);
    m_SortingToggles.resize(LiveFunction::NUM_EXPOSED_MEMBERS, false);
    m_UpdatePeriodMs = 300;
    m_LastSortedColumn = 3;/*Count*/
    GetColumnHeaders();
    OnDataChanged();
}

//-----------------------------------------------------------------------------
std::vector<int>   LiveFunctionsDataView::s_HeaderMap;
std::vector<float> LiveFunctionsDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& LiveFunctionsDataView::GetColumnHeaders()
{
    static std::vector<std::wstring> Columns;

    if( s_HeaderMap.size() == 0 )
    {
        Columns.push_back(L"selected"); s_HeaderMap.push_back(LiveFunction::SELECTED);  s_HeaderRatios.push_back(0);
        Columns.push_back(L"Index");    s_HeaderMap.push_back(LiveFunction::INDEX);     s_HeaderRatios.push_back(0);
        Columns.push_back(L"Function"); s_HeaderMap.push_back(LiveFunction::NAME);      s_HeaderRatios.push_back(0.5f);
        Columns.push_back(L"Count");    s_HeaderMap.push_back(LiveFunction::COUNT);     s_HeaderRatios.push_back(0);
        Columns.push_back(L"Total");    s_HeaderMap.push_back(LiveFunction::TIME_TOTAL);s_HeaderRatios.push_back(0);
        Columns.push_back(L"Avg");      s_HeaderMap.push_back(LiveFunction::TIME_AVG);  s_HeaderRatios.push_back(0);
        Columns.push_back(L"Min");      s_HeaderMap.push_back(LiveFunction::TIME_MIN);  s_HeaderRatios.push_back(0);
        Columns.push_back(L"Max");      s_HeaderMap.push_back(LiveFunction::TIME_MAX);  s_HeaderRatios.push_back(0);
        Columns.push_back(L"Module");   s_HeaderMap.push_back(LiveFunction::MODULE);    s_HeaderRatios.push_back(0);
        Columns.push_back(L"Address");  s_HeaderMap.push_back(LiveFunction::ADDRESS);   s_HeaderRatios.push_back(0);
    }

    return Columns;
}

//-----------------------------------------------------------------------------
const std::vector<float>& LiveFunctionsDataView::GetColumnHeadersRatios()
{
    return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
std::wstring LiveFunctionsDataView::GetValue( int a_Row, int a_Column )
{
    if( a_Row >= (int)GetNumElements() )
    {
        return L"";
    }

    Function & function = GetFunction( a_Row );
    std::shared_ptr<FunctionStats> stats = function.m_Stats;

    std::string value;
    
    switch ( s_HeaderMap[a_Column] )
    {
    case LiveFunction::SELECTED:
        value = function.IsSelected() ? "X" : "-"; break;
    case LiveFunction::INDEX:
        value = Format("%d", a_Row); break;
    case LiveFunction::NAME:
        value = function.PrettyName(); break;
    case LiveFunction::COUNT:
        value = Format( "%lu", stats->m_Count ); break;
    case LiveFunction::TIME_TOTAL:
        value = GetPrettyTime(stats->m_TotalTimeMs); break;
    case LiveFunction::TIME_AVG:
        value = GetPrettyTime(stats->m_AverageTimeMs); break;
    case LiveFunction::TIME_MIN:
        value = GetPrettyTime(stats->m_MinMs); break;
    case LiveFunction::TIME_MAX:
        value = GetPrettyTime(stats->m_MaxMs); break;
    case LiveFunction::ADDRESS:
        value = function.m_Pdb ? Format("0x%llx", function.m_Address + (DWORD64)function.m_Pdb->GetHModule()) : ""; break;
    case LiveFunction::MODULE:
        value = function.m_Pdb ? ws2s(function.m_Pdb->GetName()) : ""; break;
    default: break;
    }

    return s2ws(value);
}

//-----------------------------------------------------------------------------
#define ORBIT_FUNC_SORT( Member ) [&](int a, int b) { return OrbitUtils::Compare(functions[a]->Member, functions[b]->Member, ascending); }
#define ORBIT_STAT_SORT( Member ) [&](int a, int b) { return OrbitUtils::Compare(functions[a]->m_Stats->Member, functions[b]->m_Stats->Member, ascending); }

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnSort( int a_Column, bool a_Toggle )
{
    const std::vector<Function*> & functions = m_Functions;
    auto MemberID = LiveFunction::Columns( s_HeaderMap[a_Column] );

    if (a_Toggle)
    {
        m_SortingToggles[MemberID] = !m_SortingToggles[MemberID];
    }

    bool ascending = m_SortingToggles[MemberID];
    std::function<bool(int a, int b)> sorter = nullptr;

    switch (MemberID)
    {
    case LiveFunction::NAME:     sorter = ORBIT_FUNC_SORT( m_PrettyName );     break;
    case LiveFunction::COUNT: ascending = false; sorter = ORBIT_STAT_SORT( m_Count ); break;
    case LiveFunction::TIME_TOTAL: sorter = ORBIT_STAT_SORT( m_TotalTimeMs );  break;
    case LiveFunction::TIME_AVG: sorter = ORBIT_STAT_SORT( m_AverageTimeMs );  break;
    case LiveFunction::TIME_MIN: sorter = ORBIT_STAT_SORT( m_MinMs );          break;
    case LiveFunction::TIME_MAX: sorter = ORBIT_STAT_SORT( m_MaxMs );          break;
    case LiveFunction::ADDRESS:  sorter = ORBIT_FUNC_SORT( m_Address );        break;
    case LiveFunction::MODULE:   sorter = ORBIT_FUNC_SORT( m_Pdb->GetName() ); break;
    case LiveFunction::SELECTED: sorter = ORBIT_FUNC_SORT( IsSelected() );     break;
    default:                                                                   break;
    }

    if( sorter ) 
    {
        std::sort(m_Indices.begin(), m_Indices.end(), sorter);
    }

    m_LastSortedColumn = a_Column;
}

//-----------------------------------------------------------------------------
std::wstring TOGGLE_SELECT = L"Toggle Select";

//-----------------------------------------------------------------------------
std::vector<std::wstring> LiveFunctionsDataView::GetContextMenu(int a_Index)
{
    std::vector<std::wstring> menu = { TOGGLE_SELECT };
    Append( menu, DataView::GetContextMenu(a_Index) );
    return menu;
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnContextMenu( const std::wstring & a_Action, int a_MenuIndex, std::vector<int> & a_ItemIndices )
{
    if( a_Action == TOGGLE_SELECT )
    {
        for( int i : a_ItemIndices )
        {
            Function & func = GetFunction( i );
            func.ToggleSelect();
        }
    }
    else
    {
        DataView::OnContextMenu( a_Action, a_MenuIndex, a_ItemIndices );
    }
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnFilter( const std::wstring & a_Filter )
{
    std::vector<uint32_t> indices;

    std::vector< std::wstring > tokens = Tokenize( ToLower( a_Filter ) );

    for( uint32_t i = 0; i < (uint32_t)m_Functions.size(); ++i )
    {
        const Function* function = m_Functions[i];
        if( function )
        {
            std::wstring name = ToLower( s2ws(function->m_PrettyName) );
            //std::string file = ToLower( function.m_File );

            bool match = true;

            for( std::wstring & filterToken : tokens )
            {
                if( !( name.find( filterToken ) != std::wstring::npos/* ||
                       file.find( filterToken ) != std::string::npos*/ ) )
                {
                    match = false;
                    break;
                }
            }

            if( match )
            {
                indices.push_back(i);
            }
        }
    }

    m_Indices = indices;
    
    if( m_LastSortedColumn != -1 )
    {
        OnSort(m_LastSortedColumn, false);
    }

    // Filter drawn textboxes
    Capture::GVisibleFunctionsMap.clear();
    for( uint32_t i = 0; i < (uint32_t)m_Indices.size(); ++i )
    {
        Function & func = GetFunction(i);
        Capture::GVisibleFunctionsMap[func.GetVirtualAddress()] = &func;
    }

    GOrbitApp->NeedsRedraw();
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnDataChanged()
{
    size_t numFunctions = Capture::GFunctionCountMap.size();
    m_Indices.resize(numFunctions);
    for (uint32_t i = 0; i < numFunctions; ++i)
    {
        m_Indices[i] = i;
    }

    m_Functions.clear();
    for( auto & pair : Capture::GFunctionCountMap )
    {
        const ULONG64& address = pair.first;
        Function* func = Capture::GSelectedFunctionsMap[address];
        m_Functions.push_back( func );
    }

    OnFilter( m_Filter );
}

//-----------------------------------------------------------------------------
void LiveFunctionsDataView::OnTimer()
{
    if( Capture::IsCapturing() )
    {
        OnSort( m_LastSortedColumn, false );
    }
}

//-----------------------------------------------------------------------------
Function & LiveFunctionsDataView::GetFunction( unsigned int a_Row ) const
{
    assert( a_Row < m_Functions.size() );
    assert( m_Functions[m_Indices[a_Row]] );
    return *m_Functions[m_Indices[a_Row]];
}
