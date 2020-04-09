//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitType.h"
#include "ProcessUtils.h"

class ModulesDataView : public DataView {
 public:
  ModulesDataView();

  const std::vector<std::string>& GetColumnHeaders() override;
  const std::vector<float>& GetColumnHeadersRatios() override;
  const std::vector<SortingOrder>& GetColumnInitialOrders() override;
  int GetDefaultSortingColumn() override;
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;

  void OnFilter(const std::string& a_Filter) override;
  void OnSort(int a_Column, std::optional<SortingOrder> a_NewOrder) override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnTimer() override;
  bool WantsDisplayColor() override { return true; }
  bool GetDisplayColor(int /*a_Row*/, int /*a_Column*/, unsigned char& /*r*/,
                       unsigned char& /*g*/, unsigned char& /*b*/) override;
  std::string GetLabel() override { return "Modules"; }

  void SetProcess(std::shared_ptr<Process> a_Process);

  enum MdvColumn {
    MDV_Index,
    MDV_ModuleName,
    MDV_Path,
    MDV_AddressRange,
    MDV_HasPdb,
    MDV_PdbSize,
    MDV_Loaded,
    MDV_NumColumns
  };

 protected:
  const std::shared_ptr<Module>& GetModule(unsigned int a_Row) const;

  std::shared_ptr<Process> m_Process;
  std::vector<std::shared_ptr<Module> > m_Modules;

  static void InitColumnsIfNeeded();
  static std::vector<std::string> s_Headers;
  static std::vector<float> s_HeaderRatios;
  static std::vector<SortingOrder> s_InitialOrders;

  static const std::string MENU_ACTION_MODULES_LOAD;
  static const std::string MENU_ACTION_DLL_FIND_PDB;
  static const std::string MENU_ACTION_DLL_EXPORTS;
};
