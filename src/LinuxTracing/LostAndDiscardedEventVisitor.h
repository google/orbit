// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_LOST_AND_DISCARDED_EVENT_VISITOR_H_
#define LINUX_TRACING_LOST_AND_DISCARDED_EVENT_VISITOR_H_

#include "LinuxTracing/TracerListener.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"
#include "capture.pb.h"

namespace orbit_linux_tracing {

// This class processes LostPerfEvents and DiscardedPerfEvents and sends the corresponding
// MetadataEvents to the TracerListener.
class LostAndDiscardedEventVisitor : public PerfEventVisitor {
 public:
  void SetListener(TracerListener* listener) { listener_ = listener; }

  void visit(LostPerfEvent* event) override {
    orbit_grpc_protos::LostPerfRecordsEvent lost_perf_records_event;
    lost_perf_records_event.set_duration_ns(event->GetTimestamp() - event->GetPreviousTimestamp());
    lost_perf_records_event.set_end_timestamp_ns(event->GetTimestamp());

    CHECK(listener_ != nullptr);
    listener_->OnLostPerfRecordsEvent(std::move(lost_perf_records_event));
  }

  void visit(DiscardedPerfEvent* event) override {
    orbit_grpc_protos::OutOfOrderEventsDiscardedEvent out_of_order_events_discarded_event;
    out_of_order_events_discarded_event.set_duration_ns(event->GetEndTimestampNs() -
                                                        event->GetBeginTimestampNs());
    out_of_order_events_discarded_event.set_end_timestamp_ns(event->GetEndTimestampNs());

    CHECK(listener_ != nullptr);
    listener_->OnOutOfOrderEventsDiscardedEvent(std::move(out_of_order_events_discarded_event));
  }

 private:
  TracerListener* listener_ = nullptr;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LOST_AND_DISCARDED_EVENT_VISITOR_H_
