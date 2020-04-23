//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "Message.h"
#include "Threading.h"

struct CallStack;

//-----------------------------------------------------------------------------
class LogDataView : public DataView {
 public:
  LogDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_TIME; }
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;
  std::string GetToolTip(int a_Row, int a_Column) override;
  bool ScrollToBottom() override;
  bool SkipTimer() override;

  void OnDataChanged() override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;

  void Add(const OrbitLogEntry& a_Msg);
  void OnReceiveMessage(const Message& a_Msg);

 protected:
  // TODO: DoSort() override;
  void DoFilter() override;
  const OrbitLogEntry& GetEntry(unsigned int a_Row) const;

  std::vector<OrbitLogEntry> m_Entries;
  Mutex m_Mutex;
  std::shared_ptr<CallStack> m_SelectedCallstack;

  enum ColumnIndex {
    COLUMN_MESSAGE,
    COLUMN_TIME,
    COLUMN_THREAD_ID,
    COLUMN_NUM
  };
};
