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

#include <sys/mman.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Object.h>

#include <android-base/stringprintf.h>

namespace unwindstack {

bool Object::cache_enabled_;
std::unordered_map<std::string, std::unordered_map<uint64_t, std::shared_ptr<Object>>>*
    Object::cache_;
std::mutex* Object::cache_lock_;

void Object::SetCachingEnabled(bool enable) {
  if (!cache_enabled_ && enable) {
    cache_enabled_ = true;
    cache_ =
        new std::unordered_map<std::string, std::unordered_map<uint64_t, std::shared_ptr<Object>>>;
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
  if (!info->object()->valid()) {
    return;
  }
  (*cache_)[std::string(info->name())].emplace(info->object_start_offset(), info->object());
}

bool Object::CacheGet(MapInfo* info) {
  auto name_entry = cache_->find(std::string(info->name()));
  if (name_entry == cache_->end()) {
    return false;
  }
  // First look to see if there is a zero offset entry, this indicates
  // the whole object is the file.
  auto& offset_cache = name_entry->second;
  uint64_t object_start_offset = 0;
  auto entry = offset_cache.find(object_start_offset);
  if (entry == offset_cache.end()) {
    // Try and find using the current offset.
    object_start_offset = info->offset();
    entry = offset_cache.find(object_start_offset);
    if (entry == offset_cache.end()) {
      // If this is an execute map, then see if the previous read-only
      // map is the start of the object.
      if (!(info->flags() & PROT_EXEC)) {
        return false;
      }
      auto prev_map = info->GetPrevRealMap();
      if (prev_map == nullptr || info->offset() <= prev_map->offset() ||
          (prev_map->flags() != PROT_READ)) {
        return false;
      }
      object_start_offset = prev_map->offset();
      entry = offset_cache.find(object_start_offset);
      if (entry == offset_cache.end()) {
        return false;
      }
    }
  }

  info->set_object(entry->second);
  info->set_object_start_offset(object_start_offset);
  info->set_object_offset(info->offset() - object_start_offset);
  return true;
}

std::string Object::GetPrintableBuildID(std::string& build_id) {
  if (build_id.empty()) {
    return "";
  }
  std::string printable_build_id;
  for (const char& c : build_id) {
    // Use %hhx to avoid sign extension on abis that have signed chars.
    printable_build_id += android::base::StringPrintf("%02hhx", c);
  }
  return printable_build_id;
}

std::string Object::GetPrintableBuildID() {
  std::string build_id = GetBuildID();
  return Elf::GetPrintableBuildID(build_id);
}

}  // namespace unwindstack
