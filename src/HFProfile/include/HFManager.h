// HFManager is going to have a couple of jobs in the future, but at the moment
// We're just going to keep it in charge of the call stack.

#ifndef HFTEST_MANAGER_H_
#define HFTEST_MANAGER_H

#include "HFStack.h"
#include <stack>

namespace HFManager {
 void AddStackFrame(std::shared_ptr<HFStack>) { stackframes.push(stack); }
 [[nodiscard]] std::shared_ptr<HFStack> PopStackFrame() { return stackframes.pop(); }
 [[nodiscard]] int GetTotalStacks() { return stackframes.size(); }
 
 // Deallocate all of our frames
 void ClearStackFrames() {
   while (!stackframes.empty()) {
     stack.pop();
   }
 }

 namespace {
   std::stack<std::shared_ptr<HFStack>> stackframes;
 }
}

#endif // HFTEST_MANAGER_H_