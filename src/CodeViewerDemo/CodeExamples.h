// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QString>

inline const QString testing_example =
    R"(
// Tests to understand how the syntax highlighter works
  // Number unit tests
  a = 0x8542;
  a = 4103;
  a = 0;
  a = 01ab;
  a = a105;
  a = _103;

  // Capitalized words unit tests
  OrbitService o;
  QSyntaxHighlighter q;
  ORBIT_SCOPE; // no lowercase regex
  x = _Tree_;
  bool P = (kTreeSize == 3);

  // Class members
  Track* track_;
  int __int__ = 3;
  m_PascalCase = 5;
  Track::SortTracks();
  class TrackManager {};
  int pam_value; // not a class member

  // Function unit tests
  int my_function(void);
  int MyFunction(void);
  bool b = a.insert(3);
  void AddEvent();
  char _my_underscore_func();
  int CaptureServiceImpl::Capture(); // Function from other class
  a = absl::Milliseconds(20); // method that we don't have to highlight
  int QSyntaxHighlighter::PlayInteger;
  ORBIT_CHECK(false); // no lowercase regex

  // Preprocessor + <> unit tests
  #include <iostream>
  std::vector<Typeee>
  #include<iostream>
   #pragma<ios>
  include<iostream>

  // Strings unit tests
  "Hola mundo\n"
  "I'm an \"only\" string"
  "I'm 2" + "strings"
  "ORBIT_CHECK(false)\\\\"
  "Multi\
  Line"

  // Comments unit tests
  // return
  // This is a comment
  return; // This isn't a comment
  /*This
  is a
  long comment*/
  /* One line comment */ int main;

// CaptureServiceImpl.cpp
// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <pthread.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include "CaptureEventBuffer.h"
#include "CaptureEventSender.h"
#include "CaptureServiceImpl.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"
#include "TracingHandler.h"

namespace orbit_service {

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

namespace {

using orbit_grpc_protos::CaptureEvent;

class SenderThreadCaptureEventBuffer final : public CaptureEventBuffer {
 public:
  explicit SenderThreadCaptureEventBuffer(CaptureEventSender* event_sender)
      : capture_event_sender_{event_sender} {
    ORBIT_CHECK(capture_event_sender_ != nullptr);
    sender_thread_ = std::thread{[this] { SenderThread(); }};
  }

  void AddEvent(orbit_grpc_protos::CaptureEvent&& event) override {
    absl::MutexLock lock{&event_buffer_mutex_};
    if (stop_requested_) {
      return;
    }
    event_buffer_.emplace_back(std::move(event));
  }

  void StopAndWait() {
    ORBIT_CHECK(sender_thread_.joinable());
    {
      // Protect stop_requested_ with event_buffer_mutex_ so that we can use stop_requested_
      // in Conditions for Await/LockWhen (specifically, in SenderThread).
      absl::MutexLock lock{&event_buffer_mutex_};
      stop_requested_ = true;
    }
    sender_thread_.join();
  }

  ~SenderThreadCaptureEventBuffer() override { ORBIT_CHECK(!sender_thread_.joinable()); }

 private:
  void SenderThread() {
    orbit_base::SetCurrentThreadName("SenderThread");
    constexpr absl::Duration kSendTimeInterval = absl::Milliseconds(20);
    // This should be lower than kMaxEventsPerResponse in GrpcCaptureEventSender::SendEvents
    // as a few more events are likely to arrive after the condition becomes true.
    constexpr uint64_t kSendEventCountInterval = 5000;

    bool stopped = false;
    while (!stopped) {
      ORBIT_SCOPE("SenderThread iteration");
      event_buffer_mutex_.LockWhenWithTimeout(absl::Condition(
                                                  +[](SenderThreadCaptureEventBuffer* self) {
                                                    return self->event_buffer_.size() >=
                                                               kSendEventCountInterval ||
                                                           self->stop_requested_;
                                                  },
                                                  this),
                                              kSendTimeInterval);
      if (stop_requested_) {
        stopped = true;
      }
      std::vector<CaptureEvent> buffered_events = std::move(event_buffer_);
      event_buffer_.clear();
      event_buffer_mutex_.Unlock();
      capture_event_sender_->SendEvents(std::move(buffered_events));
    }
  }

  std::vector<orbit_grpc_protos::CaptureEvent> event_buffer_;
  absl::Mutex event_buffer_mutex_;
  CaptureEventSender* capture_event_sender_;
  std::thread sender_thread_;
  bool stop_requested_ = false;
};

class GrpcCaptureEventSender final : public CaptureEventSender {
 public:
  explicit GrpcCaptureEventSender(
      grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {
    ORBIT_CHECK(reader_writer_ != nullptr);
  }

  ~GrpcCaptureEventSender() override {
    ORBIT_LOG("Total number of events sent: %lu", total_number_of_events_sent_);
    ORBIT_LOG("Total number of bytes sent: %lu", total_number_of_bytes_sent_);

    // Ensure we can divide by 0.f safely.
    static_assert(std::numeric_limits<float>::is_iec559);
    float average_bytes =
        static_cast<float>(total_number_of_bytes_sent_) / total_number_of_events_sent_;

    ORBIT_LOG("Average number of bytes per event: %.2f", average_bytes);
  }

  void SendEvents(std::vector<orbit_grpc_protos::CaptureEvent>&& events) override {
    ORBIT_SCOPE_FUNCTION;
    ORBIT_UINT64("Number of buffered events sent", events.size());
    if (events.empty()) {
      return;
    }

    constexpr uint64_t kMaxEventsPerResponse = 10'000;
    uint64_t number_of_bytes_sent = 0;
    CaptureResponse response;
    for (CaptureEvent& event : events) {
      // We buffer to avoid sending countless tiny messages, but we also want to
      // avoid huge messages, which would cause the capture on the client to jump
      // forward in time in few big steps and not look live anymore.
      if (response.capture_events_size() == kMaxEventsPerResponse) {
        number_of_bytes_sent += response.ByteSizeLong();
        reader_writer_->Write(response);
        response.clear_capture_events();
      }
      response.mutable_capture_events()->Add(std::move(event));
    }
    number_of_bytes_sent += response.ByteSizeLong();
    reader_writer_->Write(response);

    // Ensure we can divide by 0.f safely.
    static_assert(std::numeric_limits<float>::is_iec559);
    float average_bytes = static_cast<float>(number_of_bytes_sent) / events.size();

    ORBIT_FLOAT("Average bytes per CaptureEvent", average_bytes);
    total_number_of_events_sent_ += events.size();
    total_number_of_bytes_sent_ += number_of_bytes_sent;
  }

 private:
  grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer_;

  uint64_t total_number_of_events_sent_ = 0;
  uint64_t total_number_of_bytes_sent_ = 0;
};

}  // namespace

// TracingHandler::Stop is blocking, until all perf_event_open events have been processed
// and all perf_event_open file descriptors have been closed.
// CaptureStartStopListener::OnCaptureStopRequested is also to be assumed blocking,
// for example until all CaptureEvents from external producers have been received.
// Hence why these methods need to be called in parallel on different threads.
static void StopTracingHandlerAndCaptureStartStopListenersInParallel(
    TracingHandler* tracing_handler,
    absl::flat_hash_set<CaptureStartStopListener*>* capture_start_stop_listeners) {
  std::vector<std::thread> stop_threads;

  stop_threads.emplace_back([&tracing_handler] {
    tracing_handler->Stop();
    ORBIT_LOG("TracingHandler stopped: perf_event_open tracing is done");
  });

  for (CaptureStartStopListener* listener : *capture_start_stop_listeners) {
    stop_threads.emplace_back([&listener] {
      listener->OnCaptureStopRequested();
      ORBIT_LOG("CaptureStartStopListener stopped: one or more producers finished capturing");
    });
  }

  for (std::thread& stop_thread : stop_threads) {
    stop_thread.join();
  }
}

grpc::Status CaptureServiceImpl::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("CSImpl::Capture");
  if (is_capturing) {
    ORBIT_ERROR("Cannot start capture because another capture is already in progress");
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "Cannot start capture because another capture is already in progress.");
  }
  is_capturing = true;

  GrpcCaptureEventSender capture_event_sender{reader_writer};
  SenderThreadCaptureEventBuffer capture_event_buffer{&capture_event_sender};
  TracingHandler tracing_handler{&capture_event_buffer};

  CaptureRequest request;
  reader_writer->Read(&request);
  ORBIT_LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");

  tracing_handler.Start(std::move(*request.mutable_capture_options()));
  for (CaptureStartStopListener* listener : capture_start_stop_listeners_) {
    listener->OnCaptureStartRequested(&capture_event_buffer);
  }

  // The client asks for the capture to be stopped by calling WritesDone.
  // At that point, this call to Read will return false.
  // In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  ORBIT_LOG("Client finished writing on Capture's gRPC stream: stopping capture");

  StopTracingHandlerAndCaptureStartStopListenersInParallel(&tracing_handler,
                                                           &capture_start_stop_listeners_);

  capture_event_buffer.StopAndWait();
  ORBIT_LOG("Finished handling gRPC call to Capture: all capture data has been sent");
  is_capturing = false;
  return grpc::Status::OK;
}

void CaptureServiceImpl::AddCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool new_insertion = capture_start_stop_listeners_.insert(listener).second;
  ORBIT_CHECK(new_insertion);
}

void CaptureServiceImpl::RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool was_removed = capture_start_stop_listeners_.erase(listener) > 0;
  ORBIT_CHECK(was_removed);
}

}  // namespace orbit_service
)";

inline const QString cpp_example = R"(#pragma twice
// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "OrbitGl/TrackManager.h"

class TrackManager {
 public:
  explicit TrackManager(TimeGraph* time_graph, OrbitApp* app);
  void Clear();

  [[nodiscard]] std::vector<Track*> GetAllTracks() const;

 protected:
  /* This is a protected area. "This is not a string" */
 private: /* Nobody
           * can
           * access
           * this
           * part */
  std::vector<Track*> tracks_;
}

int main (int argc, char **argv) {
  char start_capture = 'x';
  std::string multiline =
      " This \
                            is a \
                       multiline \
                          string";
  int* my_ptr = NULL;
  printf("Hi Orbit's user, if you want to start a capture please press \"%c\"\n", start_capture);
  return 0;

})";

inline const QString x86Assembly_example = R"(Platform: X86 64 (Intel syntax)
0x557e1f8091d0:	push         rbp
0x557e1f8091d1:	mov          rbp, rsp
0x557e1f8091d4:	sub          rsp, 0x10
0x557e1f8091d8:	mov          rax, qword ptr fs:[0x28]
0x557e1f8091e1:	mov          qword ptr [rbp - 8], rax
0x557e1f8091e2:	call         0x123 (bbb::foo() const)
0x557e1f8091e3:	call         0x123 (bbb::foo<int a, std::function<void (int)>>(int, std::function<void (B)>))
0x55da67cf3ee4:	call         0x55da686727c0 (???)
0x55da67cf3ee4:	call         0x55da686727c0 (_foo)
0x557e1f8091e5:	xorps        xmm0, xmm0
0x557e1f8091e8:	movups       xmmword ptr [rdi], xmm0
0x557e1f8091eb:	mov          qword ptr [rdi + 0x10], 0
0x557e1f8091f3:	mov          byte ptr [rdi], 0x2a
0x557e1f8091f6:	movabs       rax, 0x65646f6d20726570
0x557e1f809200:	mov          qword ptr [rdi + 0xe], rax
0x557e1f809204:	movups       xmm0, xmmword ptr [rip - 0x36e2d5]
0x557e1f80920b:	movups       xmmword ptr [rdi + 1], xmm0
0x557e1f80920f:	mov          byte ptr [rdi + 0x16], 0
0x557e1f809213:	mov          rax, qword ptr fs:[0x28]
0x557e1f80921c:	cmp          rax, qword ptr [rbp - 8]
0x557e1f809220:	jne          0x557e1f80922b
0x557e1f809222:	mov          rax, rdi
0x557e1f809225:	add          rsp, 0x10
0x557e1f809229:	pop          rbp
0x557e1f80922a:	ret
0x557e1f80922b:	call         0x557e2000aa70
0x557e1f809230:)";
