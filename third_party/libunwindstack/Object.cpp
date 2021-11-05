/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unwindstack/MapInfo.h>
#include <unwindstack/Object.h>

namespace unwindstack {

bool Object::cache_enabled_;
std::unordered_map<std::string, std::pair<std::shared_ptr<Object>, bool>>* Object::cache_;
std::mutex* Object::cache_lock_;

void Object::SetCachingEnabled(bool enable) {
  if (!cache_enabled_ && enable) {
    cache_enabled_ = true;
    cache_ = new std::unordered_map<std::string, std::pair<std::shared_ptr<Object>, bool>>;
    cache_lock_ = new std::mutex;
  } else if (cache_enabled_ && !enable) {
    cache_enabled_ = false;
    delete cache_;
    delete cache_lock_;
  }
}

void Object::CacheLock() {
  cache_lock_->lock();
}

void Object::CacheUnlock() {
  cache_lock_->unlock();
}

void Object::CacheAdd(MapInfo* info) {
  // If object_offset != 0, then cache both name:offset and name.
  // The cached name is used to do lookups if multiple maps for the same
  // named object file exist.
  // For example, if there are two maps boot.odex:1000 and boot.odex:2000
  // where each reference the entire boot.odex, the cache will properly
  // use the same cached object.

  if (info->offset() == 0 || info->object_offset() != 0) {
    (*cache_)[info->name()] = std::make_pair(info->object(), true);
  }

  if (info->offset() != 0) {
    // The second element in the pair indicates whether object_offset should
    // be set to offset when getting out of the cache.
    std::string key = std::string(info->name()) + ':' + std::to_string(info->offset());
    (*cache_)[key] = std::make_pair(info->object(), info->object_offset() != 0);
  }
}

bool Object::CacheAfterCreateMemory(MapInfo* info) {
  if (info->name().empty() || info->offset() == 0 || info->object_offset() == 0) {
    return false;
  }

  auto entry = cache_->find(info->name());
  if (entry == cache_->end()) {
    return false;
  }

  // In this case, the whole file is the object, and the name has already
  // been cached. Add an entry at name:offset to get this directly out
  // of the cache next time.
  info->set_object(entry->second.first);
  std::string key = std::string(info->name()) + ':' + std::to_string(info->offset());
  (*cache_)[key] = std::make_pair(info->object(), true);
  return true;
}

bool Object::CacheGet(MapInfo* info) {
  std::string name(info->name());
  if (info->offset() != 0) {
    name += ':' + std::to_string(info->offset());
  }
  auto entry = cache_->find(name);
  if (entry != cache_->end()) {
    info->set_object(entry->second.first);
    if (entry->second.second) {
      info->set_object_offset(info->offset());
    }
    return true;
  }
  return false;
}

}  // namespace unwindstack
