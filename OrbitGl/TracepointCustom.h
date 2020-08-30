//
// Created by msandru on 8/28/20.
//

#ifndef ORBIT_TRACEPOINTCUSTOM_H
#define ORBIT_TRACEPOINTCUSTOM_H

#include "tracepoint.pb.h"

namespace internal {
struct HashTracepointInfo {
  size_t operator()(const orbit_grpc_protos::TracepointInfo& info) const {
    return std::hash<std::string>{}(info.category()) * 37 + std::hash<std::string>{}(info.name());
  }
};

struct EqualTracepointInfo {
  bool operator()(const orbit_grpc_protos::TracepointInfo& left,
                  const orbit_grpc_protos::TracepointInfo& right) const {
    return left.category().compare(right.category()) == 0 && left.name().compare(right.name()) == 0;
  }
};
}  // namespace internal

#endif  // ORBIT_TRACEPOINTCUSTOM_H
