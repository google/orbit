//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <unordered_map>

#include "GlCanvas.h"
#include "ImGuiOrbit.h"
#include "OrbitRule.h"

class Function;
class OrbitType;

struct State {
  State()
      : m_PopupOpen(false),
        m_ActiveIdx(-1),
        m_ClickedIdx(-1),
        m_SelectionChanged(false),
        m_Selected(false) {}

  bool m_PopupOpen;
  int m_ActiveIdx;
  int m_ClickedIdx;
  bool m_SelectionChanged;
  bool m_Selected;
};

class RuleEditorWindow {
 public:
  RuleEditorWindow(Function* a_Function = nullptr);
  void Draw(const char* title, bool* p_opened = nullptr,
            struct ImVec2* a_Size = nullptr);
  int InputCallback(ImGuiTextEditCallbackData* data);
  void SetInputFromActiveIndex(ImGuiTextEditCallbackData* data, int entryIndex);
  void OnWordSelected(const std::string& a_Word);
  void Launch(Function* a_Function);

 protected:
  void DrawPopup(ImVec2 pos, ImVec2 size, bool& isFocused);
  std::shared_ptr<Variable> GetLastVariable(const std::string& a_Chain);
  std::string GetCurrentWord(const std::string a_Chain);
  void RefreshAutoComplete(const std::string& a_Line);
  bool CreateRule();
  void UpdateTextBuffer();

 protected:
  ImGuiWindowFlags m_WindowFlags;
  Function* m_Function;
  Type* m_Type;
  std::shared_ptr<Variable> m_LastVariable;
  std::string m_Text;
  std::string m_LastText;
  std::vector<char> m_TextBuffer;
  std::vector<std::string> m_AutoComplete;
  std::vector<int> m_PluginToggles;
  ImVec2 m_PopupPos;
  float m_MaxTextWidth;
  float m_PopupHeight;
  State m_State;
};

class RuleEditor : public GlCanvas {
 public:
  RuleEditor();
  ~RuleEditor() override;

  void OnReceiveMessage(const Message& a_Message);
  std::unordered_map<DWORD64, std::shared_ptr<Rule>>& GetRules() {
    return m_Rules;
  }

  void OnTimer() override;
  void ZoomAll();
  void KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                  bool a_Alt) override;
  void RenderUI() override;
  void ProcessVariable(const std::shared_ptr<Variable>& a_Variable,
                       char* a_Data);

  RuleEditorWindow m_Window;
  std::unordered_map<DWORD64, std::shared_ptr<Rule>> m_Rules;
  bool m_Opened;
};
