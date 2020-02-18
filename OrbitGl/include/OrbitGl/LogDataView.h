//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitCore/Message.h"
#include "OrbitCore/Threading.h"

struct CallStack;

//-----------------------------------------------------------------------------
class LogDataView : public DataView {
 public:
  LogDataView();
  virtual const std::vector<std::wstring>& GetColumnHeaders() override;
  const std::vector<float>& GetColumnHeadersRatios() override;
  virtual std::vector<std::wstring> GetContextMenu(int a_Index) override;
  virtual std::wstring GetValue(int a_Row, int a_Column) override;
  virtual std::wstring GetToolTip(int a_Row, int a_Column) override;
  virtual bool ScrollToBottom() override;
  virtual bool SkipTimer() override;

  void OnDataChanged() override;
  void OnFilter(const std::wstring& a_Filter) override;
  void OnContextMenu(const std::wstring& a_Action, int a_MenuIndex,
                     std::vector<int>& a_ItemIndices) override;

  void Add(const OrbitLogEntry& a_Msg);
  const OrbitLogEntry& GetEntry(unsigned int a_Row) const;
  void OnReceiveMessage(const Message& a_Msg);

  enum OdvColumn { LDV_Message, LDV_Time, LDV_ThreadId, LDV_NumColumns };

 protected:
  std::vector<OrbitLogEntry> m_Entries;
  Mutex m_Mutex;
  std::shared_ptr<CallStack> m_SelectedCallstack;
  static std::vector<float> s_HeaderRatios;
};
