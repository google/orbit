//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <utility>

#include "DataView.h"
#include "OrbitModule.h"
#include "OrbitType.h"

struct CallStack;

class CallStackDataView : public DataView {
 public:
  CallStackDataView();

  void SetAsMainInstance() override;
  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_ADDRESS; }
  bool IsSortingAllowed() override { return false; }
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;

  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnDataChanged() override;
  void SetCallStack(std::shared_ptr<CallStack> a_CallStack) {
    m_CallStack = std::move(a_CallStack);
    OnDataChanged();
  }

 protected:
  void DoFilter() override;

  std::shared_ptr<CallStack> m_CallStack;

  struct CallStackDataViewFrame {
    CallStackDataViewFrame() = default;
    CallStackDataViewFrame(uint64_t address, Function* function,
                           std::shared_ptr<Module> module)
        : address(address), function(function), module(std::move(module)) {}
    CallStackDataViewFrame(uint64_t address, std::string fallback_name,
                           std::shared_ptr<Module> module)
        : address(address),
          fallback_name(std::move(fallback_name)),
          module(std::move(module)) {}

    uint64_t address = 0;
    Function* function = nullptr;
    std::string fallback_name;
    std::shared_ptr<Module> module;
  };

  CallStackDataViewFrame GetFrameFromRow(int row);
  CallStackDataViewFrame GetFrameFromIndex(int index_in_callstack);

  enum ColumnIndex {
    COLUMN_SELECTED,
    COLUMN_INDEX,
    COLUMN_NAME,
    COLUMN_SIZE,
    COLUMN_FILE,
    COLUMN_LINE,
    COLUMN_MODULE,
    COLUMN_ADDRESS,
    COLUMN_NUM
  };

  static const std::string MENU_ACTION_MODULES_LOAD;
  static const std::string MENU_ACTION_SELECT;
  static const std::string MENU_ACTION_UNSELECT;
  static const std::string MENU_ACTION_VIEW;
  static const std::string MENU_ACTION_DISASSEMBLY;
};
