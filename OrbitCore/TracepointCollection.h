//
// Created by msandru on 8/29/20.
//

#ifndef ORBIT_TRACEPOINTCOLLECTION_H
#define ORBIT_TRACEPOINTCOLLECTION_H

#include <string>
#include <utility>

#include "tracepoint.pb.h"

using orbit_grpc_protos::TracepointInfo;

struct TracepointCollection {
  TracepointCollection();
  TracepointCollection(int32_t pid, int32_t tid, int64_t time, int64_t stream_id, int32_t cpu,
                       TracepointInfo tracepoint_info)
      : pid{pid},
        tid{tid},
        time{time},
        stream_id{stream_id},
        cpu{cpu},
        tracepoint_info{tracepoint_info} {}

  int32_t pid, tid;  /* if PERF_SAMPLE_TID */
  int64_t time;      /* if PERF_SAMPLE_TIME */
  int64_t stream_id; /* if PERF_SAMPLE_STREAM_ID */
  int32_t cpu;       /* if PERF_SAMPLE_CPU */
  TracepointInfo tracepoint_info;
};

#endif  // ORBIT_TRACEPOINTCOLLECTION_H
