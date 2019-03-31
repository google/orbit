//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "GlCanvas.h"
#include "TimeGraph.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "OrbitType.h"
#include "Pdb.h"
#include "Log.h"
#include "Capture.h"
#include "Params.h"
#include "Utils.h"
#include "EventTrack.h"
#include "Geometry.h"
#include "Batcher.h"
#include "PickingManager.h"
#include "SamplingProfiler.h"
#include "App.h"
#include "OrbitUnreal.h"
#include "TimerManager.h"
#include <algorithm>

#ifdef _WIN32
#include "EventTracer.h"
#endif

TimeGraph* GCurrentTimeGraph = nullptr;

//-----------------------------------------------------------------------------
TimeGraph::TimeGraph() : m_NumDrawnTextBoxes(0)
                       , m_RefEpochTimeUs(0)
                       , m_MinEpochTimeUs(0)
                       , m_MaxEpochTimeUs(0)
                       , m_TimeWindowUs(0)
                       , m_WorldStartX(0)
                       , m_WorldWidth(0)
                       , m_SessionMinCounter(0xFFFFFFFFFFFFFFFF)
                       , m_SessionMaxCounter(0)
                       , m_Margin(40)
                       , m_MainFrameCounter(0)
                       , m_TrackAlpha(255)
                       , m_NeedsUpdatePrimitives(false)
                       , m_NeedsRedraw(false)
                       , m_DrawText(true)
                       , m_Canvas(nullptr)
{
    m_LastThreadReorder.Start();
}

//-----------------------------------------------------------------------------
void TimeGraph::SetCanvas( GlCanvas* a_Canvas )
{
    m_Canvas = a_Canvas;
    m_TextRenderer->SetCanvas( a_Canvas );
    m_TextRendererStatic.SetCanvas( a_Canvas );
}

//-----------------------------------------------------------------------------
void TimeGraph::SetFontSize(int a_FontSize)
{
	m_TextRenderer->SetFontSize(a_FontSize);
	m_TextRendererStatic.SetFontSize(a_FontSize);
}

//-----------------------------------------------------------------------------
void TimeGraph::Clear()
{
    m_Batcher.Reset();
    m_TextBoxes.clear();
    m_SessionMinCounter = 0xFFFFFFFFFFFFFFFF;
    m_SessionMaxCounter = 0;
    m_ThreadDepths.clear();
    m_ThreadCountMap.clear();
    GEventTracer.GetEventBuffer().Reset();
    m_MemTracker.Clear();
    m_Layout.Reset();
}

//-----------------------------------------------------------------------------
double GNumHistorySeconds = 2.f;

//-----------------------------------------------------------------------------
bool TimeGraph::UpdateSessionMinMaxCounter()
{
    m_SessionMinCounter = LLONG_MAX;

    if( m_TextBoxes.size() )
    {
        m_SessionMinCounter = m_TextBoxes.m_Root->m_Data[0].GetTimer().m_Start;
    }

    if( GEventTracer.GetEventBuffer().HasEvent() )
    {
        m_SessionMinCounter = std::min( (long long)m_SessionMinCounter, GEventTracer.GetEventBuffer().GetMinTime() );
        m_SessionMaxCounter = std::max( (long long)m_SessionMaxCounter, GEventTracer.GetEventBuffer().GetMaxTime() );
    }

    return m_SessionMinCounter != LLONG_MAX;
}

//-----------------------------------------------------------------------------
void TimeGraph::ZoomAll()
{    
    if( UpdateSessionMinMaxCounter() )
    {
        m_MaxEpochTimeUs = MicroSecondsFromTicks( m_SessionMinCounter, m_SessionMaxCounter );
        m_MinEpochTimeUs = m_MaxEpochTimeUs - (GNumHistorySeconds*1000*1000);
        NeedsUpdate();
    }
}

//-----------------------------------------------------------------------------
void TimeGraph::Zoom( const TextBox* a_TextBox )
{
    const Timer& timer = a_TextBox->GetTimer();

    double start = MicroSecondsFromTicks(m_SessionMinCounter, timer.m_Start);
    double end   = MicroSecondsFromTicks(m_SessionMinCounter, timer.m_End);

    double mid = (start+end)/2.0;
    double extent = 1.1*(end-start)/2.0;

    SetMinMax( mid-extent, mid+extent );
}

//-----------------------------------------------------------------------------
double TimeGraph::GetSessionTimeSpanUs()
{
    if( UpdateSessionMinMaxCounter() )
    {
        return MicroSecondsFromTicks( m_SessionMinCounter, m_SessionMaxCounter );
    }

    return 0;
}

//-----------------------------------------------------------------------------
double TimeGraph::GetCurrentTimeSpanUs()
{
    return m_MaxEpochTimeUs - m_MinEpochTimeUs;
}

//-----------------------------------------------------------------------------
void TimeGraph::ZoomTime( float a_ZoomValue, double a_MouseRatio )
{
    m_ZoomValue = a_ZoomValue;
    m_MouseRatio = a_MouseRatio;

    static double incrementRatio = 0.1;
    double scale = a_ZoomValue > 0 ? 1 + incrementRatio : 1 - incrementRatio;
    m_Scale = scale;

    double CurrentTimeWindowUs = m_MaxEpochTimeUs - m_MinEpochTimeUs;
    m_CurrentTimeWindow = CurrentTimeWindowUs;
    m_RefEpochTimeUs = m_MinEpochTimeUs + a_MouseRatio * CurrentTimeWindowUs;

    double timeLeft  = std::max( m_RefEpochTimeUs - m_MinEpochTimeUs, 0.0 );
    double timeRight = std::max( m_MaxEpochTimeUs - m_RefEpochTimeUs, 0.0 );
    m_TimeLeft = timeLeft = scale * timeLeft;
    m_TimeRight = timeRight = scale * timeRight;

    double minEpochTime = m_RefEpochTimeUs - timeLeft;
    double maxEpochTime = m_RefEpochTimeUs + timeRight;

    if (maxEpochTime - minEpochTime < 0.001 /*1 ns*/ )
    {
        return;
    }

    SetMinMax( minEpochTime, maxEpochTime );

    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::SetMinMax( double a_MinEpochTime, double a_MaxEpochTime )
{
    double desiredTimeWindow = a_MaxEpochTime - a_MinEpochTime;
    m_MinEpochTimeUs = std::max( a_MinEpochTime, 0.0 );
    m_MaxEpochTimeUs = std::min( m_MinEpochTimeUs + desiredTimeWindow, GetSessionTimeSpanUs() );
    m_CurrentTimeWindow = m_MaxEpochTimeUs - m_MinEpochTimeUs;

    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::PanTime( int a_InitialX, int a_CurrentX, int a_Width, double a_InitialTime )
{
    double m_TimeWindowUs = m_MaxEpochTimeUs - m_MinEpochTimeUs;
    double initialLocalTime = (double)a_InitialX/(double)a_Width * m_TimeWindowUs;
    double dt = (double)(a_CurrentX-a_InitialX)/(double)a_Width * m_TimeWindowUs;
    double currentTime = a_InitialTime - dt;
    m_MinEpochTimeUs = clamp( currentTime - initialLocalTime, 0.0, GetSessionTimeSpanUs()-m_TimeWindowUs );
    m_MaxEpochTimeUs = m_MinEpochTimeUs + m_TimeWindowUs;

    NeedsUpdate();
}

//-----------------------------------------------------------------------------
double TimeGraph::GetTime( double a_Ratio )
{
    double CurrentWidth = m_MaxEpochTimeUs - m_MinEpochTimeUs;
    double Delta = a_Ratio * CurrentWidth;
    return m_MinEpochTimeUs + Delta;
}

//-----------------------------------------------------------------------------
double TimeGraph::GetTimeIntervalMicro( double a_Ratio )
{
    double CurrentWidth = m_MaxEpochTimeUs - m_MinEpochTimeUs;
    return a_Ratio * CurrentWidth;
}

//-----------------------------------------------------------------------------
void TimeGraph::ProcessTimer( Timer & a_Timer )
{
    TickType start = a_Timer.m_Start;
    TickType end   = a_Timer.m_End;

    if( end > m_SessionMaxCounter )
    {
        m_SessionMaxCounter = end;
    }

    switch( a_Timer.m_Type )
    {
    case Timer::ALLOC:
        m_MemTracker.ProcessAlloc( a_Timer );
        return;
    case Timer::FREE:
        m_MemTracker.ProcessFree( a_Timer );
        return;
    case Timer::CORE_ACTIVITY:
        Capture::GHasContextSwitches = true;
        break;
    }

    if( a_Timer.m_FunctionAddress > 0 )
    {
        Function* func = Capture::GTargetProcess->GetFunctionFromAddress( a_Timer.m_FunctionAddress );
        if( func )
        {
            ++Capture::GFunctionCountMap[a_Timer.m_FunctionAddress];
            if( func->m_Stats )
            {
                func->m_Stats->Update( a_Timer );
            }
        }
    }

    if( !a_Timer.IsType( Timer::THREAD_ACTIVITY ) && !a_Timer.IsType( Timer::CORE_ACTIVITY ) )
    {
        ++m_ThreadCountMap[a_Timer.m_TID];
    }

    TextBox textBox( Vec2(0, 0), Vec2(0, 0), "", m_TextRenderer, Color( 255, 0, 0, 255) );
    textBox.SetTimer( a_Timer );
    AddTextBox( textBox );
}

//-----------------------------------------------------------------------------
void TimeGraph::AddTextBox(const TextBox& a_TextBox)
{
    m_TextBoxes.push_back( a_TextBox );
}

//-----------------------------------------------------------------------------
void TimeGraph::AddContextSwitch( const ContextSwitch & a_CS )
{
    auto pair = std::make_pair(a_CS.m_Time, a_CS);

    if( a_CS.m_Type == ContextSwitch::Out )
    {

        if( true )
        {
            // Processor time line
            std::map< long long, ContextSwitch > & csMap = m_CoreUtilizationMap[a_CS.m_ProcessorIndex];

            if( csMap.rbegin() != csMap.rend() )
            {
                ContextSwitch & lastCS = csMap.rbegin()->second;
                if( lastCS.m_Type == ContextSwitch::In )
                {
                    Timer timer;
                    timer.m_Start = lastCS.m_Time;
                    timer.m_End = a_CS.m_Time;
                    timer.m_TID = a_CS.m_ThreadId;
                    timer.m_Processor = (int8_t)a_CS.m_ProcessorIndex;
                    timer.m_SessionID = Message::GSessionID;
                    timer.SetType(Timer::CORE_ACTIVITY);
                
                    GTimerManager->Add(timer);
                }
            }
        }

        if( false )
        {
            // Thread time line
            std::map< long long, ContextSwitch > & csMap = m_ContextSwitchesMap[a_CS.m_ThreadId];

            if( csMap.rbegin() != csMap.rend() )
            {
                ContextSwitch & lastCS = csMap.rbegin()->second;
                if( lastCS.m_Type == ContextSwitch::In )
                {
                    Timer timer;
                    timer.m_Start = lastCS.m_Time;
                    timer.m_End = a_CS.m_Time;
                    timer.m_TID = a_CS.m_ThreadId;
                    timer.m_SessionID = Message::GSessionID;
                    timer.SetType(Timer::THREAD_ACTIVITY);

                    GTimerManager->Add( timer );
                }
            }
        }
    }

    // TODO: if events are already sorted by timestamp, then
    //       we don't need to use maps. To investigate...
    m_ContextSwitchesMap[a_CS.m_ThreadId].insert( pair );
    m_CoreUtilizationMap[a_CS.m_ProcessorIndex].insert( pair );
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdateMaxTimeStamp( TickType a_Time )
{
    if( a_Time > m_SessionMaxCounter ) 
    {
        m_SessionMaxCounter = a_Time;
    }
};

//-----------------------------------------------------------------------------
void TimeGraph::UpdateThreadDepth( int a_ThreadId, int a_Depth )
{
    if( a_Depth > m_ThreadDepths[a_ThreadId] )
    {
        m_ThreadDepths[a_ThreadId] = a_Depth;
    }
}

//-----------------------------------------------------------------------------
int TimeGraph::GetThreadDepth( int a_ThreadId ) const
{
    auto it = m_ThreadDepths.find(a_ThreadId);
    return it != m_ThreadDepths.end() ? it->second : 0;
}

//-----------------------------------------------------------------------------
int TimeGraph::GetThreadDepthTotal() const
{
    int depth = 0;
    for( auto it = m_ThreadDepths.begin(); it != m_ThreadDepths.end(); ++it )
    {
        depth += it->second;
    }
    return depth;
}

//-----------------------------------------------------------------------------
float TimeGraph::GetThreadTotalHeight()
{
    return m_Layout.GetTotalHeight();
}

//-----------------------------------------------------------------------------
Color TimeGraph::GetThreadColor( int a_ThreadId )
{
    static unsigned char a = 255;
    static std::vector<Color> s_ThreadColors
    {
        Color( 231, 68, 53, a  ),  //red
        Color( 43, 145, 175, a ),  //blue
        Color( 185, 117, 181, a),  //purple
        Color( 87, 166, 74, a  ),  //green
        Color( 215, 171, 105, a),  //beige
        Color( 248, 101, 22, a )   //orange
    };

    return s_ThreadColors[a_ThreadId%s_ThreadColors.size()];
}

//-----------------------------------------------------------------------------
float TimeGraph::GetWorldFromRawTimeStamp( TickType a_Time ) const
{
    if( m_TimeWindowUs > 0 )
    {
        double start = MicroSecondsFromTicks(m_SessionMinCounter, a_Time) - m_MinEpochTimeUs;
        double normalizedStart = start / m_TimeWindowUs;
        float pos = float( m_WorldStartX + normalizedStart*m_WorldWidth );
        return pos;
    }

    return 0;
}

//-----------------------------------------------------------------------------
float TimeGraph::GetWorldFromUs(double a_Micros) const
{
    return GetWorldFromRawTimeStamp( GetRawTimeStampFromUs( a_Micros ) );
}

//-----------------------------------------------------------------------------
TickType TimeGraph::GetRawTimeStampFromWorld( float a_WorldX )
{
    double ratio = m_WorldWidth ? (double)( ( a_WorldX - m_WorldStartX ) / m_WorldWidth ) : 0;
    double timeStamp = GetTime( ratio );
    
    return m_SessionMinCounter + TicksFromMicroseconds( timeStamp );
}

//-----------------------------------------------------------------------------
TickType TimeGraph::GetRawTimeStampFromUs( double a_MicroSeconds ) const
{
    return m_SessionMinCounter + TicksFromMicroseconds( a_MicroSeconds );
}

//-----------------------------------------------------------------------------
void TimeGraph::GetWorldMinMax( float & a_Min, float & a_Max ) const
{
    a_Min = GetWorldFromRawTimeStamp( m_SessionMinCounter );
    a_Max = GetWorldFromRawTimeStamp( m_SessionMaxCounter );
}

//-----------------------------------------------------------------------------
void TimeGraph::Select( const Vec2 & a_WorldStart, const Vec2 a_WorldStop )
{
    float x0 = std::min( a_WorldStart[0], a_WorldStop[0] );
    float x1 = std::max( a_WorldStart[0], a_WorldStop[0] );

    TickType t0 = GetRawTimeStampFromWorld( x0 );
    TickType t1 = GetRawTimeStampFromWorld( x1 );

    m_SelectedCallstackEvents = GEventTracer.GetEventBuffer().GetCallstackEvents( t0, t1 );

    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::NeedsUpdate()
{ 
    m_NeedsUpdatePrimitives = true;
}

//-----------------------------------------------------------------------------
inline std::string GetExtraInfo( const Timer & a_Timer )
{
    std::string info;
    if( !Capture::IsCapturing() && a_Timer.GetType() == Timer::UNREAL_OBJECT )
    {
        info = "[" + ws2s( GOrbitUnreal.GetObjectNames()[a_Timer.m_UserData[0]] ) + "]";
    }
    return info;
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdatePrimitives( bool a_Picking )
{
    m_Batcher.Reset();
    m_VisibleTextBoxes.clear();
    m_TextRendererStatic.Clear();
    m_TextRendererStatic.Init();

    UpdateMaxTimeStamp( GEventTracer.GetEventBuffer().GetMaxTime() );

    int numTextBoxes = m_TextBoxes.size();
    m_SceneBox = m_Canvas->GetSceneBox();
    float minX = m_SceneBox.GetPosX();
    m_NumDrawnTextBoxes = 0;

    m_TimeWindowUs = m_MaxEpochTimeUs - m_MinEpochTimeUs;
    m_WorldStartX = m_Canvas->GetWorldTopLeftX();
    m_WorldWidth = m_Canvas->GetWorldWidth();
    double invTimeWindow = 1.0 / m_TimeWindowUs;

    UpdateThreadIds();

    {
        ScopeLock lock(m_Mutex);
        m_Layout.m_ThreadDepths = m_ThreadDepths;
    }

    m_Layout.CalculateOffsets();

    TickType rawStart = GetRawTimeStampFromUs( m_MinEpochTimeUs );
    TickType rawStop  = GetRawTimeStampFromUs( m_MaxEpochTimeUs );

    unsigned int TextBoxID = 0;
    for( TextBox & textBox : m_TextBoxes )
    {
        const Timer & timer = textBox.GetTimer();

        if( !( rawStart > timer.m_End || rawStop < timer.m_Start ) )
        {          
            double start = MicroSecondsFromTicks( m_SessionMinCounter, timer.m_Start ) - m_MinEpochTimeUs;
            double end = MicroSecondsFromTicks( m_SessionMinCounter, timer.m_End ) - m_MinEpochTimeUs;
            double elapsed = end - start;

            double NormalizedStart  = start   * invTimeWindow;
            double NormalizedEnd    = end     * invTimeWindow;
            double NormalizedLength = elapsed * invTimeWindow;

            bool isCore = timer.IsType( Timer::CORE_ACTIVITY );

            float threadOffset = !isCore ? m_Layout.GetThreadOffset( timer.m_TID, timer.m_Depth )
                : m_Layout.GetCoreOffset( timer.m_Processor );

            float boxHeight = !isCore ? m_Layout.m_TextBoxHeight : m_Layout.m_CoresHeight;

            float WorldTimerStartX = float( m_WorldStartX + NormalizedStart*m_WorldWidth );
            float WorldTimerWidth = float( NormalizedLength * m_WorldWidth );

            Vec2 pos( WorldTimerStartX, threadOffset );
            Vec2 size( WorldTimerWidth, boxHeight );

            textBox.SetPos( pos );
            textBox.SetSize( size );

            if( !timer.IsType( Timer::CORE_ACTIVITY ) )
            {
                UpdateThreadDepth( timer.m_TID, timer.m_Depth + 1 );
            }

            bool isContextSwitch = timer.IsType( Timer::THREAD_ACTIVITY );
            bool isCoreActivity  = timer.IsType( Timer::CORE_ACTIVITY );
            bool isVisibleWidth = NormalizedLength * m_Canvas->getWidth() > 1;
            bool isMainFrameFunction = Capture::GMainFrameFunction && ( Capture::GMainFrameFunction == timer.m_FunctionAddress );
            bool isSameThreadIdAsSelected = isCoreActivity && timer.m_TID == Capture::GSelectedThreadId;
            bool isInactive = ( !isContextSwitch && timer.m_FunctionAddress && ( Capture::GVisibleFunctionsMap[timer.m_FunctionAddress] == nullptr ) ) ||
                              ( Capture::GSelectedThreadId != 0 && isCoreActivity && !isSameThreadIdAsSelected );
            bool isSelected = &textBox == Capture::GSelectedTextBox;


            const unsigned char g = 100;
            Color grey( g, g, g, 255 );
            static Color selectionColor( 0, 128, 255, 255 );
            Color col = m_Layout.GetThreadColor(timer.m_TID);
            col = isSelected ? selectionColor : isSameThreadIdAsSelected ? col : isInactive ? grey : col;
            textBox.SetColor( col[0], col[1], col[2] );
            static int oddAlpha = 210;
            if( !( timer.m_Depth & 0x1 ) )
            {
                col[3] = oddAlpha;
            }

            float z = isInactive ? GlCanvas::Z_VALUE_BOX_INACTIVE : GlCanvas::Z_VALUE_BOX_ACTIVE;

            if( isVisibleWidth )
            {
                Box box;
                box.m_Vertices[0] = Vec3( pos[0]          , pos[1]          , z );
                box.m_Vertices[1] = Vec3( pos[0]          , pos[1] + size[1], z );
                box.m_Vertices[2] = Vec3( pos[0] + size[0], pos[1] + size[1], z );
                box.m_Vertices[3] = Vec3( pos[0] + size[0], pos[1]          , z );
                Color colors[4];
                Fill( colors, col );

                static float coeff = 0.94f;
                Vec3 dark = Vec3( col[0], col[1], col[2] ) * coeff;
                colors[1] = Color( (unsigned char)dark[0], (unsigned char)dark[1], (unsigned char)dark[2], (unsigned char)col[3] );
                colors[0] = colors[1];
                m_Batcher.AddBox( box, colors, PickingID::BOX, &textBox );

                if( !isContextSwitch && textBox.GetText().size() == 0 )
                {
                    double elapsedMillis = ( (double)elapsed ) * 0.001;
                    std::string time = GetPrettyTime( elapsedMillis );
                    Function* func = Capture::GSelectedFunctionsMap[timer.m_FunctionAddress];

                    const char* name = nullptr;
                    if( func )
                    {
                        std::string extraInfo = GetExtraInfo( timer );
                        name = func->PrettyNameStr().c_str();
                        std::string text = Format( "%s %s %s", name, extraInfo.c_str(), time.c_str() );

                        textBox.SetText( text );
                    }
                    else if( !Capture::IsCapturing() )
                    {
                        // GZoneNames is populated when capturing, prevent race
                        // by accessing it only when not capturing.
                        auto it = Capture::GZoneNames.find( timer.m_FunctionAddress );
                        if( it != Capture::GZoneNames.end() )
                        {
                            name = it->second.c_str();
                            std::string text = Format( "%s %s", name, time.c_str() );
                            textBox.SetText( text );
                        }
                    }
                }

                if( !isCoreActivity )
                {
                    //m_VisibleTextBoxes.push_back(&textBox);
                    float minX = m_SceneBox.GetPosX();
                    static Color s_Color( 255, 255, 255, 255 );

                    const Vec2 & pos  = textBox.GetPos();
                    const Vec2 & size = textBox.GetSize();
                    float posX = std::max( pos[0], minX );
                    float maxSize = pos[0] + size[0] - posX;
                    m_TextRendererStatic.AddText( textBox.GetText().c_str()
                                                , posX
                                                , textBox.GetPosY() + 1.f
                                                , GlCanvas::Z_VALUE_TEXT
                                                , s_Color
                                                , maxSize );
                }
            }
            else
            {
                Line line;
                line.m_Beg = Vec3( pos[0], pos[1]          , z );
                line.m_End = Vec3( pos[0], pos[1] + size[1], z );
                Color colors[2];
                Fill( colors, col );
                m_Batcher.AddLine( line, colors, PickingID::LINE, &textBox );
            }

            if( ++m_NumDrawnTextBoxes > numTextBoxes )
            {
                break;
            }
        }

        ++TextBoxID;
    }

    if( !a_Picking )
    {
        UpdateEvents();
    }

    m_NeedsUpdatePrimitives = false;
    m_NeedsRedraw = true;
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdateEvents()
{
    float x0 = GetWorldFromRawTimeStamp( m_SessionMinCounter );
    float x1 = GetWorldFromRawTimeStamp( m_SessionMaxCounter );
    float sizeX = x1 - x0;

    TickType rawMin = GetRawTimeStampFromUs( m_MinEpochTimeUs );
    TickType rawMax = GetRawTimeStampFromUs( m_MaxEpochTimeUs );

    ScopeLock lock( GEventTracer.GetEventBuffer().GetMutex() );

    Color lineColor[2];
    Color white(255, 255, 255, 255);
    Fill( lineColor, white );

    for( auto & pair : GEventTracer.GetEventBuffer().GetCallstacks() )
    {
        ThreadID threadID = pair.first;
        std::map< long long, CallstackEvent > & callstacks = pair.second;

        // Sampling Events
        float ThreadOffset = (float)m_Layout.GetSamplingTrackOffset( threadID );
        for( auto & callstackPair : callstacks )
        {
            unsigned long long time = callstackPair.first;

            if( time > rawMin && time < rawMax )
            {
                float x = GetWorldFromRawTimeStamp( time );
                Line line;
                line.m_Beg = Vec3( x, ThreadOffset, GlCanvas::Z_VALUE_EVENT );
                line.m_End = Vec3( x, ThreadOffset - m_Layout.m_EventTrackHeight, GlCanvas::Z_VALUE_EVENT );
                m_Batcher.AddLine( line, lineColor, PickingID::EVENT );
            }
        }
    }

    // Draw selected events
    Color selectedColor[2];
    Color col( 0, 255, 0, 255 );
    Fill( selectedColor, col );
    for( CallstackEvent & event : m_SelectedCallstackEvents )
    {
        float x = GetWorldFromRawTimeStamp( event.m_Time );
        float ThreadOffset = (float)m_Layout.GetSamplingTrackOffset( event.m_TID );

        Line line;
        line.m_Beg = Vec3( x, ThreadOffset, GlCanvas::Z_VALUE_EVENT );
        line.m_End = Vec3( x, ThreadOffset - m_Layout.m_EventTrackHeight, GlCanvas::Z_VALUE_TEXT );
        m_Batcher.AddLine( line, selectedColor, PickingID::EVENT );
    }
}

//-----------------------------------------------------------------------------
void TimeGraph::SelectEvents( float a_WorldStart, float a_WorldEnd, ThreadID a_TID )
{
    if( a_WorldStart > a_WorldEnd )
    {
        std::swap( a_WorldEnd, a_WorldStart );
    }

    TickType t0 = GetRawTimeStampFromWorld( a_WorldStart );
    TickType t1 = GetRawTimeStampFromWorld( a_WorldEnd );

    m_SelectedCallstackEvents = GEventTracer.GetEventBuffer().GetCallstackEvents( t0, t1, a_TID );

    // Generate report
    std::shared_ptr<SamplingProfiler> samplingProfiler = std::make_shared<SamplingProfiler>( Capture::GTargetProcess );
    samplingProfiler->SetState( SamplingProfiler::Sampling );

    samplingProfiler->SetGenerateSummary(a_TID==-1);

    for( CallstackEvent & event : m_SelectedCallstackEvents )
    {
        const std::shared_ptr<CallStack> callstack = Capture::GSamplingProfiler->GetCallStack(event.m_Id);
        if( callstack )
        {
            callstack->m_ThreadId = event.m_TID;
            samplingProfiler->AddCallStack( *callstack );
        }
    }
    samplingProfiler->ProcessSamples();

    if( samplingProfiler->GetNumSamples() > 0 )
    {
        GOrbitApp->AddSelectionReport(samplingProfiler);
    }

    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void TimeGraph::Draw( bool a_Picking )
{
    if( m_TextBoxes.keep( GParams.m_MaxNumTimers ) || (!a_Picking && m_NeedsUpdatePrimitives) || a_Picking )
    {
        UpdatePrimitives( a_Picking );
    }

    DrawBuffered( a_Picking );
    DrawEvents( a_Picking );

    m_NeedsRedraw = false;
}

//-----------------------------------------------------------------------------
void TimeGraph::UpdateThreadIds()
{
    {
        ScopeLock lock( GEventTracer.GetEventBuffer().GetMutex() );
        m_EventCount.clear();

        for( auto & pair : GEventTracer.GetEventBuffer().GetCallstacks() )
        {
            ThreadID threadID = pair.first;
            std::map< long long, CallstackEvent > & callstacks = pair.second;

            m_EventCount[threadID] = (uint32_t)callstacks.size();

            if( m_ThreadDepths.find( threadID ) == m_ThreadDepths.end() )
            {
                m_ThreadDepths[threadID] = 0;
            }
        }
    }

    m_Layout.ClearThreadColors();

    // Reorder threads once every second when capturing
    if( !Capture::IsCapturing() || m_LastThreadReorder.QueryMillis() > 1000.0 )
    {
        m_Layout.m_SortedThreadIds.clear();

        // Show threads with instrumented functions first
        std::vector< std::pair< ThreadID, uint32_t > > sortedThreads = OrbitUtils::ReverseValueSort( m_ThreadCountMap );
        for( auto & pair : sortedThreads )
        {
            m_Layout.m_SortedThreadIds.push_back( pair.first );
        }

        // Then show threads sorted by number of events
        std::vector< std::pair< ThreadID, uint32_t > > sortedByEvents = OrbitUtils::ReverseValueSort( m_EventCount );
        for( auto & pair : sortedByEvents )
        {
            if( m_ThreadCountMap.find( pair.first ) == m_ThreadCountMap.end() )
            {
                m_Layout.m_SortedThreadIds.push_back( pair.first );
            }
        }
     
        m_LastThreadReorder.Reset();
    }

    for( uint32_t i = 0; i < m_Layout.m_SortedThreadIds.size(); ++i )
    {
        ThreadID threadId = m_Layout.m_SortedThreadIds[i];
        m_Layout.SetThreadColor( threadId, GetThreadColor( i ) );

        if( m_EventTracks.find( threadId ) == m_EventTracks.end() )
        {
            EventTrack* eventTrack = new EventTrack( this );
            eventTrack->SetThreadId( threadId );
            m_EventTracks[threadId] = eventTrack;
        }
    }
}

//-----------------------------------------------------------------------------
void TimeGraph::DrawIdentifiers()
{
    float x0 = GetWorldFromRawTimeStamp( m_SessionMinCounter );
    int xPos, yPos;
    m_Canvas->WorldToScreen( x0, 0, xPos, yPos );
    xPos = std::max( 0, xPos );
    int i = 0;
    

    // Draw threads
    for( uint32_t i = 0; i < m_Layout.m_SortedThreadIds.size(); ++i )
    {
        ThreadID threadId = m_Layout.m_SortedThreadIds[i];
        float threadOffset = m_Layout.GetThreadBlockStart( threadId );
        int x, y;
        m_Canvas->WorldToScreen( 0, threadOffset-m_Layout.m_EventTrackHeight, x, y );
        std::string threadName = ws2s(Capture::GTargetProcess->GetThreadNameFromTID(threadId));
        threadName = Format( "%s [%u]", threadName.c_str(), threadId );
        m_TextRenderer->AddText2D( threadName.c_str(), xPos, y, GlCanvas::Z_VALUE_TEXT, Color( 255, 255, 255, 255 ) );

        int trackHeightScreen = m_Canvas->WorldToScreenHeight(m_Layout.m_EventTrackHeight);

        int strWidth, strHeight;
        m_TextRenderer->GetStringSize( threadName.c_str(), strWidth, strHeight );

        y = m_Canvas->getHeight() - y;

        Color col = GetThreadColor( i+1 );
        col[3] = m_TrackAlpha;
        glColor4ubv( &col[0] );
        glBegin( GL_QUADS );

        static float zValue = -0.01f;

        glVertex3f( float(xPos), float(y), zValue );
        glVertex3f( float(xPos+strWidth), float(y), zValue );
        glVertex3f( float(xPos+strWidth), float(y + trackHeightScreen), zValue );
        glVertex3f( float(xPos), float(y + trackHeightScreen), zValue );
        glEnd();
    }
}

//----------------------------------------------------------------------------
void TimeGraph::DrawText()
{
    if( m_DrawText )
    {
        m_TextRendererStatic.Display();
    }
}

//----------------------------------------------------------------------------
void TimeGraph::DrawBuffered( bool a_Picking )
{
    glPushAttrib( GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_CULL_FACE );
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );
    glEnable( GL_TEXTURE_2D );

    DrawBoxBuffer( a_Picking );
    DrawLineBuffer( a_Picking );

    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );
    glPopAttrib();
}

//----------------------------------------------------------------------------
void TimeGraph::DrawBoxBuffer( bool a_Picking )
{
    Block<Box,   BoxBuffer::NUM_BOXES_PER_BLOCK>*   boxBlock = m_Batcher.GetBoxBuffer().m_Boxes.m_Root;
    Block<Color, BoxBuffer::NUM_BOXES_PER_BLOCK*4>* colorBlock;

    colorBlock = !a_Picking ? m_Batcher.GetBoxBuffer().m_Colors.m_Root
                            : m_Batcher.GetBoxBuffer().m_PickingColors.m_Root;

    while( boxBlock )
    {
        if( int numElems = boxBlock->m_Size )
        {
            glVertexPointer( 3, GL_FLOAT, sizeof(Vec3), boxBlock->m_Data );
            glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( Color ), (void*)(colorBlock->m_Data ) );
            glDrawArrays( GL_QUADS, 0, numElems*4 );
        }

        boxBlock = boxBlock->m_Next;
        colorBlock = colorBlock->m_Next;
    }   
}

//----------------------------------------------------------------------------
void TimeGraph::DrawLineBuffer( bool a_Picking )
{
    Block<Line,  LineBuffer::NUM_LINES_PER_BLOCK>*   lineBlock  = m_Batcher.GetLineBuffer().m_Lines.m_Root;
    Block<Color, LineBuffer::NUM_LINES_PER_BLOCK*2>* colorBlock;
    
    colorBlock = !a_Picking ? m_Batcher.GetLineBuffer().m_Colors.m_Root
                            : m_Batcher.GetLineBuffer().m_PickingColors.m_Root;
    
    while( lineBlock )
    {
        if( int numElems = lineBlock->m_Size )
        {
            glVertexPointer( 3, GL_FLOAT, sizeof( Vec3 ), lineBlock->m_Data );
            glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( Color ), (void*)( colorBlock->m_Data ) );
            glDrawArrays( GL_LINES, 0, numElems * 2 );
        }

        lineBlock = lineBlock->m_Next;
        colorBlock = colorBlock->m_Next;
    }
}

//-----------------------------------------------------------------------------
void TimeGraph::DrawEvents( bool a_Picking )
{
    double timeWindow = m_MaxEpochTimeUs - m_MinEpochTimeUs;
    float WorldStartX = m_Canvas->GetWorldTopLeftX();
    float WorldWidth = m_Canvas->GetWorldWidth();

    TickType rawMin = GetRawTimeStampFromUs( m_MinEpochTimeUs );
    TickType rawMax = GetRawTimeStampFromUs( m_MaxEpochTimeUs );

    // Draw track background
    float x0 = GetWorldFromRawTimeStamp( m_SessionMinCounter );
    float x1 = GetWorldFromRawTimeStamp( m_SessionMaxCounter );
    float sizeX = x1 - x0;

    for( uint32_t i = 0; i < m_Layout.m_SortedThreadIds.size(); ++i )
    {
        ThreadID threadId = m_Layout.m_SortedThreadIds[i];
        float y0 = m_Layout.GetSamplingTrackOffset( threadId );

        if( y0 == -1.f )
            continue;

        float y1 = y0 - m_Layout.m_EventTrackHeight;
        EventTrack* eventTrack = m_EventTracks[threadId];
        
        eventTrack->SetPos( x0, y0 );
        eventTrack->SetSize( sizeX,  m_Layout.m_EventTrackHeight );
        eventTrack->Draw( m_Canvas, a_Picking );
    }
}

//-----------------------------------------------------------------------------
void TimeGraph::DrawMainFrame( TextBox & a_Box )
{
    if( a_Box.GetMainFrameCounter() == -1 )
    {
        a_Box.SetMainFrameCounter( ++m_MainFrameCounter );
    }

    static unsigned char grey = 180;
    static Color frameColor( grey, grey, grey, 10 );
    
    if( a_Box.GetMainFrameCounter() % 2 == 0 )
    {
        float minX = m_SceneBox.GetPosX();
        TextBox frameBox;

        frameBox.SetPosX( a_Box.GetPosX() );
        frameBox.SetPosY( m_SceneBox.GetPosY() );
        frameBox.SetSizeX( a_Box.GetSize()[0] );
        frameBox.SetSizeY( m_SceneBox.GetSize()[0] );
        frameBox.SetColor( frameColor );
        frameBox.Draw( *m_TextRenderer, minX, true, false, false );
    }
}

//-----------------------------------------------------------------------------
bool TimeGraph::IsVisible( const Timer & a_Timer )
{
    double start = MicroSecondsFromTicks(m_SessionMinCounter, a_Timer.m_Start);
    double end   = MicroSecondsFromTicks(m_SessionMinCounter, a_Timer.m_End);

    if( m_MinEpochTimeUs > end || m_MaxEpochTimeUs < start )
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
void TimeGraph::GetBounds(Vec2 & a_Min, Vec2 & a_Max)
{
    if( m_TextBoxes.size() > 0 )
    {
        TextBox dummyBox /*= m_TextBoxes[0]*/;
        for( TextBox & box : m_TextBoxes )
        {
            dummyBox.Expand( box );
        }

        a_Min = dummyBox.GetMin();
        a_Max = dummyBox.GetMax();
    }
}
