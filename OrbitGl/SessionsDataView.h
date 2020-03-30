//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>

#include "DataView.h"

class Session;

class SessionsDataView : public DataView {
 public:
  SessionsDataView();
  const std::vector<std::wstring>& GetColumnHeaders() override;
  const std::vector<float>& GetColumnHeadersRatios() override;
  const std::vector<SortingOrder>& GetColumnInitialOrders() override;
  std::vector<std::wstring> GetContextMenu(int a_Index) override;
  std::wstring GetValue(int a_Row, int a_Column) override;
  std::wstring GetToolTip(int a_Row, int a_Column) override;
  std::wstring GetLabel() override { return L"Sessions"; }

  void OnDataChanged() override;
  void OnFilter(const std::wstring& a_Filter) override;
  void OnSort(int a_Column, std::optional<SortingOrder> a_NewOrder) override;
  void OnContextMenu(const std::wstring& a_Action, int a_MenuIndex,
                     std::vector<int>& a_ItemIndices) override;

  void SetSessions(const std::vector<std::shared_ptr<Session> >& a_Sessions);

  enum SdvColumn {
    SDV_SessionName,
    SDV_ProcessName,
    // SDV_LastUsed,
    SDV_NumColumns
  };

 protected:
  const std::shared_ptr<Session>& GetSession(unsigned int a_Row) const;

  std::vector<std::shared_ptr<Session> > m_Sessions;

  static void InitColumnsIfNeeded();
  static std::vector<std::wstring> s_Headers;
  static std::vector<float> s_HeaderRatios;
  static std::vector<SortingOrder> s_InitialOrders;
};
