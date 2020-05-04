// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_PROCESS_MEMORY_CLIENT_H_
#define ORBIT_CORE_PROCESS_MEMORY_CLIENT_H_

#include "TransactionClient.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

class ProcessMemoryClient {
 public:
  explicit ProcessMemoryClient(TransactionClient* transaction_client);

  ProcessMemoryClient() = delete;
  ProcessMemoryClient(const ProcessMemoryClient&) = delete;
  ProcessMemoryClient& operator=(const ProcessMemoryClient&) = delete;
  ProcessMemoryClient(ProcessMemoryClient&&) = delete;
  ProcessMemoryClient& operator=(ProcessMemoryClient&&) = delete;

  using ProcessMemoryCallback =
      std::function<void(const std::vector<uint8_t>&)>;

  void GetRemoteMemory(uint32_t pid, uint64_t address, uint64_t size,
                       const ProcessMemoryCallback& callback);

 private:
  void HandleResponse(const Message& message, uint64_t id);

  TransactionClient* transaction_client_;
  absl::Mutex transaction_mutex_;
  absl::flat_hash_map<uint64_t, ProcessMemoryCallback> callbacks_;
};

#endif  // ORBIT_CORE_PROCESS_MEMORY_CLIENT_H_
