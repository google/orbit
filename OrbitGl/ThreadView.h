//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "GlCanvas.h"
#include "ImGuiOrbit.h"

class SamplingProfiler;

class ThreadView : public GlCanvas
{
public:
    ThreadView();
    virtual ~ThreadView();

    void Draw() override;
    void RenderUI() override;

    void KeyPressed( unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt ) override;
    void OnTimer() override;

    struct ThreadViewWindow* m_ThreadViewWindow;
};

struct ThreadViewManager
{
    static std::shared_ptr<SamplingProfiler> s_CurrentSamplingProfiler;
};

struct ThreadViewWindow
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImGuiWindowFlags    WindowFlags;

    ThreadViewWindow();
    void FitCanvas();
    void Draw(const char* title, bool* p_opened = NULL, ImVec2* a_Size = nullptr, SamplingProfiler* a_Profiler = nullptr);
    void DrawLive(const char* title, bool* p_opened = NULL, ImVec2* a_Size = nullptr, SamplingProfiler* a_Profiler = nullptr);
    void DrawReport(const char* title, bool* p_opened = NULL, ImVec2* a_Size = nullptr);
    
    std::shared_ptr<SamplingProfiler> m_SamplingProfiler;
};

