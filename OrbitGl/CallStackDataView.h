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
  void OnDataChanged() override;
  std::string GetValue(int a_Row, int a_Column) override;
  void OnFilter(const std::string& a_Filter) override;
  void SetCallStack(std::shared_ptr<CallStack> a_CallStack) {
    m_CallStack = std::move(a_CallStack);
    OnDataChanged();
  }

 protected:
  Function& GetFunction(unsigned int a_Row) override;

  std::shared_ptr<CallStack> m_CallStack;
};
