//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CallStackDataView.h"
#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "Core.h"
#include "OrbitProcess.h"
#include "Pdb.h"
#include "SamplingProfiler.h"

//----------------------------------------------------------------------------
CallStackDataView::CallStackDataView() : m_CallStack(nullptr) {}

//-----------------------------------------------------------------------------
void CallStackDataView::SetAsMainInstance() {
  GOrbitApp->RegisterCallStackDataView(this);
}

//-----------------------------------------------------------------------------
size_t CallStackDataView::GetNumElements() { return m_Indices.size(); }

//-----------------------------------------------------------------------------
void CallStackDataView::OnDataChanged() {
  size_t numFunctions = m_CallStack ? m_CallStack->m_Depth : 0;
  m_Indices.resize(numFunctions);
  for (uint32_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }
}

//-----------------------------------------------------------------------------
std::wstring CallStackDataView::GetValue(int a_Row, int a_Column) {
  if (a_Row >= (int)GetNumElements()) {
    return L"";
  }

  Function& function = GetFunction(a_Row);

  std::string value;

  switch (s_HeaderMap[a_Column]) {
    case Function::INDEX:
      value = Format("%d", a_Row);
      break;
    case Function::SELECTED:
      value = function.IsSelected() ? "X" : "-";
      break;
    case Function::NAME:
      value = function.PrettyName();
      break;
    case Function::ADDRESS:
      value = Format("0x%llx", function.GetVirtualAddress());
      break;
    case Function::FILE:
      value = function.m_File;
      break;
    case Function::MODULE:
      value = ws2s(function.GetModuleName());
      break;
    case Function::LINE:
      value = Format("%i", function.m_Line);
      break;
    case Function::SIZE:
      value = Format("%lu", function.m_Size);
      break;
    case Function::CALL_CONV:
      value = ws2s(Function::GetCallingConventionString(function.m_CallConv));
      break;
    default:
      break;
  }

  return s2ws(value);
}

//-----------------------------------------------------------------------------
void CallStackDataView::OnFilter(const std::wstring& a_Filter) {
  if (!m_CallStack) return;

  std::vector<uint32_t> indices;
  std::vector<std::wstring> tokens = Tokenize(ToLower(a_Filter));

  for (int i = 0; i < (int)m_CallStack->m_Depth; ++i) {
    const Function& function = GetFunction(i);
    std::wstring name = ToLower(s2ws(function.m_PrettyName));
    bool match = true;

    for (std::wstring& filterToken : tokens) {
      if( !( name.find( filterToken ) != std::wstring::npos/* ||
                   file.find( filterToken ) != std::string::npos*/ ) )
            {
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
Function& CallStackDataView::GetFunction(unsigned int a_Row) {
  static Function dummy;

  if (m_CallStack) {
    if (a_Row < m_CallStack->m_Depth) {
      ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

      DWORD64 addr = m_CallStack->m_Data[a_Row];
      Function* func =
          Capture::GTargetProcess->GetFunctionFromAddress(addr, false);

      if (func) {
        return *func;
      } else if (Capture::GSamplingProfiler) {
        dummy.m_PrettyName =
            ws2s(Capture::GSamplingProfiler->GetSymbolFromAddress(addr));
        dummy.m_Address = addr;
        return dummy;
      }
    }
  }

  dummy.m_PrettyName = "";
  return dummy;
}