#ifndef ORBIT_LINUX_TRACING_EVENTS_H_
#define ORBIT_LINUX_TRACING_EVENTS_H_

#include <unistd.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace LinuxTracing {

class ContextSwitch {
 public:
  pid_t GetTid() const { return tid_; }
  uint16_t GetCore() const { return core_; }
  uint64_t GetTimestampNs() const { return timestamp_ns_; }

 protected:
  ContextSwitch(pid_t tid, uint16_t core, uint64_t timestamp_ns)
      : tid_(tid), core_(core), timestamp_ns_(timestamp_ns) {}

 private:
  pid_t tid_;
  uint16_t core_;
  uint64_t timestamp_ns_;
};

class ContextSwitchIn : public ContextSwitch {
 public:
  ContextSwitchIn(pid_t tid, uint16_t core, uint64_t timestamp_ns)
      : ContextSwitch(tid, core, timestamp_ns) {}
};

class ContextSwitchOut : public ContextSwitch {
 public:
  ContextSwitchOut(pid_t tid, uint16_t core, uint64_t timestamp_ns)
      : ContextSwitch(tid, core, timestamp_ns) {}
};

class CallstackFrame {
 public:
  CallstackFrame(uint64_t pc, std::string function_name,
                 uint64_t function_offset, std::string map_name)
      : pc_(pc),
        function_name_(std::move(function_name)),
        function_offset_(function_offset),
        map_name_(std::move(map_name)) {}

  uint64_t GetPc() const { return pc_; }
  const std::string& GetFunctionName() const { return function_name_; }
  uint64_t GetFunctionOffset() const { return function_offset_; }
  const std::string& GetMapName() const { return map_name_; }

 private:
  uint64_t pc_;
  std::string function_name_;
  uint64_t function_offset_;
  std::string map_name_;
};

class Callstack {
 public:
  Callstack(pid_t tid, std::vector<CallstackFrame> frames,
            uint64_t timestamp_ns)
      : tid_(tid), frames_(std::move(frames)), timestamp_ns_(timestamp_ns) {}

  pid_t GetTid() const { return tid_; }
  const std::vector<CallstackFrame>& GetFrames() const { return frames_; }
  uint64_t GetTimestampNs() const { return timestamp_ns_; }

 private:
  pid_t tid_;
  std::vector<CallstackFrame> frames_;
  uint64_t timestamp_ns_;
};

class FunctionCall {
 public:
  FunctionCall(pid_t tid, uint64_t virtual_address, uint64_t begin_timestamp_ns,
               uint64_t end_timestamp_ns, uint32_t depth)
      : tid_(tid),
        virtual_address_(virtual_address),
        begin_timestamp_ns_(begin_timestamp_ns),
        end_timestamp_ns_(end_timestamp_ns),
        depth_{depth} {}

  pid_t GetTid() const { return tid_; }
  uint64_t GetVirtualAddress() const { return virtual_address_; }
  uint64_t GetBeginTimestampNs() const { return begin_timestamp_ns_; }
  uint64_t GetEndTimestampNs() const { return end_timestamp_ns_; }
  uint32_t GetDepth() const { return depth_; }

 private:
  pid_t tid_;
  uint64_t virtual_address_;
  uint64_t begin_timestamp_ns_;
  uint64_t end_timestamp_ns_;
  uint32_t depth_;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_EVENTS_H_
