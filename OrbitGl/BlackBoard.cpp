//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TcpServer.h"
#include "BlackBoard.h"
#include "Capture.h"
#include "Card.h"
#include "Context.h"
#include "Log.h"
#include "RuleEditor.h"
#include "App.h"


//-----------------------------------------------------------------------------
BlackBoard::BlackBoard()
    : GlCanvas()
{
}

//-----------------------------------------------------------------------------
BlackBoard::~BlackBoard()
{
}

//-----------------------------------------------------------------------------
void BlackBoard::OnTimer()
{
    GlCanvas::OnTimer();
}

//-----------------------------------------------------------------------------
void BlackBoard::ZoomAll()
{
}

//-----------------------------------------------------------------------------
void BlackBoard::KeyPressed( unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt )
{
    if (!m_ImguiActive)
    {
        switch (a_KeyCode)
        {
        case 'A':
            ZoomAll();
            break;
        }
    }

    ImGuiIO& io = ImGui::GetIO();
    io.KeyCtrl = a_Ctrl;
    io.KeyShift = a_Shift;
    io.KeyAlt = a_Alt;

    Orbit_ImGui_KeyCallback(this, a_KeyCode, true);
}

ImVector<ImVec2> points;
ImVec2 GMin;
ImVec2 GMax;

//-----------------------------------------------------------------------------
void BlackBoard::AddPos( float x, float y )
{
    points.push_back( ImVec2(x, y) );
}

//-----------------------------------------------------------------------------
static void ShowExampleAppCustomRendering(bool* opened)
{
    GMin = ImVec2(FLT_MAX, FLT_MAX);
    GMax = ImVec2(-FLT_MAX, -FLT_MAX);
    for( ImVec2 p : points )
    {
        if( p.x < GMin.x )
            GMin.x = p.x;
        if (p.y < GMin.y)
            GMin.y = p.y;
        if( p.x > GMax.x)
            GMax.x = p.x;
        if( p.y > GMax.y )
            GMax.y = p.y;
    }
    ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiSetCond_FirstUseEver);
    if (!ImGui::Begin("Player Position XY", opened))
    {
        ImGui::End();
        return;
    }

    // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of overloaded operators, etc.
    // Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your types and ImVec2/ImVec4.
    // ImGui defines overloaded operators but they are internal to imgui.cpp and not exposed outside (to avoid messing with your types)
    // In this example we are not using the maths operators!
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGui::Separator();
    {
        static bool adding_line = false;
        //ImGui::Text("Canvas example");
        if (ImGui::Button("Clear")) points.clear();
        /*if (points.Size >= 2) 
        { ImGui::SameLine(); 
            if (ImGui::Button("Undo")) 
            { points.pop_back(); points.pop_back(); 
            } 
        }*/
        //ImGui::Text("Left-click and drag to add lines,\nRight-click to undo");

        // Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
        // However you can draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
        // If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
        if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
        if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
        draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(50,50,50), ImColor(50,50,60), ImColor(60,60,70), ImColor(50,50,60));
        draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(255,255,255));

        bool adding_preview = false;
        ImGui::InvisibleButton("canvas", canvas_size);
        /*if (ImGui::IsItemHovered())
        {
            ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
            if (!adding_line && ImGui::IsMouseClicked(0))
            {
                points.push_back(mouse_pos_in_canvas);
                adding_line = true;
            }
            if (adding_line)
            {
                adding_preview = true;
                points.push_back(mouse_pos_in_canvas);
                if (!ImGui::GetIO().MouseDown[0])
                    adding_line = adding_preview = false;
            }
            if (ImGui::IsMouseClicked(1) && !points.empty())
            {
                adding_line = adding_preview = false;
                points.pop_back();
                points.pop_back();
            }
        }*/
        draw_list->PushClipRect(ImVec2(canvas_pos.x, canvas_pos.y), ImVec2(canvas_pos.x+canvas_size.x, canvas_pos.y+canvas_size.y));      // clip lines within the canvas (if we resize it, etc.)

        float xsize = GMax.x - GMin.x;
        float ysize = GMax.y - GMin.y;
        float size = std::max(ysize, xsize);
        if( size == 0.f ) size = 1.f;
        for (int i = 0; i < points.Size - 1; i += 2)
        {
            ImVec2 p0 = points[i];
            ImVec2 p1 = points[i+1];

            p0.x = ((p0.x - GMin.x)/size)*canvas_size.x;
            p1.x = ((p1.x - GMin.x)/size)*canvas_size.x;

            p0.y = ((p0.y - GMin.y) / size)*canvas_size.y;
            p1.y = ((p1.y - GMin.y) / size)*canvas_size.y;
            
            draw_list->AddLine(ImVec2(canvas_pos.x + p0.x, canvas_pos.y + p0.y), ImVec2(canvas_pos.x + p1.x, canvas_pos.y + p1.y), 0xFF00FFFF, 2.0f);
        }
        draw_list->PopClipRect();
        if (adding_preview)
            points.pop_back();
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
void BlackBoard::RenderUI()
{
    ScopeImguiContext state(m_ImGuiContext);
    Orbit_ImGui_NewFrame( this );
    GCardContainer.DrawImgui( this );
    
    // Rendering
    glViewport(0, 0, getWidth(), getHeight());
    ImGui::Render();
}

//-----------------------------------------------------------------------------
bool BlackBoard::GetNeedsRedraw() const
{
    extern bool GRedrawBlackBoard;
    return m_NeedsRedraw || GRedrawBlackBoard;
    GRedrawBlackBoard = false;
}
