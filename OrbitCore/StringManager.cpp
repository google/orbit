#include "StringManager.h"

std::unique_ptr<StringManager> GStringManager;

StringManager::StringManager() {}
StringManager::~StringManager() {}

void StringManager::Add(uint64_t key, const std::string& str) {
  if (key_to_string_.count(key) == 0) {
    key_to_string_.emplace(key, str);
  }
}

std::string StringManager::Get(uint64_t key) {
  auto it = key_to_string_.find(key);
  if (it != key_to_string_.end()) {
    return it->second;
  } else {
    return "";
  }
}

bool StringManager::Exists(uint64_t key) {
  return key_to_string_.count(key) > 0;
}
