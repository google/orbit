#include "LogDataView.h"

#include <chrono>

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "SamplingProfiler.h"
#include "TcpServer.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
LogDataView::LogDataView() : DataView(DataViewType::LOG) {
  GOrbitApp->RegisterOutputLog(this);
  GTcpServer->AddCallback(Msg_OrbitLog, [=](const Message& a_Msg) {
    this->OnReceiveMessage(a_Msg);
  });
  m_UpdatePeriodMs = 100;
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& LogDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_MESSAGE] = {"Log", .7f, SortingOrder::Ascending};
    columns[COLUMN_TIME] = {"Time", .15f, SortingOrder::Descending};
    columns[COLUMN_THREAD_ID] = {"ThreadId", .15f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string LogDataView::GetValue(int a_Row, int a_Column) {
  const OrbitLogEntry& entry = GetEntry(a_Row);
  std::string value;

  switch (a_Column) {
    case COLUMN_MESSAGE:
      return entry.m_Text;

    case COLUMN_TIME: {
      TickType micros = (TickType)MicroSecondsFromTicks(
          Capture::GCaptureTimer.m_Start, entry.m_Time);
      std::chrono::system_clock::time_point sysTime =
          Capture::GCaptureTimePoint + std::chrono::microseconds(micros);
      std::time_t now_c = std::chrono::system_clock::to_time_t(sysTime);
      std::tm now_tm;
#ifdef WIN32
      localtime_s(&now_tm, &now_c);
#else
      now_tm = *std::localtime(&now_c);
#endif
      char buffer[256];
      strftime(buffer, sizeof(buffer), "%H:%M:%S", &now_tm);
      return buffer;
    }

    case COLUMN_THREAD_ID:
      return absl::StrFormat("%u", entry.m_ThreadId);
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
std::string LogDataView::GetToolTip(int /*row*/, int /*column*/) { return ""; }

//-----------------------------------------------------------------------------
bool LogDataView::ScrollToBottom() { return true; }

//-----------------------------------------------------------------------------
bool LogDataView::SkipTimer() { return !Capture::IsCapturing(); }

//-----------------------------------------------------------------------------
void LogDataView::OnDataChanged() {
  ScopeLock lock(m_Mutex);
  m_Indices.resize(m_Entries.size());
  for (size_t i = 0; i < m_Entries.size(); ++i) {
    m_Indices[i] = i;
  }

  DataView::OnDataChanged();
}

//-----------------------------------------------------------------------------
void LogDataView::DoFilter() {
  std::vector<std::string> tokens = Tokenize(ToLower(m_Filter));
  std::vector<uint32_t> indices;

  for (size_t i = 0; i < m_Entries.size(); ++i) {
    const OrbitLogEntry& entry = m_Entries[i];
    std::string text = ToLower(entry.m_Text);

    bool match = true;

    for (std::string& filterToken : tokens) {
      if (text.find(filterToken) == std::string::npos) {
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
std::vector<std::string> LogDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  const OrbitLogEntry& entry = LogDataView::GetEntry(a_ClickedIndex);
  m_SelectedCallstack = Capture::GetCallstack(entry.m_CallstackHash);
  std::vector<std::string> menu;
  if (m_SelectedCallstack) {
    for (uint32_t i = 0; i < m_SelectedCallstack->m_Depth; ++i) {
      uint64_t addr = m_SelectedCallstack->m_Data[i];
      menu.push_back(Capture::GSamplingProfiler->GetSymbolFromAddress(addr));
    }
  }
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void LogDataView::OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                                const std::vector<int>& a_ItemIndices) {
  if (m_SelectedCallstack && (int)m_SelectedCallstack->m_Depth > a_MenuIndex) {
    GOrbitApp->GoToCode(m_SelectedCallstack->m_Data[a_MenuIndex]);
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void LogDataView::Add(const OrbitLogEntry& a_Msg) {
  ScopeLock lock(m_Mutex);
  m_Entries.push_back(a_Msg);
  OnDataChanged();
}

//-----------------------------------------------------------------------------
const OrbitLogEntry& LogDataView::GetEntry(unsigned int a_Row) const {
  return m_Entries[m_Indices[a_Row]];
}

//-----------------------------------------------------------------------------
void LogDataView::OnReceiveMessage(const Message& a_Msg) {
  bool isLog = a_Msg.GetType() == Msg_OrbitLog;
  assert(isLog);
  if (isLog) {
    OrbitLogEntry entry;
    const OrbitLogEntry* msgEntry = (OrbitLogEntry*)(a_Msg.GetData());
    entry.m_Time = msgEntry->m_Time;
    entry.m_CallstackHash = msgEntry->m_CallstackHash;
    entry.m_ThreadId = msgEntry->m_ThreadId;
    const char* log = a_Msg.GetData() + OrbitLogEntry::GetSizeWithoutString();
    entry.m_Text = log;
    RemoveTrailingNewLine(entry.m_Text);
    Add(entry);
  }
}
