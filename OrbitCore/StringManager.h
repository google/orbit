#ifndef ORBIT_CORE_STRING_MANAGER_H_
#define ORBIT_CORE_STRING_MANAGER_H_

#include <optional>
#include <string>

#include "SerializationMacros.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

class StringManager {
 public:
  StringManager() = default;

  bool AddIfNotPresent(uint64_t key, std::string_view str);
  std::optional<std::string> Get(uint64_t key);
  bool Contains(uint64_t key);
  void Clear();

  ORBIT_SERIALIZABLE;

 private:
  absl::flat_hash_map<uint64_t, std::string> key_to_string_;
  absl::Mutex mutex_;
};

#endif  // ORBIT_CORE_STRING_MANAGER_H_
