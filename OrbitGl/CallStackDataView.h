//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <utility>

#include "FunctionsDataView.h"
#include "OrbitType.h"

struct CallStack;

//-----------------------------------------------------------------------------
class CallStackDataView : public FunctionsDataView {
 public:
  CallStackDataView();
  bool IsSortingAllowed() override { return false; }
  void SetAsMainInstance() override;
  size_t GetNumElements() override;
  std::string GetValue(int a_Row, int a_Column) override;
  void OnFilter(const std::string& a_Filter) override;
  void OnDataChanged() override;
  void SetCallStack(std::shared_ptr<CallStack> a_CallStack) {
    m_CallStack = std::move(a_CallStack);
    OnDataChanged();
  }

 protected:
  Function* GetFunction(int a_Row) override;
  Function& GetFunctionOrDummy(int a_Row);

  std::shared_ptr<CallStack> m_CallStack;
};
