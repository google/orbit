#include "LogDataView.h"
#include "App.h"
#include "TcpServer.h"
#include "Capture.h"
#include "App.h"
#include "Callstack.h"
#include "SamplingProfiler.h"
#include <chrono>

std::vector<float> LogDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
LogDataView::LogDataView()
{
    GOrbitApp->RegisterOutputLog( this );
    GTcpServer->SetCallback( Msg_OrbitLog, [=]( const Message & a_Msg ){ this->OnReceiveMessage(a_Msg); } );
    m_UpdatePeriodMs = 100;
}

//-----------------------------------------------------------------------------
const std::vector<std::wstring>& LogDataView::GetColumnHeaders()
{
    static std::vector<std::wstring> Columns;
    if( Columns.size() == 0 )
    {
        Columns.push_back( L"Log" );      s_HeaderRatios.push_back(0.7f);
        Columns.push_back( L"Time" );     s_HeaderRatios.push_back(0.15f);
        Columns.push_back( L"ThreadId" ); s_HeaderRatios.push_back(0.15f);
    }
    return Columns;
}

//-----------------------------------------------------------------------------
const std::vector<float>& LogDataView::GetColumnHeadersRatios()
{
    return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
std::wstring LogDataView::GetValue( int a_Row, int a_Column )
{
    const OrbitLogEntry & entry = GetEntry( a_Row );
    std::wstring value;

    switch( a_Column )
    {
    case LDV_Time:
    {
        TickType micros = (TickType)MicroSecondsFromTicks(Capture::GCaptureTimer.m_Start, entry.m_Time);
        std::chrono::system_clock::time_point sysTime = Capture::GCaptureTimePoint + std::chrono::microseconds(micros);
        std::time_t now_c = std::chrono::system_clock::to_time_t(sysTime);
        std::tm now_tm;
        #ifdef WIN32
        localtime_s(&now_tm, &now_c);
        #else
        now_tm = *std::localtime(&now_c);
        #endif
        TCHAR buffer[256];
        wcsftime( buffer, sizeof( buffer ), L"%H:%M:%S", &now_tm );

        value = buffer;
        break;
    }
    case LDV_Message:
        value = s2ws(entry.m_Text); break;
    case LDV_ThreadId:
        value = Format(L"%u", entry.m_ThreadId); break;
    default: break;
    }

    return value;
}

//-----------------------------------------------------------------------------
std::wstring LogDataView::GetToolTip( int a_Row, int a_Column )
{
    return std::wstring();
}

//-----------------------------------------------------------------------------
bool LogDataView::ScrollToBottom()
{
    return true;
}

//-----------------------------------------------------------------------------
bool LogDataView::SkipTimer()
{
    return !Capture::IsCapturing();
}

//-----------------------------------------------------------------------------
void LogDataView::OnDataChanged()
{
    ScopeLock lock( m_Mutex );
    m_Indices.resize( m_Entries.size() );
    for( int i = 0; i < m_Entries.size(); ++i )
    {
        m_Indices[i] = i;
    }
}

//-----------------------------------------------------------------------------
void LogDataView::OnFilter( const std::wstring & a_Filter )
{
    std::vector< std::string > tokens = Tokenize( ToLower( ws2s( a_Filter ) ) );
    std::vector<int> indices;

    for( int i = 0; i < (int)m_Entries.size(); ++i )
    {
        const OrbitLogEntry & entry = m_Entries[i];
        std::string text = ToLower( entry.m_Text );

        bool match = true;

        for( std::string & filterToken : tokens )
        {
            if( !( text.find( filterToken ) != std::wstring::npos ) )
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

    m_Indices = indices;
}

//-----------------------------------------------------------------------------
std::vector<std::wstring> LogDataView::GetContextMenu( int a_Index )
{
    const OrbitLogEntry & entry = LogDataView::GetEntry( a_Index );
    m_SelectedCallstack = Capture::GetCallstack( entry.m_CallstackHash );
    std::vector<std::wstring> menu;
    if( m_SelectedCallstack )
    {
        for( int i = 0; i < m_SelectedCallstack->m_Depth; ++i )
        {
            DWORD64 addr = m_SelectedCallstack->m_Data[i];
            menu.push_back( Capture::GSamplingProfiler->GetSymbolFromAddress(addr) );
        }
    }
    Append( menu, DataView::GetContextMenu(a_Index) );
    return menu;
}

//-----------------------------------------------------------------------------
void LogDataView::OnContextMenu( const std::wstring & a_Action, int a_MenuIndex, std::vector<int> & a_ItemIndices )
{
    if( m_SelectedCallstack && m_SelectedCallstack->m_Depth > a_MenuIndex )
    {
        GOrbitApp->GoToCode( m_SelectedCallstack->m_Data[a_MenuIndex] );
    }
    else
    {
        DataView::OnContextMenu( a_Action, a_MenuIndex, a_ItemIndices );
    }
}

//-----------------------------------------------------------------------------
void LogDataView::Add( const OrbitLogEntry & a_Msg )
{
    ScopeLock lock( m_Mutex );
    m_Entries.push_back( a_Msg );
    OnDataChanged();
}

//-----------------------------------------------------------------------------
const OrbitLogEntry & LogDataView::GetEntry( unsigned int a_Row ) const
{
    return m_Entries[m_Indices[a_Row]];
}

//-----------------------------------------------------------------------------
void LogDataView::OnReceiveMessage( const Message & a_Msg )
{
    bool isLog = a_Msg.GetType() == Msg_OrbitLog;
    assert(isLog);
    if( isLog )
    {
        OrbitLogEntry entry;
        memcpy( &entry, a_Msg.GetData(), OrbitLogEntry::GetSizeWithoutString() );
        const char* log = a_Msg.GetData() + OrbitLogEntry::GetSizeWithoutString();
        entry.m_Text = log;
        RemoveTrailingNewLine(entry.m_Text);
        Add(entry);
    }
}
