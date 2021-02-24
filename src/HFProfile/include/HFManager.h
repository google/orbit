// HFManager is going to have a couple of jobs in the future, but at the moment
// We're just going to keep it in charge of the call stack.
// HFTest file written & modified - 02/23/2023

#ifndef HFTEST_MANAGER_H_
#define HFTEST_MANAGER_H

#include "HFStack.h"
#include <stack>

namespace HFManager {

 void AddStackFrame(std::shared_ptr<HFStack>);
 [[nodiscard]] std::shared_ptr<HFStack> PopStackFrame();
 [[nodiscard]] int GetTotalStacks() const;
 void ClearStackFrames();

} // namespace HFManager

#endif // HFTEST_MANAGER_H_