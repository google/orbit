//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ThreadView.h"
#include "Log.h"
#include "Capture.h"
#include "OrbitThread.h"
#include "SamplingProfiler.h"

std::shared_ptr<SamplingProfiler> ThreadViewManager::s_CurrentSamplingProfiler = nullptr;

//-----------------------------------------------------------------------------
ThreadView::ThreadView()
{
    SetBackgroundColor(Vec4( 45.f / 255.f, 45.f / 255.f, 48.f / 255.f, 1.0f ));
    m_ThreadViewWindow = new ThreadViewWindow();

    if( ThreadViewManager::s_CurrentSamplingProfiler )
    {
        m_ThreadViewWindow->m_SamplingProfiler = ThreadViewManager::s_CurrentSamplingProfiler;
    }
}

//-----------------------------------------------------------------------------
ThreadView::~ThreadView()
{
    delete m_ThreadViewWindow;
}

//-----------------------------------------------------------------------------
void ThreadView::OnTimer()
{
    GlCanvas::OnTimer();
}

//-----------------------------------------------------------------------------
void ThreadView::RenderUI()
{
    if (!m_DrawUI)
        return;

    ScopeImguiContext state(m_ImGuiContext);
    Orbit_ImGui_NewFrame(this);

    ImVec2 size( (float)getWidth(), (float)getHeight() );
    m_ThreadViewWindow->Draw("ThreadView", nullptr, &size);

    // Rendering
    glViewport(0, 0, getWidth(), getHeight());
    ImGui::Render();

    if (m_IsSelecting)
    {
        float sizex = fabs(m_SelectStop[0] - m_SelectStart[0]);
        float sizey = fabs(m_SelectStop[1] - m_SelectStart[1]);
        float posx = min(m_SelectStop[0], m_SelectStart[0]);
        float posy = min(m_SelectStop[1], m_SelectStart[1]);
        Vec2 pos(posx, posy);
        Vec2 size(sizex, sizey);

        TextRenderer & textRenderer = GetTextRenderer();
        TextBox box(pos, size, "", &textRenderer, Color(255, 255, 255, 255));
        box.Draw(GetTextRenderer(), -FLT_MAX, true, true);
    }
}

//-----------------------------------------------------------------------------
void ThreadView::KeyPressed( unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt )
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
void ThreadView::Draw()
{
    
}

//-----------------------------------------------------------------------------
ThreadViewWindow::ThreadViewWindow() : WindowFlags(0)
{
    FitCanvas();
}

//-----------------------------------------------------------------------------
void ThreadViewWindow::FitCanvas()
{
    WindowFlags |= ImGuiWindowFlags_NoTitleBar;
    //WindowFlags |= ImGuiWindowFlags_ShowBorders;
    WindowFlags |= ImGuiWindowFlags_NoResize;
    WindowFlags |= ImGuiWindowFlags_NoMove;
    //WindowFlags |= ImGuiWindowFlags_NoScrollbar;
    WindowFlags |= ImGuiWindowFlags_NoCollapse;
    //WindowFlags |= ImGuiWindowFlags_MenuBar;
}

//-----------------------------------------------------------------------------
void ThreadViewWindow::Draw(const char* title, bool* p_opened, ImVec2* a_Size, SamplingProfiler* a_Profiler)
{
    if( m_SamplingProfiler )
        DrawReport( title, p_opened, a_Size );
    else
        DrawLive( title, p_opened, a_Size, a_Profiler );
}

ImVec2 GThreadViewGraphSize(200, 20);
int GThreadViewColWidth = 8;
int GThreadViewColumnOffset = 80;

//-----------------------------------------------------------------------------
void ThreadViewWindow::DrawLive(const char* title, bool* p_opened, ImVec2* a_Size, SamplingProfiler* a_Profiler)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    if (a_Size)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImVec2 CanvasSize = *a_Size;
        CanvasSize.x -= 20;
        CanvasSize.y -= 20;
        ImGui::SetNextWindowSize(CanvasSize, ImGuiSetCond_Always);
        ImGui::Begin(title, p_opened, CanvasSize, 1.f, WindowFlags);
    }
    else
    {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
        ImGui::Begin(title, p_opened, WindowFlags);
    }

    if( Capture::GTargetProcess )
    {
        if( !Capture::GIsSampling )
        {
            if( ImGui::Button("Start Sampling") )
            {
                Capture::StartSampling();
            }
        }
        else
        {
            if( ImGui::Button("Stop Sampling") )
            {
                Capture::StopSampling();
            }
        }
    }

    ImGui::Text("Num Ticks: %i", Capture::GNumSamplingTicks);
    ImGui::Text("Num Samples: %i", Capture::GNumSamples);

    ImGui::Columns(2, "ThreadViewColumns");
    ImGui::Separator();
    bool sortByThread = ImGui::Button("Thread"); ImGui::NextColumn();
    bool sortByUsage  = ImGui::Button("Usage");  ImGui::NextColumn();

    ImGui::Separator();
    const char* names[3] = { "One", "Two", "Three" };
    const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
    static int selected = -1;

    
    ImGui::SetColumnOffset( 1, (float)GThreadViewColumnOffset );

    int i = 0;
    if (Capture::GTargetProcess)
    {
        if (sortByThread)
        {
            Capture::GTargetProcess->SortThreadsById();
        }

        if (sortByUsage)
        {
            Capture::GTargetProcess->SortThreadsByUsage();
        }

        for( auto & thread : Capture::GTargetProcess->GetThreads() )
        {
            char label[32];
            sprintf(label, "%*i", GThreadViewColWidth, thread->m_TID);
            if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
                selected = i;
            ImGui::NextColumn();
            std::string ThreadUsage = Format("%.2f %%", thread->m_Usage.Latest());
            //ImGui::SameLine();
            ImGui::PlotLines(ThreadUsage.c_str(), thread->m_Usage.Data(), thread->m_Usage.Size(), thread->m_Usage.IndexOfOldest(), nullptr/*"avg 0.0"*/, 0.f, 100.f, GThreadViewGraphSize);
            ImGui::NextColumn();
            ++i;
        }
    }

    ImGui::Columns(1);

    if (Filter.IsActive())
    {

    }
    else
    {
        ImGui::TextUnformatted(Buf.begin());
    }

    //ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar();
}

//-----------------------------------------------------------------------------
void ThreadViewWindow::DrawReport( const char* title, bool* p_opened, ImVec2* a_Size )
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    if (a_Size)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImVec2 CanvasSize = *a_Size;
        CanvasSize.x -= 20;
        CanvasSize.y -= 20;
        ImGui::SetNextWindowSize(CanvasSize, ImGuiSetCond_Always);
        ImGui::Begin(title, p_opened, CanvasSize, 1.f, WindowFlags);
    }
    else
    {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
        ImGui::Begin(title, p_opened, WindowFlags);
    }

    static ImVec2 graphSize(200, 20);
    static int width = 8;

    ImGui::Columns(2, "ThreadViewColumns");
    ImGui::Separator();
    bool sortByThread = ImGui::Button("Thread"); ImGui::NextColumn();
    bool sortByUsage = ImGui::Button("Usage");  ImGui::NextColumn();

    ImGui::Separator();
    const char* names[3] = { "One", "Two", "Three" };
    const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
    static int selected = -1;

    static int columnOffset = 80;
    ImGui::SetColumnOffset( 1, (float)columnOffset );

    int i = 0;
    if (m_SamplingProfiler)
    {
        if (sortByThread)
        {
            m_SamplingProfiler->SortByThreadID();
        }

        if (sortByUsage)
        {
            m_SamplingProfiler->SortByThreadUsage();
        }

        for (ThreadSampleData* threadData : m_SamplingProfiler->GetThreadSampleData())
        {
            char label[32];
            sprintf(label, "%*i", width, threadData->m_TID);
            if (ImGui::Selectable(label, selected == i))
                selected = i;
            ImGui::NextColumn();
            //ImGui::SameLine();
            std::string average = Format("%f", threadData->m_AverageThreadUsage);
            ImGui::PlotLines( average.c_str(), threadData->m_ThreadUsage.data(), (int)threadData->m_ThreadUsage.size(), 0, nullptr/*"avg 0.0"*/, 0.f, 100.f, graphSize);
            ImGui::NextColumn();
            ++i;
        }
    }

    ImGui::Columns(1);

    if (Filter.IsActive())
    {

    }
    else
    {
        ImGui::TextUnformatted(Buf.begin());
    }

    //ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar();
}
