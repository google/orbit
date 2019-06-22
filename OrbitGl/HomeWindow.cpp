//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "HomeWindow.h"
#include "Log.h"
#include "App.h"

//-----------------------------------------------------------------------------
HomeWindow::HomeWindow()
{
    m_DrawDebugDisplay = false;
    m_DrawTestUI = false;
    m_DrawLog = true;

    VariableTracing::AddCallback( [=]( std::vector<std::string> & a_Entries ){ this->VariableTracingCallback(a_Entries); } );
}

//-----------------------------------------------------------------------------
HomeWindow::~HomeWindow()
{
    //TODO: remove variable tracing callback
}

//-----------------------------------------------------------------------------
void HomeWindow::VariableTracingCallback(std::vector< std::string > & a_Entries)
{
    if (m_DrawDebugDisplay)
    {
        m_DebugWindow.Clear();
        for (std::string & entry : a_Entries)
        {
            m_DebugWindow.AddLog("%s\n", entry.c_str());
        }
    }
}

//-----------------------------------------------------------------------------
void HomeWindow::OnTimer()
{
    GlCanvas::OnTimer();
}

//-----------------------------------------------------------------------------
void HomeWindow::RenderUI()
{
    if (!m_DrawUI)
        return;

    ScopeImguiContext state(m_ImGuiContext);

    Orbit_ImGui_NewFrame(this);

    RenderSamplingUI();
    RenderProcessUI();

    m_WatchWindow.Draw("Watch");

    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    if (m_DrawTestUI)
    {
        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Test Window")) show_test_window ^= 1;
            if (ImGui::Button("Another Window")) show_another_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow();
        }
    }

    if (m_DrawDebugDisplay)
    {
        m_DebugWindow.Draw("Debug", &m_DrawDebugDisplay);
    }

    if (m_DrawLog)
    {
        GLogger.GetLockedLog(OrbitLog::Global, [&](const std::vector<std::string> & a_Entries)
        {
            m_LogWindow.Draw("Log", a_Entries, &m_LogWindow.m_Open);
        });
    }

    // Rendering
    glViewport(0, 0, getWidth(), getHeight());
    ImGui::Render();
}

//-----------------------------------------------------------------------------
void HomeWindow::RenderProcessUI()
{
}

//-----------------------------------------------------------------------------
void HomeWindow::KeyPressed( unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt )
{
    ScopeImguiContext state(m_ImGuiContext);

    if (!m_ImguiActive)
    {
        switch (a_KeyCode)
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
        }
    }

    ImGuiIO& io = ImGui::GetIO();
    io.KeyCtrl = a_Alt;
    io.KeyShift = a_Shift;
    io.KeyAlt = a_Alt;
    Orbit_ImGui_KeyCallback(this, a_KeyCode, true);
}

//-----------------------------------------------------------------------------
void HomeWindow::Draw()
{
    static volatile bool doCallbacks = true;
    if (doCallbacks)
    {
        VariableTracing::ProcessCallbacks();
    }
}
