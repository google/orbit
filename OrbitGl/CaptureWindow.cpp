//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CaptureWindow.h"
#include "Capture.h"
#include "GlUtils.h"
#include "App.h"
#include "SymbolUtils.h"
#include "TcpServer.h"
#include "TimerManager.h"
#include "PluginManager.h"
#include "../OrbitPlugin/OrbitSDK.h"

//-----------------------------------------------------------------------------
CaptureWindow::CaptureWindow()
{
    GCurrentTimeGraph = &m_TimeGraph;
    m_TimeGraph.m_TextRenderer = &m_TextRenderer;
    m_TimeGraph.m_PickingManager = &m_PickingManager;
    m_TimeGraph.SetCanvas(this);
    m_DrawUI = false;
    m_DrawHelp = false;
    m_DrawMemTracker = false;
    m_FirstHelpDraw = true;
    m_DrawStats = false;
    m_Picking = false;

    m_DesiredWorldWidth  = (float)m_Width;
    m_DesiredWorldHeight = (float)m_Height;
    m_WorldTopLeftX = 0;
    m_WorldTopLeftY = 0;
    m_WorldMaxY = 0;
    m_ProcessX = 0;

    GTimerManager->m_TimerAddedCallbacks.push_back( [=]( Timer & a_Timer ){ this->OnTimerAdded( a_Timer ); } );
    GTimerManager->m_ContextSwitchAddedCallback = [=]( const ContextSwitch & a_CS ){ this->OnContextSwitchAdded( a_CS ); };

    m_HoverDelayMs = 300;
    m_CanHover = false;
    m_IsHovering = false;
    ResetHoverTimer();

    m_Slider.SetCanvas(this);
    m_Slider.SetDragCallback( [&]( float a_Ratio ){ this->OnDrag( a_Ratio ); } );

    GOrbitApp->RegisterCaptureWindow(this);
}

//-----------------------------------------------------------------------------
CaptureWindow::~CaptureWindow()
{
    if( GCurrentTimeGraph == &m_TimeGraph )
        GCurrentTimeGraph = nullptr;
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnTimer()
{
    GlCanvas::OnTimer();
}

//-----------------------------------------------------------------------------
void CaptureWindow::ZoomAll()
{
    m_TimeGraph.ZoomAll();
    m_DesiredWorldHeight = m_TimeGraph.GetThreadTotalHeight();

    float margin = (float)m_TimeGraph.GetMarginInPixels();
    m_WorldTopLeftY = m_WorldMaxY;
    ResetHoverTimer();
    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::UpdateWheelMomentum(float a_DeltaTime)
{
    GlCanvas::UpdateWheelMomentum( a_DeltaTime );

    bool zoomWidth = true;// TODO: !wxGetKeyState(WXK_CONTROL);
    if (zoomWidth && m_WheelMomentum != 0.f)
    {
        m_TimeGraph.ZoomTime(m_WheelMomentum, m_MouseRatio);
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::MouseMoved( int a_X, int a_Y, bool a_Left, bool a_Right, bool a_Middle )
{
    int mousex = a_X;
    int mousey = a_Y;

    float worldx, worldy;
    ScreenToWorld(a_X, a_Y, worldx, worldy);

    m_MouseX = worldx;
    m_MouseY = worldy;
    m_MousePosX = mousex;
    m_MousePosY = mousey;

    // Pan
    if ( a_Left && !m_ImguiActive && !m_PickingManager.IsDragging() && !Capture::IsCapturing() )
    {
        float worldMin;
        float worldMax;

        m_TimeGraph.GetWorldMinMax( worldMin, worldMax );

        m_WorldTopLeftX = m_WorldClickX - (float)mousex / (float)getWidth() * m_WorldWidth;
        m_WorldTopLeftY = m_WorldClickY + (float)mousey / (float)getHeight() * m_WorldHeight;

        m_WorldTopLeftX = clamp( m_WorldTopLeftX, worldMin, worldMax-m_WorldWidth );
        m_WorldTopLeftY = clamp( m_WorldTopLeftY, -FLT_MAX, m_WorldMaxY );
        UpdateSceneBox();

        m_TimeGraph.PanTime(m_ScreenClickX, a_X, getWidth(), (double)m_RefTimeClick);

        NeedsUpdate();
    }

    if ( m_IsSelecting  )
    {
        m_SelectStop = Vec2(worldx, worldy);
        m_TimeStop   = m_TimeGraph.GetRawTimeStampFromWorld( worldx );
    }

    if( a_Left )
    {
        m_PickingManager.Drag( a_X, a_Y );
    }

    ResetHoverTimer();

    NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::LeftDown( int a_X, int a_Y )
{
    // Store world clicked pos for panning
    ScreenToWorld(a_X, a_Y, m_WorldClickX, m_WorldClickY);
    m_ScreenClickX = a_X;
    m_ScreenClickY = a_Y;
    m_RefTimeClick = (TickType)m_TimeGraph.GetTime((double)a_X / (double)getWidth());

    m_IsSelecting = false;

    Orbit_ImGui_MouseButtonCallback(this, 0, true);

    m_Picking = true;
    NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::LeftUp()
{
    GlCanvas::LeftUp();
    NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::LeftDoubleClick()
{
    GlCanvas::LeftDoubleClick();
    m_DoubleClicking = true;
    m_Picking = true;
}

//-----------------------------------------------------------------------------
void CaptureWindow::Pick()
{
    m_Picking = true;
    NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::Pick( int a_X, int a_Y )
{
    // 4 bytes per pixel (RGBA), 1x1 bitmap
    std::vector< unsigned char > pixels(1 * 1 * 4);
    glReadPixels(a_X, m_MainWindowHeight - a_Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    
    PickingID pickId = PickingID::Get( *((uint32_t*)(&pixels[0])) );

    Capture::GSelectedTextBox = nullptr;
    Capture::GSelectedThreadId = 0;

    Pick( pickId, a_X, a_Y );

    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::Pick( PickingID a_PickingID, int a_X, int a_Y  )
{
    uint32_t type = a_PickingID.m_Type;
    uint32_t id = a_PickingID.m_Id;

    switch( type )
    {
    case PickingID::BOX:
    {
        void** textBoxPtr = m_TimeGraph.m_Batcher.GetBoxBuffer().m_UserData.SlowAt( id );
        if( textBoxPtr )
        {
            TextBox* textBox = (TextBox*)*textBoxPtr;
            SelectTextBox( textBox );
        }
        break;
    }
    case PickingID::LINE:
    {
        void** textBoxPtr = m_TimeGraph.m_Batcher.GetLineBuffer().m_UserData.SlowAt( id );
        if( textBoxPtr )
        {
            TextBox* textBox = (TextBox*)*textBoxPtr;
            SelectTextBox( textBox );
        }
        break;
    }
    case PickingID::PICKABLE:
        m_PickingManager.Pick( a_PickingID.m_Id, a_X, a_Y );
        break;
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::SelectTextBox( class TextBox* a_TextBox )
{
    Capture::GSelectedTextBox = a_TextBox;
    Capture::GSelectedThreadId = a_TextBox->GetTimer().m_TID;
    Capture::GSelectedCallstack = Capture::GetCallstack( a_TextBox->GetTimer().m_CallstackHash );
    GOrbitApp->SetCallStack( Capture::GSelectedCallstack );

    const Timer & a_Timer = a_TextBox->GetTimer();
    DWORD64 address = a_Timer.m_FunctionAddress;
    if( a_Timer.IsType( Timer::ZONE ) )
    {
        std::shared_ptr<CallStack> callStack = Capture::GetCallstack( a_Timer.m_CallstackHash );
        if( callStack && callStack->m_Depth > 1 )
        {
            address = callStack->m_Data[1];
        }
    }

    FindCode( address );

    if( m_DoubleClicking && a_TextBox )
    {
        m_TimeGraph.Zoom( a_TextBox );
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::Hover( int a_X, int a_Y )
{
    // 4 bytes per pixel (RGBA), 1x1 bitmap
    std::vector< unsigned char > pixels( 1 * 1 * 4 );
    glReadPixels( a_X, m_MainWindowHeight - a_Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0] );

    PickingID pickId = *((PickingID*)(&pixels[0]));

    TextBox* textBox = m_TimeGraph.m_Batcher.GetTextBox( pickId );
    if( textBox )
    {
        if( !textBox->GetTimer().IsType(Timer::CORE_ACTIVITY) )
        {
            Function* func = Capture::GSelectedFunctionsMap[textBox->GetTimer().m_FunctionAddress];
            m_ToolTip = Format( L"%s %s", func ? func->PrettyName().c_str() : L"", s2ws(textBox->GetText()).c_str() );
            GOrbitApp->SendToUiAsync( L"tooltip:" + m_ToolTip );
            NeedsRedraw();
        }
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::FindCode( DWORD64 address )
{
    SCOPE_TIMER_LOG( L"FindCode" );

    LineInfo lineInfo;
    
    if( SymUtils::GetLineInfo( address, lineInfo ) || ( Capture::GSamplingProfiler && Capture::GSamplingProfiler->GetLineInfo( address, lineInfo ) ) )
    {
        --lineInfo.m_Line;

        //File mapping
        const std::map< std::wstring, std::wstring > & fileMap = GOrbitApp->GetFileMapping();
        for( const auto & pair : fileMap )
        {
            ReplaceStringInPlace( lineInfo.m_File, pair.first, pair.second );
        }

        if( lineInfo.m_Address != 0 )
        {
            GOrbitApp->SendToUiAsync( Format(L"code^%s^%i", lineInfo.m_File.c_str(), lineInfo.m_Line ) );
        }
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::PreRender()
{
    if( m_CanHover && m_HoverTimer.QueryMillis() > m_HoverDelayMs )
    {
        m_IsHovering = true;
        m_Picking = true;
        NeedsRedraw();
    }

    m_NeedsRedraw = m_NeedsRedraw || m_TimeGraph.m_NeedsRedraw;
}

//-----------------------------------------------------------------------------
void CaptureWindow::PostRender()
{
    if( m_IsHovering )
    {
        m_IsHovering = false;
        m_CanHover = false;
        m_Picking = false;
        m_HoverTimer.Reset();

        Hover( m_MousePosX, m_MousePosY );
        NeedsUpdate();
        GlCanvas::Render( m_Width, m_Height );
        m_HoverTimer.Reset();
    }

    if( m_Picking )
    {
        m_Picking = false;
        Pick( m_ScreenClickX, m_ScreenClickY );
        NeedsRedraw();
        GlCanvas::Render( m_Width, m_Height );
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::Resize( int a_Width, int a_Height )
{
    m_Width = a_Width;
    m_Height = a_Height;
    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::RightDown( int a_X, int a_Y )
{
    ScreenToWorld( a_X, a_Y, m_WorldClickX, m_WorldClickY );
    m_ScreenClickX = a_X;
    m_ScreenClickY = a_Y;
    Pick();

    m_IsSelecting = true;
    m_SelectStart = Vec2( m_WorldClickX, m_WorldClickY );
    m_SelectStop  = m_SelectStart;
    m_TimeStart   = m_TimeGraph.GetRawTimeStampFromWorld( m_WorldClickX );
    m_TimeStop    = m_TimeStart;
}

//-----------------------------------------------------------------------------
bool CaptureWindow::RightUp()
{
    if( m_IsSelecting && ( m_SelectStart[0] != m_SelectStop[0] ) && ControlPressed() )
    {
        float minWorld = std::min(m_SelectStop[0], m_SelectStart[0]);
        float maxWorld = std::max(m_SelectStop[0], m_SelectStart[0]);

        double newMin = m_TimeGraph.GetTime( (minWorld-m_WorldTopLeftX)/m_WorldWidth );
        double newMax = m_TimeGraph.GetTime( (maxWorld-m_WorldTopLeftX)/m_WorldWidth );

        m_TimeGraph.SetMinMax( newMin, newMax );
        m_SelectStart = m_SelectStop;
    }

    bool showContextMenu = m_SelectStart[0] == m_SelectStop[0];
    m_IsSelecting = false;
    NeedsRedraw();
    return showContextMenu;
}

//-----------------------------------------------------------------------------
void CaptureWindow::MiddleDown( int a_X, int a_Y )
{
    float worldx, worldy;
    ScreenToWorld( a_X, a_Y, worldx, worldy );
    m_IsSelecting = true;
    m_SelectStart = Vec2( worldx, worldy );
    m_SelectStop = m_SelectStart;
}

//-----------------------------------------------------------------------------
void CaptureWindow::MiddleUp( int a_X, int a_Y )
{
    float worldx, worldy;
    ScreenToWorld( a_X, a_Y, worldx, worldy );
    m_IsSelecting = false;

    m_SelectStop = Vec2( worldx, worldy );
    //m_TimeGraph.SelectEvents( m_SelectStart[0], m_SelectStop[0], -1 );

    NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::MouseWheelMoved( int a_X, int a_Y, int a_Delta, bool a_Ctrl )
{
	if ( a_Delta == 0 )
		return;

    // Zoom
    int delta = -a_Delta / abs(a_Delta);
    m_WheelDelta = delta;

    if (delta < m_MinWheelDelta)
        m_MinWheelDelta = delta;
    if (delta > m_MaxWheelDelta)
        m_MaxWheelDelta = delta;

    float mousex = (float)a_X;
    float mousey = (float)a_Y;

    float worldx;
    float worldy;

    ScreenToWorld(a_X, a_Y, worldx, worldy);
    m_MouseRatio = (double)mousex / (double)getWidth();

    static float zoomRatio = 0.1f;
    bool zoomWidth = !a_Ctrl;

    if (zoomWidth)
    {
        m_TimeGraph.ZoomTime((float)delta, (double)mousex / (double)getWidth());
        m_WheelMomentum = delta*m_WheelMomentum < 0 ? 0.f : m_WheelMomentum + (float)delta;
    }
    else
    {
        float zoomInc = zoomRatio * m_DesiredWorldHeight;
        m_DesiredWorldHeight += delta * zoomInc;
        m_WorldTopLeftY = worldy + (float)mousey / (float)getHeight()*m_DesiredWorldHeight;
        m_WorldTopLeftY = std::min( m_WorldMaxY, m_WorldTopLeftY );
    }

    Orbit_ImGui_ScrollCallback(this, -delta);

    m_CanHover = true;

    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::KeyPressed( unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt )
{
    UpdateSpecialKeys( a_Ctrl, a_Shift, a_Alt );

    ScopeImguiContext state(m_ImGuiContext);

    if (!m_ImguiActive)
    {
        switch (a_KeyCode)
        {
        case 'A':
            ZoomAll();
            break;
        case 'W':
            m_DrawStats = !m_DrawStats;
            break;
        case 'H':
            m_DrawHelp = !m_DrawHelp;
            break;
        case 'S':
            ToggleSampling();
            break;
        case 'X':
            GOrbitApp->ToggleCapture();
            break;
        case 'T':
            m_TimeGraph.m_DrawText = !m_TimeGraph.m_DrawText;
            break;
        case 'O':
            if( a_Ctrl )
            {
                m_TextRenderer.ToggleDrawOutline();
            }
            break;
        case 'M':
            m_DrawMemTracker = !m_DrawMemTracker;
            break;
        case 'K':
            CrashHandler::SendMiniDump();
            break;
        }
    }

    ImGuiIO& io = ImGui::GetIO();
    io.KeyCtrl  = a_Ctrl;
    io.KeyShift = a_Shift;
    io.KeyAlt   = a_Alt;

    Orbit_ImGui_KeyCallback(this, a_KeyCode, true);

    NeedsRedraw();
}

//-----------------------------------------------------------------------------
std::wstring GOTO_CALLSTACK = L"Go to Callstack";
std::wstring GOTO_SOURCE    = L"Go to Source";

//-----------------------------------------------------------------------------
std::vector<std::wstring> CaptureWindow::GetContextMenu()
{
    static std::vector< std::wstring > menu = { GOTO_CALLSTACK, GOTO_SOURCE };
    static std::vector< std::wstring > emptyMenu;
    TextBox* selection = Capture::GSelectedTextBox;
    return selection && !selection->GetTimer().IsCoreActivity() ? menu : emptyMenu;
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnContextMenu( const std::wstring & a_Action, int a_MenuIndex )
{
    if( Capture::GSelectedTextBox )
    {
        if( a_Action == GOTO_SOURCE )
        {
            GOrbitApp->GoToCode( Capture::GSelectedTextBox->GetTimer().m_FunctionAddress );
        }
        else if( a_Action == GOTO_CALLSTACK )
        {
            GOrbitApp->GoToCallstack();
        }
    }    
}

//-----------------------------------------------------------------------------
void CaptureWindow::ToggleSampling()
{
    if( Capture::GIsSampling )
    {
        Capture::StopSampling();
    }
    else if( !GTimerManager->m_IsRecording )
    {
        Capture::StartSampling();
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnCaptureStarted()
{
    m_DesiredWorldWidth = (float)m_Width;
    m_DesiredWorldHeight = (float)m_Height;
    m_TimeGraph.m_Layout.Reset();
    m_TimeGraph.ZoomAll();
    NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::ResetHoverTimer()
{
    m_HoverTimer.Reset();
    m_CanHover = true;
}

//-----------------------------------------------------------------------------
void CaptureWindow::Draw()
{   
    m_WorldMaxY = 1.5f * ScreenToWorldHeight( (int)m_Slider.GetPixelHeight() );

    if( Capture::IsCapturing() )
    {
        ZoomAll();
    }

    m_TimeGraph.Draw( m_Picking );

    if( m_SelectStart[0] != m_SelectStop[0] )
    {
        TickType minTime = std::min( m_TimeStart, m_TimeStop );
        TickType maxTime = std::max( m_TimeStart, m_TimeStop );

        float from = m_TimeGraph.GetWorldFromRawTimeStamp( minTime );
        float to   = m_TimeGraph.GetWorldFromRawTimeStamp( maxTime );

        double micros = MicroSecondsFromTicks( minTime, maxTime );
        float sizex = to - from;
        Vec2 pos( from, m_WorldTopLeftY - m_WorldHeight );
        Vec2 size( sizex, m_WorldHeight );

        std::string time = GetPrettyTime( micros*0.001 );
        TextBox box( pos, size, time, &m_TextRenderer, Color(0, 128, 0, 128) );
        box.SetTextY( m_SelectStop[1] );
        box.Draw( m_TextRenderer, -FLT_MAX, true, true );
    }

    if( !m_Picking && !m_IsHovering )
    {
        DrawStatus();
        RenderTimeBar();

        // Vertical line
        glColor4f(0, 1, 0, 0.5f);
        glBegin(GL_LINES);
        glVertex3f(m_MouseX, m_WorldTopLeftY, Z_VALUE_TEXT);
        glVertex3f(m_MouseX, m_WorldTopLeftY - m_WorldHeight, Z_VALUE_TEXT);
        glEnd();
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::DrawScreenSpace()
{
    double timeSpan = m_TimeGraph.GetSessionTimeSpanUs();

    Color col = m_Slider.GetBarColor();
    float height = m_Slider.GetPixelHeight();
    float canvasHeight = (float)getHeight();
    float z = GlCanvas::Z_VALUE_TEXT_UI_BG;

    // Top bar
    glColor4ubv( &col[0] );
    glBegin( GL_QUADS );
    glVertex3f( 0, canvasHeight, z );
    glVertex3f( (float)getWidth(), canvasHeight, z );
    glVertex3f( (float)getWidth(), canvasHeight - height, z );
    glVertex3f( 0, canvasHeight - height, z );
    glEnd();

    // Time bar
    if( m_TimeGraph.GetSessionTimeSpanUs() > 0 )
    {
        glColor4ub( 70, 70, 70, 200 );
        glBegin( GL_QUADS );
        glVertex3f( 0, height, z );
        glVertex3f( (float)getWidth(), height, z );
        glVertex3f( (float)getWidth(), 2*height, z );
        glVertex3f( 0, 2*height, z );
        glEnd();
    }

    if( timeSpan > 0 )
    {
        double start = m_TimeGraph.m_MinEpochTimeUs;
        double stop = m_TimeGraph.m_MaxEpochTimeUs;
        double width = stop - start;
        double maxStart = timeSpan - width;
        double ratio = Capture::IsCapturing() ? 1.0 : maxStart ? start/maxStart : 0.0;
        
        m_Slider.SetSliderRatio( (float)ratio );
        m_Slider.SetSliderWidthRatio( float(width/timeSpan) );
        m_Slider.Draw( this, m_Picking );
    }

    m_TimeGraph.DrawIdentifiers();
}   

//-----------------------------------------------------------------------------
void CaptureWindow::OnDrag( float a_Ratio )
{
    double timeSpan = m_TimeGraph.GetSessionTimeSpanUs();
    double timeWindow = m_TimeGraph.m_MaxEpochTimeUs - m_TimeGraph.m_MinEpochTimeUs;
    m_TimeGraph.m_MinEpochTimeUs = a_Ratio*(timeSpan-timeWindow);
    m_TimeGraph.m_MaxEpochTimeUs = m_TimeGraph.m_MinEpochTimeUs + timeWindow;
    NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::NeedsUpdate()
{
    m_TimeGraph.NeedsUpdate();
    m_NeedsRedraw = true;
}

//-----------------------------------------------------------------------------
float CaptureWindow::GetTopBarTextY()
{
    return m_Slider.GetPixelHeight()*0.5f + (float)m_TextRenderer.GetStringHeight("FpjT_H")*0.5f;
}

//-----------------------------------------------------------------------------
void CaptureWindow::DrawStatus()
{
    float iconSize = m_Slider.GetPixelHeight();
    int s_PosX = (int)iconSize;
    float s_PosY = GetTopBarTextY();
    static int s_IncY = 20;

    static Color s_Color( 255, 255, 255, 255 );

    int PosX = getWidth() - s_PosX;
    int PosY = (int)s_PosY;
    int LeftY = (int)s_PosY;

    m_TextRenderer.AddText2D( " press 'H'", s_PosX, LeftY, Z_VALUE_TEXT_UI, s_Color ); LeftY += s_IncY;

    if( Capture::GInjected )
    {
        std::string injectStr = Format( " %s", Capture::GInjectedProcess.c_str() );
        m_ProcessX = m_TextRenderer.AddText2D( injectStr.c_str(), PosX, PosY, Z_VALUE_TEXT_UI, s_Color, -1, true ); PosY += s_IncY;
    }

    if( Capture::GIsTesting )
    {
        m_TextRenderer.AddText2D( "TESTING", PosX, PosY, Z_VALUE_TEXT_UI, s_Color, -1, true ); PosY += s_IncY;
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderUI()
{
    ScopeImguiContext state(m_ImGuiContext);
    Orbit_ImGui_NewFrame(this);

    if( m_DrawStats )
    {
        m_StatsWindow.Clear();

        /*m_StatsWindow.AddLog( VAR_TO_CHAR( m_Width ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_Height ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_DesiredWorldHeight ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_DesiredWorldWidth ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_WorldHeight ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_WorldWidth ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_WorldTopLeftX ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_WorldTopLeftY ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_WorldMinWidth ) );

        m_StatsWindow.AddLog( VAR_TO_CHAR( GTimerManager->m_NumTimersFromPreviousSession ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( GTimerManager->m_NumFlushedTimers ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( Capture::GNumMessagesFromPreviousSession ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( Capture::GNumTargetFlushedEntries ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( Capture::GNumTargetFlushedTcpPackets ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( GTimerManager->m_NumQueuedEntries ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( GTimerManager->m_NumQueuedMessages) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( GTimerManager->m_NumQueuedTimers) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_SelectStart[0] ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_SelectStop[0] ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_TimeGraph.m_CurrentTimeWindow ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( Capture::GMaxTimersAtOnce ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( Capture::GNumTimersAtOnce ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( Capture::GNumTargetQueuedEntries ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_MousePosX ) );
        m_StatsWindow.AddLog( VAR_TO_CHAR( m_MousePosY ) );*/

        m_StatsWindow.AddLine( VAR_TO_ANSI( Capture::GNumProfileEvents ) );
        m_StatsWindow.AddLine( VAR_TO_ANSI( Capture::GNumInstalledHooks ) );
        m_StatsWindow.AddLine( VAR_TO_ANSI( m_TimeGraph.GetNumDrawnTextBoxes() ) );
        m_StatsWindow.AddLine( VAR_TO_ANSI( m_TimeGraph.m_TextBoxes.m_NumItems ) );
        m_StatsWindow.AddLine( VAR_TO_ANSI( m_TimeGraph.m_TextBoxes.m_NumBlocks ) );

        for( std::string & line : GTcpServer->GetStats() )
        {
            m_StatsWindow.AddLine( line );
        }

        bool hasConnection = GTcpServer->HasConnection();
        m_StatsWindow.AddLine( VAR_TO_ANSI( hasConnection ) );

        m_StatsWindow.Draw( "Capture Stats", &m_DrawStats );
    }

    if( m_DrawHelp )
    {
        RenderHelpUi();
        
        if( m_FirstHelpDraw )
        {
            // Redraw so that Imgui resizes the 
            // window properly on first draw
            NeedsRedraw();
            m_FirstHelpDraw = false;
        }
    }

    if( m_DrawMemTracker && !m_DrawHelp )
    {
        RenderMemTracker();
    }

    // Rendering
    glViewport(0, 0, getWidth(), getHeight());
    ImGui::Render();

    RenderBar();
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderText()
{
    if( !m_Picking )
    {
        m_TimeGraph.DrawText();
    }
}

//-----------------------------------------------------------------------------
void ColorToFloat( Color a_Color, float* o_Float )
{
    for( int i = 0; i < 4; ++i )
    {
        o_Float[i] = (float)a_Color[i] / 255.f;
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderHelpUi()
{
    float barHeight = m_Slider.GetPixelHeight();
    ImGui::SetNextWindowPos( ImVec2( 0, barHeight*1.5f ) );

    ImVec4 color( 1.f, 0, 0, 1.f );
    ColorToFloat( m_Slider.GetBarColor(), &color.x );
    ImGui::PushStyleColor(ImGuiCol_WindowBg, color );

    if( !ImGui::Begin( "Help Overlay", &m_DrawHelp, ImVec2( 0, 0 ), 1.f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
    {
        ImGui::PopStyleColor();
        ImGui::End();
        return;
    }

    ImGui::Text( "Start/Stop capture: 'X'" );
    ImGui::Text( "Time zoom: scroll or CTRL+right-click/drag" );
    ImGui::Text( "Y axis zoom: CTRL+scroll" );
    ImGui::Text( "Zoom last 2 seconds: 'A'" );
    ImGui::Separator();
    ImGui::Text( "Icons:" );

    extern GLuint GTextureInjected;
    extern GLuint GTextureTimer;
    extern GLuint GTextureHelp;
    extern GLuint GTextureRecord;
    float size = m_Slider.GetPixelHeight();

    ImGui::Image((void*)(DWORD64)GTextureInjected, ImVec2(size, size), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
    ImGui::SameLine();
    ImGui::Text("Injected in process");

    ImGui::Image( (void*)(DWORD64)GTextureTimer, ImVec2( size, size ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 128 ) );
    ImGui::SameLine();
    ImGui::Text( "Capture time" );

    ImGui::Image( (void*)(DWORD64)GTextureRecord, ImVec2( size, size ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 128 ) );
    ImGui::SameLine();
    ImGui::Text( "Recording" );

    ImGui::Image( (void*)(DWORD64)GTextureHelp, ImVec2( size, size ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 128 ) );
    ImGui::SameLine();
    ImGui::Text( "Help" );

    ImGui::End();

    ImGui::PopStyleColor();
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderMemTracker()
{
    float barHeight = m_Slider.GetPixelHeight();
    ImGui::SetNextWindowPos( ImVec2( 0, barHeight*1.5f ) );

    ImVec4 color( 1.f, 0, 0, 1.f );
    ColorToFloat( m_Slider.GetBarColor(), &color.x );
    ImGui::PushStyleColor( ImGuiCol_WindowBg, color );

    bool dummy = true;
    if( !ImGui::Begin( "MemTracker Overlay", &dummy, ImVec2( 0, 0 ), 1.f, ImGuiWindowFlags_NoTitleBar /*| ImGuiWindowFlags_NoResize*/ | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
    {
        ImGui::PopStyleColor();
        ImGui::End();
        return;
    }

    ImGui::Text( "=== Memory Tracker ===" );

    MemoryTracker & memTracker = m_TimeGraph.m_MemTracker;
    if( memTracker.NumAllocatedBytes() == 0 )
    {
        std::string str = VAR_TO_ANSI( memTracker.NumAllocatedBytes() ) + std::string( "            ");
        ImGui::Text( str.c_str() );
        ImGui::Text( VAR_TO_ANSI( memTracker.NumFreedBytes() ) );
        ImGui::Text( VAR_TO_ANSI( memTracker.NumLiveBytes() ) );
    }
    else
    {
        ImGui::Text( VAR_TO_ANSI( memTracker.NumAllocatedBytes() ) );
        ImGui::Text( VAR_TO_ANSI( memTracker.NumFreedBytes() ) );
        ImGui::Text( VAR_TO_ANSI( memTracker.NumLiveBytes() ) );
    }

    ImGui::End();

    ImGui::PopStyleColor();
}

//-----------------------------------------------------------------------------
void DrawTexturedSquare( GLuint a_TextureId, float a_Size, float a_X, float a_Y )
{
    glUseProgram( 0 );
    glColor4ub( 255, 255, 255, 255 );

    glEnable( GL_TEXTURE_2D );
    glDisable( GL_COLOR_MATERIAL );
    glBindTexture( GL_TEXTURE_2D, a_TextureId );

    glBegin( GL_QUADS );
    glTexCoord2f( 0.f, 1.f );   glVertex3f( a_X, a_Y, 0.f );
    glTexCoord2f( 0.f, 0.f );   glVertex3f( a_X, a_Y + a_Size, 0.f );
    glTexCoord2f( 1.f, 0.f );   glVertex3f( a_X + a_Size, a_Y+a_Size, 0.f );
    glTexCoord2f( 1.f, 1.f );   glVertex3f( a_X + a_Size, a_Y, 0.f );
    glEnd();

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderBar()
{
    extern GLuint GTextureInjected;
    extern GLuint GTextureTimer;
    extern GLuint GTextureHelp;
    extern GLuint GTextureRecord;

    float barHeight = m_Slider.GetPixelHeight();
    float size = barHeight;
    float h = (float)getHeight();
    float y = h - barHeight;
    float timerX = getWidth()/2.f;
    DrawTexturedSquare( GTextureInjected, size, m_ProcessX - size, y );
    DrawTexturedSquare( GTextureTimer, size, timerX, y );
    DrawTexturedSquare( GTextureHelp, size, 0, y );

    if( Capture::IsCapturing() )
    {
        DrawTexturedSquare( GTextureRecord, size, timerX - size, y );
    }
    
    double timeSpan = m_TimeGraph.GetSessionTimeSpanUs();
    timerX += size;
    m_TextRenderer.AddText2D( Format( "%s", GetPrettyTime( timeSpan*0.001 ).c_str() ).c_str()
                            , (int)timerX
                            , (int)GetTopBarTextY()
                            , GlCanvas::Z_VALUE_TEXT_UI
                            , Color( 255, 255, 255, 255 ) );

    /*m_TextRenderer.AddText2D(Format("%s", GetPrettyTime(m_TimeGraph.GetCurrentTimeSpanUs()*0.001).c_str()).c_str()
        , timerX
        , 100
        , GlCanvas::Z_VALUE_TEXT_UI
        , Color(255, 255, 255, 255));*/
}

//-----------------------------------------------------------------------------
inline double GetIncrementMs( double a_MilliSeconds )
{
    const double Day = 24 * 60 * 60 * 1000;
    const double Hour = 60 * 60 * 1000;
    const double Minute = 60 * 1000;
    const double Second = 1000;
    const double Milli = 1;
    const double Micro = 0.001;
    const double Nano = 0.000001;

    std::string res;

    if (a_MilliSeconds < Micro)
        return Nano;
    else if (a_MilliSeconds < Milli)
        return Micro;
    else if (a_MilliSeconds < Second)
        return Milli;
    else if (a_MilliSeconds < Minute)
        return Second;
    else if (a_MilliSeconds < Hour)
        return Minute;
    else if (a_MilliSeconds < Day)
        return Hour;
    else
        return Day;
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderTimeBar()
{
    static int numTimePoints = 10;
    
    if( m_TimeGraph.GetSessionTimeSpanUs() > 0 )
    {
        double millis = m_TimeGraph.GetCurrentTimeSpanUs()*0.001;
        double incr   = millis/float(numTimePoints-1);
        double unit   = GetIncrementMs(incr);
        double normInc= (double((int)((incr+unit)/unit)))*unit;

        double startMs = m_TimeGraph.m_MinEpochTimeUs*0.001;
        double endMs   = m_TimeGraph.m_MaxEpochTimeUs*0.001;

        double normStartUs = 1000.0*(double(int(startMs/normInc)))*normInc;

        static int pixelMargin = 2;
        int screenY = getHeight() - (int)m_Slider.GetPixelHeight() - pixelMargin;
        float dummy, worldY;
        ScreenToWorld( 0, screenY, dummy, worldY );

        float height = ScreenToWorldHeight( (int)GParams.m_FontSize + pixelMargin );
        float xMargin = ScreenToworldWidth( 4 );

        for (int i = 0; i < numTimePoints; ++i)
        {
            double currentMicros = normStartUs + double(i)*1000*normInc;
            if( currentMicros < 0 )
                continue;

            double currentMillis = currentMicros * 0.001;
            std::string text = GetPrettyTime(currentMillis);
            float worldX = m_TimeGraph.GetWorldFromUs(currentMicros);
            m_TextRenderer.AddText(text.c_str(), worldX + xMargin, worldY, GlCanvas::Z_VALUE_TEXT_UI, Color(255, 255, 255, 255));
        
            glColor4f(1.f, 1.f, 1.f, 1.f);
            glBegin(GL_LINES);
            glVertex3f(worldX, worldY, Z_VALUE_UI);
            glVertex3f(worldX, worldY + height, Z_VALUE_UI);
            glEnd();
        }
    }
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnTimerAdded( Timer & a_Timer )
{
    m_TimeGraph.ProcessTimer( a_Timer );
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnContextSwitchAdded( const ContextSwitch & a_CS )
{
    m_TimeGraph.AddContextSwitch( a_CS );
}
