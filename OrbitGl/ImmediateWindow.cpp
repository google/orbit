//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ImmediateWindow.h"
#include "Log.h"

//-----------------------------------------------------------------------------
ImmediateWindow::ImmediateWindow()
{
    assert(0);
    m_ImmediateWindow.FitCanvas();
}

//-----------------------------------------------------------------------------
ImmediateWindow::~ImmediateWindow()
{
}

//-----------------------------------------------------------------------------
void ImmediateWindow::OnTimer()
{
    GlCanvas::OnTimer();
}

//-----------------------------------------------------------------------------
void ImmediateWindow::RenderUI()
{
    if (!m_DrawUI)
        return;

    ScopeImguiContext state(m_ImGuiContext);
    Orbit_ImGui_NewFrame(this);

    RenderSamplingUI();
    RenderProcessUI();
    ImVec4 clear_color = ImColor(114, 144, 154);

    //GLogger.GetLockedLog(Log::Viz, [&](const std::vector<std::string> & a_Entries)
    //{
    //    for( auto & line : a_Entries )
    //    {
    //        m_ImmediateWindow.AddLog(line.c_str());
    //        //m_ImmediateWindow.AddLog("\n");
    //    }

    //    ImVec2 size((float)getWidth(), (float)getHeight());
    //    m_ImmediateWindow.Draw("Immediate", nullptr, &size );
    //}, true );

    // Rendering
    glViewport(0, 0, getWidth(), getHeight());
    ImGui::Render();
}

//-----------------------------------------------------------------------------
void ImmediateWindow::RenderProcessUI()
{
}

//-----------------------------------------------------------------------------
void ImmediateWindow::KeyPressed( unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt )
{
    ScopeImguiContext state(m_ImGuiContext);

    if (!m_ImguiActive)
    {
        /*switch (event.GetKeyCode())
        {
            case 'D':
                m_DrawDebugDisplay = !m_DrawDebugDisplay;
                Refresh();
                break;
                case 'U':
                m_DrawTestUI = !m_DrawTestUI;
                Refresh();
                break;
                case 'L':
                m_DrawLog = !m_DrawLog;
                Refresh();
                break;
        }*/
    }

    ImGuiIO& io = ImGui::GetIO();
    io.KeyCtrl = a_Ctrl;
    io.KeyShift = a_Shift;
    io.KeyAlt = a_Alt;
    Orbit_ImGui_KeyCallback(this, a_KeyCode, true);
}

//-----------------------------------------------------------------------------
void ImmediateWindow::Draw()
{
    
}
