#ifndef ORBIT_CORE_STRING_MANAGER_H_
#define ORBIT_CORE_STRING_MANAGER_H_

#include <mutex>
#include <optional>
#include <string>

#include "absl/container/flat_hash_map.h"

class StringManager {
 public:
  StringManager() = default;

  void Add(uint64_t key, std::string_view str);
  std::optional<std::string> Get(uint64_t key);
  bool Exists(uint64_t key);
  void Clear();

 private:
  absl::flat_hash_map<uint64_t, std::string> key_to_string_;
  std::mutex mutex_;
};

#endif  // ORBIT_CORE_STRING_MANAGER_H_
