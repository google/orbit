// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_LOST_AND_DISCARDED_EVENT_VISITOR_H_
#define LINUX_TRACING_LOST_AND_DISCARDED_EVENT_VISITOR_H_

#include "GrpcProtos/capture.pb.h"
#include "LinuxTracing/TracerListener.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"

namespace orbit_linux_tracing {

// This class processes LostPerfEvents and DiscardedPerfEvents and sends the corresponding
// MetadataEvents to the TracerListener.
class LostAndDiscardedEventVisitor : public PerfEventVisitor {
 public:
  explicit LostAndDiscardedEventVisitor(TracerListener* listener) : listener_{listener} {
    ORBIT_CHECK(listener_ != nullptr);
  }

  void Visit(uint64_t event_timestamp, const LostPerfEventData& event_data) override {
    orbit_grpc_protos::LostPerfRecordsEvent lost_perf_records_event;
    lost_perf_records_event.set_duration_ns(event_timestamp - event_data.previous_timestamp);
    lost_perf_records_event.set_end_timestamp_ns(event_timestamp);

    ORBIT_CHECK(listener_ != nullptr);
    listener_->OnLostPerfRecordsEvent(std::move(lost_perf_records_event));
  }

  void Visit(uint64_t event_timestamp, const DiscardedPerfEventData& event_data) override {
    orbit_grpc_protos::OutOfOrderEventsDiscardedEvent out_of_order_events_discarded_event;
    out_of_order_events_discarded_event.set_duration_ns(event_timestamp -
                                                        event_data.begin_timestamp_ns);
    out_of_order_events_discarded_event.set_end_timestamp_ns(event_timestamp);

    ORBIT_CHECK(listener_ != nullptr);
    listener_->OnOutOfOrderEventsDiscardedEvent(std::move(out_of_order_events_discarded_event));
  }

 private:
  TracerListener* listener_;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LOST_AND_DISCARDED_EVENT_VISITOR_H_
