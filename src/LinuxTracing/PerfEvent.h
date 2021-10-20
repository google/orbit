// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_H_
#define LINUX_TRACING_PERF_EVENT_H_

#include <asm/perf_regs.h>
#include <string.h>
#include <sys/types.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "GrpcProtos/Constants.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "PerfEventRecords.h"

namespace orbit_linux_tracing {

class PerfEventVisitor;

[[nodiscard]] std::array<uint64_t, PERF_REG_X86_64_MAX>
perf_event_sample_regs_user_all_to_register_array(const perf_event_sample_regs_user_all& regs);

static constexpr int kNotOrderedInAnyFileDescriptor = -1;

struct ForkPerfEventData {
  pid_t pid;
  pid_t tid;
};

struct ExitPerfEventData {
  pid_t pid;
  pid_t tid;
};

struct LostPerfEventData {
  uint64_t previous_timestamp = 0;
};
struct DiscardedPerfEventData {
  uint64_t begin_timestamp_ns;
};

struct StackSamplePerfEventData {
  [[nodiscard]] std::array<uint64_t, PERF_REG_X86_64_MAX> GetRegisters() const {
    return perf_event_sample_regs_user_all_to_register_array(*regs);
  }
  [[nodiscard]] const char* GetStackData() const { return data.get(); }
  [[nodiscard]] char* GetMutableStackData() { return data.get(); }
  [[nodiscard]] uint64_t GetStackSize() const { return dyn_size; }

  pid_t pid;
  pid_t tid;
  std::unique_ptr<perf_event_sample_regs_user_all> regs;
  uint64_t dyn_size;
  std::unique_ptr<char[]> data;
};

struct CallchainSamplePerfEventData {
  [[nodiscard]] const uint64_t* GetCallchain() const { return ips.get(); }
  [[nodiscard]] uint64_t GetCallchainSize() const { return ips_size; }
  [[nodiscard]] std::array<uint64_t, PERF_REG_X86_64_MAX> GetRegisters() const {
    return perf_event_sample_regs_user_all_to_register_array(*regs);
  }
  [[nodiscard]] const char* GetStackData() const { return data.get(); }
  void SetIps(const std::vector<uint64_t>& new_ips) {
    ips_size = new_ips.size();
    ips = make_unique_for_overwrite<uint64_t[]>(ips_size);
    memcpy(ips.get(), new_ips.data(), ips_size * sizeof(uint64_t));
  }
  [[nodiscard]] std::vector<uint64_t> CopyOfIpsAsVector() const {
    return std::vector<uint64_t>(ips.get(), ips.get() + ips_size);
  }

  pid_t pid;
  pid_t tid;
  // Mutability is needed in SetIps which in turn is needed by
  // LeafFunctionCallManager::PatchCallerOfLeafFunction.
  uint64_t ips_size;
  std::unique_ptr<uint64_t[]> ips;
  std::unique_ptr<perf_event_sample_regs_user_all> regs;
  std::unique_ptr<char[]> data;
};

struct UprobesPerfEventData {
  pid_t pid;
  pid_t tid;
  uint32_t cpu;
  uint64_t function_id = orbit_grpc_protos::kInvalidFunctionId;
  uint64_t sp;
  uint64_t ip;
  uint64_t return_address;
};

struct UprobesWithArgumentsPerfEventData {
  pid_t pid;
  pid_t tid;
  uint32_t cpu;
  uint64_t function_id = orbit_grpc_protos::kInvalidFunctionId;
  uint64_t return_address;
  perf_event_sample_regs_user_sp_ip_arguments regs;
};

struct UretprobesPerfEventData {
  pid_t pid;
  pid_t tid;
};

struct UretprobesWithReturnValuePerfEventData {
  pid_t pid;
  pid_t tid;
  uint64_t rax;
};

struct MmapPerfEventData {
  uint64_t address;
  uint64_t length;
  uint64_t page_offset;
  std::string filename;
  pid_t pid;
};

struct GenericTracepointPerfEventData {
  pid_t pid;
  pid_t tid;
  uint32_t cpu;
};

struct TaskNewtaskPerfEventData {
  char comm[16];
  pid_t new_tid;
};

struct TaskRenamePerfEventData {
  char newcomm[16];
  pid_t renamed_tid;
};

struct SchedSwitchPerfEventData {
  uint32_t cpu;
  pid_t prev_pid_or_minus_one;
  pid_t prev_tid;
  int64_t prev_state;
  int32_t next_tid;
};

struct SchedWakeupPerfEventData {
  pid_t woken_tid;
};

struct AmdgpuCsIoctlPerfEventData {
  pid_t pid;
  pid_t tid;
  uint32_t context;
  uint32_t seqno;
  std::string timeline_string;
};

struct AmdgpuSchedRunJobPerfEventData {
  pid_t pid;
  pid_t tid;
  uint32_t context;
  uint32_t seqno;
  std::string timeline_string;
};

struct DmaFenceSignaledPerfEventData {
  pid_t pid;
  pid_t tid;
  uint32_t context;
  uint32_t seqno;
  std::string timeline_string;
};

class PerfEvent {
 public:
  using DataVariant =
      std::variant<ForkPerfEventData, ExitPerfEventData, LostPerfEventData, DiscardedPerfEventData,
                   StackSamplePerfEventData, CallchainSamplePerfEventData, UprobesPerfEventData,
                   UprobesWithArgumentsPerfEventData, UretprobesPerfEventData,
                   UretprobesWithReturnValuePerfEventData, MmapPerfEventData,
                   GenericTracepointPerfEventData, TaskNewtaskPerfEventData,
                   TaskRenamePerfEventData, SchedSwitchPerfEventData, SchedWakeupPerfEventData,
                   AmdgpuCsIoctlPerfEventData, AmdgpuSchedRunJobPerfEventData,
                   DmaFenceSignaledPerfEventData>;

  PerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, DataVariant&& data)
      : ordered_in_file_descriptor_{ordered_in_file_descriptor},
        timestamp_{timestamp},
        data_{std::move(data)} {}

  [[nodiscard]] int ordered_in_file_descriptor() const { return ordered_in_file_descriptor_; }
  [[nodiscard]] uint64_t timestamp() const { return timestamp_; }
  [[nodiscard]] const DataVariant& data_variant() const { return data_; }
  [[nodiscard]] DataVariant& data_variant() { return data_; }

  void Accept(PerfEventVisitor* visitor);

 private:
  // These top-level fields are common to all events, while each of the `...PerfEventData`s in the
  // `DataVariant data_` contains the data specific to each type of event.
  int ordered_in_file_descriptor_;
  uint64_t timestamp_;
  DataVariant data_;
};

template <typename PerfEventDataT>
class TypedPerfEvent : public PerfEvent {
 public:
  TypedPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, PerfEventDataT&& data)
      : PerfEvent{ordered_in_file_descriptor, timestamp, std::move(data)} {}

  [[nodiscard]] const PerfEventDataT& data() const {
    return std::get<PerfEventDataT>(data_variant());
  }
  [[nodiscard]] PerfEventDataT& data() { return std::get<PerfEventDataT>(data_variant()); }
};

class ForkPerfEvent : public TypedPerfEvent<ForkPerfEventData> {
 public:
  ForkPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid, pid_t tid)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       ForkPerfEventData{.pid = pid, .tid = tid}} {}
};

class ExitPerfEvent : public TypedPerfEvent<ExitPerfEventData> {
 public:
  ExitPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid, pid_t tid)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       ExitPerfEventData{.pid = pid, .tid = tid}} {}
};

class LostPerfEvent : public TypedPerfEvent<LostPerfEventData> {
 public:
  LostPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, uint64_t previous_timestamp)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       LostPerfEventData{.previous_timestamp = previous_timestamp}} {}
};

// This class doesn't correspond to any event generated by perf_event_open. Rather, these events
// are produced by PerfEventProcessor. We need them to be part of the same PerfEvent "hierarchy".
class DiscardedPerfEvent : public TypedPerfEvent<DiscardedPerfEventData> {
 public:
  DiscardedPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp,
                     uint64_t begin_timestamp_ns)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       DiscardedPerfEventData{.begin_timestamp_ns = begin_timestamp_ns}} {}
};

class StackSamplePerfEvent : public TypedPerfEvent<StackSamplePerfEventData> {
 public:
  StackSamplePerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid, pid_t tid,
                       uint64_t dyn_size)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       StackSamplePerfEventData{
                           .pid = pid,
                           .tid = tid,
                           .regs = make_unique_for_overwrite<perf_event_sample_regs_user_all>(),
                           .dyn_size = dyn_size,
                           .data = make_unique_for_overwrite<char[]>(dyn_size)}} {}
};

class CallchainSamplePerfEvent : public TypedPerfEvent<CallchainSamplePerfEventData> {
 public:
  CallchainSamplePerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid, pid_t tid,
                           uint64_t nr, uint64_t dyn_size)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       CallchainSamplePerfEventData{
                           .pid = pid,
                           .tid = tid,
                           .ips_size = nr,
                           .ips = make_unique_for_overwrite<uint64_t[]>(nr),
                           .regs = make_unique_for_overwrite<perf_event_sample_regs_user_all>(),
                           .data = make_unique_for_overwrite<char[]>(dyn_size)}} {}
};

class UprobesPerfEvent : public TypedPerfEvent<UprobesPerfEventData> {
 public:
  UprobesPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid, pid_t tid,
                   uint32_t cpu, uint64_t function_id, uint64_t sp, uint64_t ip,
                   uint64_t return_address)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       UprobesPerfEventData{.pid = pid,
                                            .tid = tid,
                                            .cpu = cpu,
                                            .function_id = function_id,
                                            .sp = sp,
                                            .ip = ip,
                                            .return_address = return_address}} {}
};

class UprobesWithArgumentsPerfEvent : public TypedPerfEvent<UprobesWithArgumentsPerfEventData> {
 public:
  UprobesWithArgumentsPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid,
                                pid_t tid, uint32_t cpu, uint64_t function_id,
                                uint64_t return_address,
                                perf_event_sample_regs_user_sp_ip_arguments regs)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       UprobesWithArgumentsPerfEventData{.pid = pid,
                                                         .tid = tid,
                                                         .cpu = cpu,
                                                         .function_id = function_id,
                                                         .return_address = return_address,
                                                         .regs = regs}} {}
};

class UretprobesPerfEvent : public TypedPerfEvent<UretprobesPerfEventData> {
 public:
  UretprobesPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid, pid_t tid)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       UretprobesPerfEventData{.pid = pid, .tid = tid}} {}
};

class UretprobesWithReturnValuePerfEvent
    : public TypedPerfEvent<UretprobesWithReturnValuePerfEventData> {
 public:
  UretprobesWithReturnValuePerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid,
                                     pid_t tid, uint64_t rax)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       UretprobesWithReturnValuePerfEventData{.pid = pid, .tid = tid, .rax = rax}} {
  }
};

class MmapPerfEvent : public TypedPerfEvent<MmapPerfEventData> {
 public:
  MmapPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, uint64_t address,
                uint64_t length, uint64_t page_offset, std::string filename, pid_t pid)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       MmapPerfEventData{.address = address,
                                         .length = length,
                                         .page_offset = page_offset,
                                         .filename = std::move(filename),
                                         .pid = pid}} {}
};

class GenericTracepointPerfEvent : public TypedPerfEvent<GenericTracepointPerfEventData> {
 public:
  GenericTracepointPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid,
                             pid_t tid, uint32_t cpu)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       GenericTracepointPerfEventData{.pid = pid, .tid = tid, .cpu = cpu}} {}
};

class TaskNewtaskPerfEvent : public TypedPerfEvent<TaskNewtaskPerfEventData> {
 public:
  TaskNewtaskPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, const char comm[16],
                       pid_t new_tid)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       TaskNewtaskPerfEventData{.new_tid = new_tid}} {
    memcpy(data().comm, comm, 16);
  }
};

class TaskRenamePerfEvent : public TypedPerfEvent<TaskRenamePerfEventData> {
 public:
  TaskRenamePerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, const char newcomm[16],
                      pid_t renamed_tid)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       TaskRenamePerfEventData{.renamed_tid = renamed_tid}} {
    memcpy(data().newcomm, newcomm, 16);
  }
};

class SchedSwitchPerfEvent : public TypedPerfEvent<SchedSwitchPerfEventData> {
 public:
  SchedSwitchPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, uint32_t cpu,
                       pid_t prev_pid_or_minus_one, pid_t prev_tid, int64_t prev_state,
                       int32_t next_tid)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       SchedSwitchPerfEventData{.cpu = cpu,
                                                .prev_pid_or_minus_one = prev_pid_or_minus_one,
                                                .prev_tid = prev_tid,
                                                .prev_state = prev_state,
                                                .next_tid = next_tid}} {}
};

class SchedWakeupPerfEvent : public TypedPerfEvent<SchedWakeupPerfEventData> {
 public:
  SchedWakeupPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t woken_tid)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       SchedWakeupPerfEventData{.woken_tid = woken_tid}} {}
};

class AmdgpuCsIoctlPerfEvent : public TypedPerfEvent<AmdgpuCsIoctlPerfEventData> {
 public:
  AmdgpuCsIoctlPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid, pid_t tid,
                         uint32_t context, uint32_t seqno, std::string timeline_string)
      : TypedPerfEvent{ordered_in_file_descriptor, timestamp,
                       AmdgpuCsIoctlPerfEventData{.pid = pid,
                                                  .tid = tid,
                                                  .context = context,
                                                  .seqno = seqno,
                                                  .timeline_string = std::move(timeline_string)}} {}
};

class AmdgpuSchedRunJobPerfEvent : public TypedPerfEvent<AmdgpuSchedRunJobPerfEventData> {
 public:
  AmdgpuSchedRunJobPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid,
                             pid_t tid, uint32_t context, uint32_t seqno,
                             std::string timeline_string)
      : TypedPerfEvent{
            ordered_in_file_descriptor, timestamp,
            AmdgpuSchedRunJobPerfEventData{.pid = pid,
                                           .tid = tid,
                                           .context = context,
                                           .seqno = seqno,
                                           .timeline_string = std::move(timeline_string)}} {}
};

class DmaFenceSignaledPerfEvent : public TypedPerfEvent<DmaFenceSignaledPerfEventData> {
 public:
  DmaFenceSignaledPerfEvent(int ordered_in_file_descriptor, uint64_t timestamp, pid_t pid,
                            pid_t tid, uint32_t context, uint32_t seqno,
                            std::string timeline_string)
      : TypedPerfEvent{
            ordered_in_file_descriptor, timestamp,
            DmaFenceSignaledPerfEventData{.pid = pid,
                                          .tid = tid,
                                          .context = context,
                                          .seqno = seqno,
                                          .timeline_string = std::move(timeline_string)}} {}
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_H_
