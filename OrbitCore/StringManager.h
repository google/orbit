#ifndef ORBIT_CORE_STRING_MANAGER_H_
#define ORBIT_CORE_STRING_MANAGER_H_

#include <mutex>
#include <string>
#include "absl/container/flat_hash_map.h"

class StringManager {
 public:
  StringManager();
  ~StringManager();

  void Add(uint64_t key, const std::string_view str);
  std::string Get(uint64_t key);
  bool Exists(uint64_t key);
private:
  absl::flat_hash_map<uint64_t, std::string> key_to_string_;
  std::mutex mutex_;
};

extern std::unique_ptr<StringManager> GStringManager;

#endif
