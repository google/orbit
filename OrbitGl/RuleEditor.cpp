//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "RuleEditor.h"

#include "../OrbitPlugin/OrbitSDK.h"
#include "App.h"
#include "Capture.h"
#include "Card.h"
#include "Context.h"
#include "Core.h"
#include "Log.h"
#include "OrbitFunction.h"
#include "OrbitType.h"
#include "PluginManager.h"
#include "TcpServer.h"

//-----------------------------------------------------------------------------
RuleEditorWindow::RuleEditorWindow(Function* a_Function)
    : m_WindowFlags(0),
      m_Function(a_Function),
      m_Type(nullptr),
      m_LastVariable(nullptr) {
  m_Text = "this";
}

//-----------------------------------------------------------------------------
void RuleEditorWindow::SetInputFromActiveIndex(ImGuiTextEditCallbackData* data,
                                               int entryIndex) {
  RuleEditorWindow* ruleEditor =
      reinterpret_cast<RuleEditorWindow*>(data->UserData);
  UNUSED(ruleEditor);

  std::string entry = m_AutoComplete[entryIndex];
  ReplaceStringInPlace(entry, GetCurrentWord(m_Text), "");
  m_Text += entry;

  const size_t length = strlen(m_Text.c_str());
  memmove(data->Buf, m_Text.c_str(), length + 1);

  data->BufTextLen = (int)length;
  data->BufDirty = true;

  UpdateTextBuffer();
}

//-----------------------------------------------------------------------------
void RuleEditorWindow::OnWordSelected(const std::string& a_Word) {
  m_Text += a_Word;
}

//-----------------------------------------------------------------------------
std::shared_ptr<Variable> RuleEditorWindow::GetLastVariable(
    const std::string& a_Chain) {
  Type* type = m_Type;
  std::shared_ptr<Variable> var = type ? type->GetTemplateVariable() : nullptr;

  std::vector<std::string> tokens = Tokenize(a_Chain, ".->");
  if (tokens.size() > 0) {
    type = m_Type;

    for (size_t i = tokens[0] == "this" ? 1 : 0;
         var && type && i < tokens.size(); ++i) {
      std::shared_ptr<Variable> child = var->FindImmediateChild(tokens[i]);
      if (child) {
        var = child;
      }
      type = child ? child->GetType() : nullptr;
    }
  }

  return var;
}

//-----------------------------------------------------------------------------
std::string RuleEditorWindow::GetCurrentWord(const std::string a_Chain) {
  std::vector<std::string> tokens = Tokenize(a_Chain, ".->");
  if (tokens.size() > 0) {
    std::string lastWord = tokens[tokens.size() - 1];
    if (absl::EndsWith(a_Chain, lastWord.c_str())) {
      return lastWord;
    }
  }

  return "";
}

//-----------------------------------------------------------------------------
void RuleEditorWindow::RefreshAutoComplete(const std::string& a_Line) {
  // if( a_Line.size() && a_Line != m_LastText )
  {
    m_AutoComplete.clear();
    m_MaxTextWidth = 0;
    std::shared_ptr<Variable> lastVar = GetLastVariable(a_Line);
    Type* type = lastVar ? lastVar->GetType() : nullptr;
    if (type) {
      std::string currentWord = GetCurrentWord(a_Line);
      for (auto& pair : type->m_DataMembers) {
        Variable& var = pair.second;
        const std::string& varName = var.m_Name;

        if (absl::StrContains(varName, currentWord)) {
          m_AutoComplete.push_back(varName);
          m_MaxTextWidth =
              std::max(m_MaxTextWidth, ImGui::CalcTextSize(varName.c_str()).x);
        }
      }
    }

    m_PopupHeight = ImGui::CalcTextSize("Test").y * m_AutoComplete.size();
    m_LastVariable = lastVar;
    m_LastText = a_Line;
    if (m_AutoComplete.size() == 0) {
      m_State.m_PopupOpen = false;
    }
  }
}

//-----------------------------------------------------------------------------
void RuleEditorWindow::Launch(Function* a_Function) {
  m_Function = a_Function;
  if (m_Function) {
    m_Type = m_Function->GetParentType();
    if (m_Type) {
      m_Type->LoadDiaInfo();
    }
  }
}

//-----------------------------------------------------------------------------
bool RuleEditorWindow::CreateRule() {
  if (m_Function && m_Function->Hookable()) {
    std::shared_ptr<Variable> var = GetLastVariable(m_Text);
    if (var) {
      m_Function->Select();
      std::unordered_map<DWORD64, std::shared_ptr<Rule>>& rules =
          GOrbitApp->GetRuleEditor()->GetRules();
      std::shared_ptr<Rule> rule = rules[m_Function->GetVirtualAddress()];
      if (rule == nullptr) {
        rule = rules[m_Function->GetVirtualAddress()] =
            std::make_shared<Rule>();
        rule->m_Function = m_Function;
      }

      rule->m_TrackedVariables.push_back(var);
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
int InputCallbackGlobal(ImGuiTextEditCallbackData* data) {
  RuleEditorWindow* ruleEditor =
      reinterpret_cast<RuleEditorWindow*>(data->UserData);
  return ruleEditor->InputCallback(data);
}

//-----------------------------------------------------------------------------
int RuleEditorWindow::InputCallback(ImGuiTextEditCallbackData* data) {
  switch (data->EventFlag) {
    case ImGuiInputTextFlags_CallbackCompletion:
      if (m_State.m_PopupOpen && m_State.m_ActiveIdx != -1) {
        // Tab was pressed, grab the item's text
        SetInputFromActiveIndex(data, m_State.m_ActiveIdx);
      }

      m_State.m_PopupOpen = false;
      m_State.m_ActiveIdx = -1;
      m_State.m_ClickedIdx = -1;
      break;
    case ImGuiInputTextFlags_CallbackHistory:
      if (data->EventKey == ImGuiKey_UpArrow) {
        --m_State.m_ActiveIdx;
        m_State.m_SelectionChanged = true;
      } else if (data->EventKey == ImGuiKey_DownArrow) {
        ++m_State.m_ActiveIdx;
        m_State.m_SelectionChanged = true;
      }
      break;
    case ImGuiInputTextFlags_CallbackAlways:
      if (m_State.m_ClickedIdx != -1) {
        // The user has clicked an item, grab the item text
        SetInputFromActiveIndex(data, m_State.m_ClickedIdx);

        // Hide the popup
        m_State.m_PopupOpen = false;
        m_State.m_ActiveIdx = -1;
        m_State.m_ClickedIdx = -1;
      }

      break;

    case ImGuiInputTextFlags_CallbackCharFilter:
      if (data->EventChar == '.') {
        ImVec2 pos = ImGui::GetCursorPos();
        m_PopupPos = pos;
        m_PopupPos.y += ImGui::GetFontSize();
        m_State.m_PopupOpen = true;
      }
      break;
  }

  return 0;
}

//-----------------------------------------------------------
void DrawWindow(State& state, ImVec2& popupPos, ImVec2& popupSize,
                bool& isFocused) {
  ImGuiWindowFlags winFlags = 0;

  // Set this flag in order to be able to show the popup on top
  // of the window. This is only really necessary if you want to
  // show the popup over the window region
  if (state.m_PopupOpen) winFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

  /// Begin main window
  ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("DeveloperConsole", nullptr, winFlags)) {
    ImGui::End();
    return;
  }

  /// Draw any window content here
  {
    ImGui::BeginChild("scrollRegion",
                      ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), true,
                      ImGuiWindowFlags_HorizontalScrollbar);

    // Draw entries
    for (int i = 0; i < 3; i++) ImGui::Text("Foo %d", i);

    ImGui::EndChild();
  }

  /// Input Box
  {
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                ImGuiInputTextFlags_CallbackAlways |
                                ImGuiInputTextFlags_CallbackCharFilter |
                                ImGuiInputTextFlags_CallbackCompletion |
                                ImGuiInputTextFlags_CallbackHistory;

    const size_t INPUT_BUF_SIZE = 256;
    static char inputBuf[INPUT_BUF_SIZE] = {0};

    if (ImGui::InputText("Input", inputBuf, INPUT_BUF_SIZE, flags,
                         InputCallbackGlobal, &state)) {
      ImGui::SetKeyboardFocusHere(-1);

      if (state.m_PopupOpen && state.m_ActiveIdx != -1) {
        // This means that enter was pressed whilst a
        // the popup was open and we had an 'active' item.
        // So we copy the entry to the input buffer here
        const char* entry = "";  // ENTRIES[state.activeIdx];
        const size_t length = strlen(entry);

        memmove(inputBuf, entry, length + 1);
      } else {
        // Handle text input here
        inputBuf[0] = '\0';
      }

      // Hide popup
      state.m_PopupOpen = false;
      state.m_ActiveIdx = -1;
    }

    // Restore focus to the input box if we just clicked an item
    if (state.m_ClickedIdx != -1) {
      ImGui::SetKeyboardFocusHere(-1);

      // NOTE: We do not reset the 'clickedIdx' here because
      // we want to let the callback handle it in order to
      // modify the buffer, therefore we simply restore keyboard input instead
      state.m_PopupOpen = false;
    }

    // Get input box position, so we can place the popup under it
    popupPos = ImGui::GetItemRectMin();

    // Based on Omar's developer console demo: Retain focus on the input box
    if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() &&
        !ImGui::IsMouseClicked(0)) {
      ImGui::SetKeyboardFocusHere(-1);
    }

    // Grab the position for the popup
    popupSize = ImVec2(ImGui::GetItemRectSize().x - 60,
                       ImGui::GetItemsLineHeightWithSpacing() * 4);
    popupPos.y += ImGui::GetItemRectSize().y;
  }

  isFocused = ImGui::IsRootWindowFocused();

  ImGui::End();
}

float GCoeffWidth = 2.f;
float GCoeffHeight = 3.f;

//-----------------------------------------------------------
void RuleEditorWindow::DrawPopup(ImVec2 pos, ImVec2 /*size*/, bool& isFocused) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_HorizontalScrollbar |
                           ImGuiWindowFlags_NoSavedSettings;
  // ImGuiWindowFlags_ShowBorders;
  // ImGui::SetNextWindowFocus();
  ImGui::SetNextWindowPos(pos);

  ImGui::SetNextWindowSize(
      ImVec2(m_MaxTextWidth * GCoeffWidth, m_PopupHeight * GCoeffHeight));
  ImGui::Begin("input_popup", nullptr, flags);
  ImGui::PushAllowKeyboardFocus(false);

  for (uint32_t i = 0; i < m_AutoComplete.size(); i++) {
    // Track if we're drawing the active index so we
    // can scroll to it if it has changed
    bool isIndexActive = (size_t)m_State.m_ActiveIdx == i;

    if (isIndexActive) {
      // Draw the currently 'active' item differently
      // ( used appropriate colors for your own style )
      ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 0, 0, 1));
    }

    ImGui::PushID(i);
    if (ImGui::Selectable(m_AutoComplete[i].c_str(), isIndexActive)) {
      // An item was clicked, notify the input
      // callback so that it can modify the input buffer
      m_State.m_ClickedIdx = i;
      // NeedsRedraw();
    }
    ImGui::PopID();

    if (isIndexActive) {
      if (m_State.m_SelectionChanged) {
        // Make sure we bring the currently 'active' item into view.
        ImGui::SetScrollHere();
        m_State.m_SelectionChanged = false;
      }

      ImGui::PopStyleColor(1);
    }
  }

  isFocused = ImGui::IsRootWindowFocused();

  ImGui::PopAllowKeyboardFocus();
  ImGui::End();
  ImGui::PopStyleVar(1);
}

void RuleEditorWindow::UpdateTextBuffer() {
  m_TextBuffer.resize(m_Text.size() + 8192);
  memcpy(&m_TextBuffer[0], &m_Text[0], m_Text.size() + 1);
}

void RuleEditorWindow::Draw(const char* title, bool* p_opened, ImVec2* a_Size) {
  m_WindowFlags = 0;
  if (m_State.m_PopupOpen)
    m_WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

  if (a_Size) {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImVec2 CanvasSize = *a_Size;
    CanvasSize.x -= 20;
    CanvasSize.y -= 20;
    ImGui::SetNextWindowSize(CanvasSize, ImGuiCond_Always);
    ImGui::Begin(title, p_opened, CanvasSize, 1.f, m_WindowFlags);
  } else {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImVec2 size(400, 400);
    ImGui::Begin(title, p_opened, size, 1.f, m_WindowFlags);
  }

  if (m_Function) {
    ImGui::NewLine();
    std::string funcName = m_Function->PrettyName();
    ImGui::Text("When [%s] is called, send:", funcName.c_str());
  }

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::PopStyleVar();

  UpdateTextBuffer();

  ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
                              ImGuiInputTextFlags_CallbackAlways |
                              ImGuiInputTextFlags_CallbackCharFilter |
                              ImGuiInputTextFlags_CallbackCompletion |
                              ImGuiInputTextFlags_CallbackHistory;

  if (ImGui::InputText("", &m_TextBuffer[0], m_TextBuffer.size(), flags,
                       InputCallbackGlobal, this)) {
    ImGui::SetKeyboardFocusHere(-1);

    if (m_State.m_PopupOpen && m_State.m_ActiveIdx != -1) {
      // This means that enter was pressed whilst a
      // the popup was open and we had an 'active' item.
      // So we copy the entry to the input buffer here
      std::string entry = m_AutoComplete[m_State.m_ActiveIdx];

      ReplaceStringInPlace(entry, GetCurrentWord(m_Text), "");
      m_Text += entry;
      UpdateTextBuffer();
    }

    // Hide popup
    m_State.m_PopupOpen = false;
    m_State.m_ActiveIdx = -1;
  }

  bool popupFocused;

  // Based on Omar's developer console demo: Retain focus on the input box
  if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() &&
      !ImGui::IsMouseClicked(0)) {
    ImGui::SetKeyboardFocusHere(-1);
  }

  if (m_LastVariable) {
    const std::string& typeName = m_LastVariable->GetTypeName();
    ImGui::Text("Type: %s\nLength: %i\nOffset:%i", typeName.c_str(),
                (int)m_LastVariable->m_Size, (int)m_LastVariable->m_Address);
  }

  size_t numOptions = GPluginManager.m_Plugins.size() + Card::NUM_CARD_TYPES;

  if (m_PluginToggles.size() != numOptions) {
    m_PluginToggles.resize(numOptions, 0);
  }

  const std::vector<Orbit::Plugin*>& plugins = GPluginManager.m_Plugins;
  ImGui::Text("To plugin:");
  ImGui::SameLine();

  // Showing a menu with toggles
  if (ImGui::Button("Choose...")) {
    ImGui::OpenPopup("toggle");
  }

  std::map<int, std::string>& typeMap = Card::GetTypeMap();

  if (ImGui::BeginPopup("toggle")) {
    // Cards
    for (int i = 0; i < Card::NUM_CARD_TYPES; ++i) {
      ImGui::MenuItem(typeMap[i].c_str(), "", (bool*)&m_PluginToggles[i]);
    }

    ImGui::Separator();

    // Plugins
    for (size_t i = 0; i < plugins.size(); i++) {
      ImGui::MenuItem(plugins[i]->GetName(), "",
                      (bool*)&m_PluginToggles[Card::NUM_CARD_TYPES + i]);
    }

    ImGui::EndPopup();
  }

  for (uint32_t i = 0; i < m_PluginToggles.size(); ++i) {
    if (i < Card::NUM_CARD_TYPES) {
      if (m_PluginToggles[i]) {
        ImGui::SameLine();
        ImGui::Text("%s,", typeMap[i].c_str());
      }
    } else if (m_PluginToggles[i]) {
      ImGui::SameLine();
      ImGui::Text("%s,", plugins[i - Card::NUM_CARD_TYPES]->GetName());
    }
  }

  ImGui::NewLine();
  if (ImGui::Button("Create Rule")) {
    CreateRule();
  }

  ImGui::End();

  m_Text = (char*)&m_TextBuffer[0];

  RefreshAutoComplete(m_Text);

  if (m_State.m_PopupOpen) {
    // ImGui::SetNextWindowPos();
    DrawPopup(ImGui::GetCursorPos(), ImGui::GetWindowSize(), popupFocused);
  }

  // Restore focus to the input box if we just clicked an item
  if (m_State.m_ClickedIdx != -1) {
    ImGui::SetKeyboardFocusHere(-1);

    // NOTE: We do not reset the 'clickedIdx' here because
    // we want to let the callback handle it in order to
    // modify the buffer, therefore we simply restore keyboard input instead
    m_State.m_PopupOpen = false;
  }
}

//-----------------------------------------------------------------------------
RuleEditor::RuleEditor() : GlCanvas() {
  GOrbitApp->RegisterRuleEditor(this);
  GTcpServer->AddCallback(Msg_SavedContext, [=](const Message& a_Msg) {
    this->OnReceiveMessage(a_Msg);
  });
  m_Opened = true;
}

//-----------------------------------------------------------------------------
RuleEditor::~RuleEditor() {}

//-----------------------------------------------------------------------------
void RuleEditor::OnTimer() { GlCanvas::OnTimer(); }

//-----------------------------------------------------------------------------
void RuleEditor::ZoomAll() {}

//-----------------------------------------------------------------------------
void RuleEditor::KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                            bool a_Alt) {
  ScopeImguiContext state(m_ImGuiContext);

  if (!m_ImguiActive) {
    switch (a_KeyCode) {
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

  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void RuleEditor::RenderUI() {
  ScopeImguiContext state(m_ImGuiContext);
  Orbit_ImGui_NewFrame(this);
  m_Window.Draw("Rule Editor", &m_Opened);

  glViewport(0, 0, getWidth(), getHeight());

  // Rendering

  ImGui::Render();
}

//-----------------------------------------------------------------------------
void DemoPosition(const SavedContext32&, void* a_Data, int /*a_NumBytes*/) {
  float* pos = static_cast<float*>(a_Data);
  GCardContainer.Update("posX", pos[0]);
  GCardContainer.Update("posY", pos[1]);

  // BlackBoard::AddPos(pos[0], pos[1]);
  GCardContainer.Update("posZ", pos[2]);
}

bool GRedrawBlackBoard = false;

//-----------------------------------------------------------------------------
void RuleEditor::OnReceiveMessage(const Message& a_Message) {
  if (a_Message.GetType() == Msg_SavedContext) {
    GRedrawBlackBoard = true;
    m_NeedsRedraw = true;

    int ContextSize = Capture::GTargetProcess->GetIs64Bit()
                          ? sizeof(SavedContext64)
                          : sizeof(SavedContext32);
    void* argData = (void*)(a_Message.GetData() + ContextSize);

    DWORD64 address = a_Message.m_Header.m_GenericHeader.m_Address;

    std::shared_ptr<Rule> rule = m_Rules[address];
    int offset = 0;
    const char* maxAddress = a_Message.GetData() + a_Message.m_Size;
    for (const std::shared_ptr<Variable> var : rule->m_TrackedVariables) {
      char* data = (char*)argData + offset;
      if (data + var->m_Size > maxAddress) {
        break;
      }

      ProcessVariable(var, data);
      offset += var->m_Size;
    }
  }
}

//-----------------------------------------------------------------------------
void RuleEditor::ProcessVariable(const std::shared_ptr<Variable>& a_Variable,
                                 char* a_Data) {
  if (a_Variable->m_Size == 4) {
    GCardContainer.Update(a_Variable->m_Name, *((float*)a_Data));
  }
}

//-----------------------------------------------------------------------------
void ArgTracking() {
  if (Capture::GTargetProcess->GetIs64Bit()) {
    /*ORBIT_LOGV(context.m_Context.m_XMM0.m_RegFloat.m_F0);
    ORBIT_LOGV(context.m_EpilogContext.m_RAX.m_Reg32.Low);
    ORBIT_LOGV(context.m_EpilogContext.m_XMM0.m_RegFloat.m_F0);
*/
    /* Function * func = Capture::GTargetProcess->GetFunctionFromExactAddress(
     (void*)context.m_Context.m_RET.m_Reg64 );
     GCardContainer.Update(func->m_Name,
     context.m_EpilogContext.m_XMM0.m_RegFloat.m_F0);*/
  } else {
    // SavedContext32 & context = *(SavedContext32*)( a_Message.GetData() );
    /*ORBIT_LOGV(context.m_Context.m_XMM0.m_RegFloat.m_F0);
    ORBIT_LOGV(context.m_EpilogContext.m_EAX);
    ORBIT_LOGV(context.m_EpilogContext.m_XMM0.m_RegFloat.m_F0);

    Function * func = Capture::GTargetProcess->GetFunctionFromExactAddress(
    (void*)context.m_Context.m_RET ); GCardContainer.Update( func->m_Name,
    (float)*(reinterpret_cast<int*>(&context.m_EpilogContext.m_EAX))/100.f );*/
  }
}
