// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_THREAD_BAR_H_
#define ORBIT_GL_THREAD_BAR_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <utility>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Viewport.h"

class OrbitApp;

namespace orbit_gl {

class ThreadBar : public CaptureViewElement, public std::enable_shared_from_this<ThreadBar> {
 public:
  explicit ThreadBar(CaptureViewElement* parent, OrbitApp* app,
                     const orbit_gl::TimelineInfoInterface* timeline_info,
                     orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                     const orbit_client_data::ModuleManager* module_manager,
                     const orbit_client_data::CaptureData* capture_data, int64_t thread_id,
                     std::string name)
      : CaptureViewElement(parent, viewport, layout),
        app_(app),
        timeline_info_(timeline_info),
        module_manager_(module_manager),
        capture_data_(capture_data),
        thread_id_(thread_id),
        name_(std::move(name)) {}

  [[nodiscard]] virtual bool IsEmpty() const { return false; }
  [[nodiscard]] bool ShouldBeRendered() const override {
    return CaptureViewElement::ShouldBeRendered() && !IsEmpty();
  }

  [[nodiscard]] virtual const std::string& GetName() const { return name_; }

 protected:
  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;
  [[nodiscard]] int64_t GetThreadId() const { return thread_id_; }

  OrbitApp* app_;
  const orbit_gl::TimelineInfoInterface* timeline_info_;
  const orbit_client_data::ModuleManager* module_manager_;
  const orbit_client_data::CaptureData* capture_data_;

 private:
  int64_t thread_id_;
  std::string name_;
};

}  // namespace orbit_gl
#endif