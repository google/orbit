#include "HFStack/HFStack.h"

bool HFStack::AddFunctionInfo(const orbit_client_protos::FunctionInfo& func) {
  if (_locked) return false;
  std::string name(function_utils::GetDisplayName(func));
  if (name == _triggerName) {
    _locked = true;
    return false;
  }
  _lookupTable.try_emplace(name, func);
}

[[nodiscard]] HFStack::orbit_client_protos::FunctionInfo& GetFunctionInfo(std::string& name) const { 
  auto it = _lookupTable.find(name);
  if (it != _lookupTable.end()) {
    return it->second;
  }
  return nullptr;
}