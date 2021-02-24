// HFTest file written & modified - 02/23/2023

#include "HFManager.h"

namespace HFManager {
 void AddStackFrame(std::shared_ptr<HFStack>) { stackframes.push(stack); }
 [[nodiscard]] std::shared_ptr<HFStack> PopStackFrame() { return stackframes.pop(); }
 [[nodiscard]] int GetTotalStacks() const { return stackframes.size(); }
 
 // Deallocate all of our frames
 void ClearStackFrames() {
   while (!stackframes.empty()) {
     stack.pop();
   }
 }

 namespace {
   std::stack<std::shared_ptr<HFStack>> stackframes;
 } // namespace

} // namespace HFManager