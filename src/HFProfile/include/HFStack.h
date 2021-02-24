// Keeps a miniature, more lightweight callstack that is populated
// and emptied between the detection of a particular function signature
// HFTest file written & modified - 02/23/2023

#ifndef HFTEST_CALLSTACK_DATA_H_
#define HFTEST_CALLSTACK_DATA_H_

#include "OrbitClientData/FunctionUtils.h"

class HFStack {
 public:
  HFStack(std::string triggerName) : _triggerName(triggerName) {}
  
  // Use function utils for our lookup table to handle possible function overloading
  bool AddFunctionInfo(const orbit_client_protos::FunctionInfo& func);
  [[nodiscard]] orbit_client_protos::FunctionInfo& GetFunctionInfo(std::string& name) const;

 private:
  std::string _triggerName;
  absl::flat_hash_map<std::string, orbit_client_protos::FunctionInfo&> _lookupTable;
  bool _locked;
}

#endif // HFTEST_CALLSTACK_DATA_H_