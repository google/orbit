//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CallStackDataView.h"

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "Core.h"
#include "OrbitProcess.h"
#include "SamplingProfiler.h"
#include "absl/strings/str_format.h"

//----------------------------------------------------------------------------
CallStackDataView::CallStackDataView() : m_CallStack(nullptr) {}

//-----------------------------------------------------------------------------
void CallStackDataView::SetAsMainInstance() {
  GOrbitApp->RegisterCallStackDataView(this);
}

//-----------------------------------------------------------------------------
size_t CallStackDataView::GetNumElements() { return m_Indices.size(); }

//-----------------------------------------------------------------------------
std::string CallStackDataView::GetValue(int a_Row, int a_Column) {
  if (a_Row >= (int)GetNumElements()) {
    return "";
  }

  Function& function = GetFunctionOrDummy(m_Indices[a_Row]);

  std::string value;

  switch (s_HeaderMap[a_Column]) {
    case Function::INDEX:
      value = absl::StrFormat("%d", a_Row);
      break;
    case Function::SELECTED:
      value = function.IsSelected() ? "X" : "-";
      break;
    case Function::NAME:
      value = function.PrettyName();
      break;
    case Function::ADDRESS:
      value = absl::StrFormat("0x%llx", function.GetVirtualAddress());
      break;
    case Function::FILE:
      value = function.File();
      break;
    case Function::MODULE:
      value = function.GetModuleName();
      break;
    case Function::LINE:
      value = absl::StrFormat("%i", function.Line());
      break;
    case Function::SIZE:
      value = absl::StrFormat("%lu", function.Size());
      break;
    case Function::CALL_CONV:
      value = function.GetCallingConventionString();
      break;
    default:
      break;
  }

  return value;
}

//-----------------------------------------------------------------------------
void CallStackDataView::OnFilter(const std::string& a_Filter) {
  if (!m_CallStack) return;

  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = Tokenize(ToLower(a_Filter));

  for (int i = 0; i < (int)m_CallStack->m_Depth; ++i) {
    const Function& function = GetFunctionOrDummy(i);
    std::string name = ToLower(function.PrettyName());
    bool match = true;

    for (std::string& filterToken : tokens) {
      if (name.find(filterToken) == std::string::npos) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  m_Indices = indices;
}

//-----------------------------------------------------------------------------
void CallStackDataView::OnDataChanged() {
  size_t numFunctions = m_CallStack ? m_CallStack->m_Depth : 0;
  m_Indices.resize(numFunctions);
  for (size_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }
}

//-----------------------------------------------------------------------------
Function* CallStackDataView::GetFunction(int a_Row) {
  int index = m_Indices[a_Row];
  if (m_CallStack != nullptr && index < static_cast<int>(m_CallStack->m_Depth) &&
      Capture::GTargetProcess != nullptr) {
    ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

    uint64_t address = m_CallStack->m_Data[index];
    Function* function =
        Capture::GTargetProcess->GetFunctionFromAddress(address, false);
    return function;
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
Function& CallStackDataView::GetFunctionOrDummy(int a_IndexInCallstack) {
  Function* function = GetFunction(a_IndexInCallstack);
  if (function != nullptr) {
    return *function;
  }

  static Function dummy;
  if (m_CallStack != nullptr && a_IndexInCallstack < static_cast<int>(m_CallStack->m_Depth) &&
      Capture::GSamplingProfiler != nullptr) {
    uint64_t address = m_CallStack->m_Data[a_IndexInCallstack];
    dummy.SetPrettyName(
        ws2s(Capture::GSamplingProfiler->GetSymbolFromAddress(address)));
    dummy.SetAddress(address);
  }
  return dummy;
}
