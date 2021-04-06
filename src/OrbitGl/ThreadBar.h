// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_THREAD_BAR_H_
#define ORBIT_GL_THREAD_BAR_H_

#include <memory>
#include <string>

#include "CaptureViewElement.h"
#include "OrbitAccessibility/WrappedAccessibility.h"
#include "OrbitClientModel/CaptureData.h"
#include "TimeGraphLayout.h"

class OrbitApp;

namespace orbit_gl {

class ThreadBar : public CaptureViewElement, public std::enable_shared_from_this<ThreadBar> {
 public:
  explicit ThreadBar(WrappedAccessibility* parent, OrbitApp* app, TimeGraph* time_graph,
                     TimeGraphLayout* layout, const CaptureData* capture_data, ThreadID thread_id,
                     std::string name)
      : CaptureViewElement(parent, time_graph, layout),
        thread_id_(thread_id),
        app_(app),
        capture_data_(capture_data),
        name_(name) {}

  void SetThreadId(ThreadID thread_id) { thread_id_ = thread_id; }
  [[nodiscard]] virtual bool IsEmpty() const { return false; }

  [[nodiscard]] virtual const std::string& GetName() const { return name_; }

 protected:
  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;

  ThreadID thread_id_ = -1;
  OrbitApp* app_;
  const CaptureData* capture_data_;

  std::string name_;
};

}  // namespace orbit_gl
#endif