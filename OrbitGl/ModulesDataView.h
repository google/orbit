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

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_PDB_SIZE; }
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;

  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnTimer() override;
  bool WantsDisplayColor() override { return true; }
  bool GetDisplayColor(int /*a_Row*/, int /*a_Column*/, unsigned char& /*r*/,
                       unsigned char& /*g*/, unsigned char& /*b*/) override;
  std::string GetLabel() override { return "Modules"; }

  void SetProcess(const std::shared_ptr<Process>& a_Process);

 protected:
  void DoSort() override;
  void DoFilter() override;
  const std::shared_ptr<Module>& GetModule(unsigned int a_Row) const;

  std::shared_ptr<Process> m_Process;
  std::vector<std::shared_ptr<Module> > m_Modules;

  enum ColumnIndex {
    COLUMN_INDEX,
    COLUMN_NAME,
    COLUMN_PATH,
    COLUMN_ADDRESS_RANGE,
    COLUMN_HAS_PDB,
    COLUMN_PDB_SIZE,
    COLUMN_LOADED,
    COLUMN_NUM
  };

  static const std::string MENU_ACTION_MODULES_LOAD;
  static const std::string MENU_ACTION_DLL_FIND_PDB;
  static const std::string MENU_ACTION_DLL_EXPORTS;
};
