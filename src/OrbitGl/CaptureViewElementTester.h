// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_VIEW_ELEMENT_TESTER_H_
#define ORBIT_GL_CAPTURE_VIEW_ELEMENT_TESTER_H_

#include "CaptureViewElement.h"

namespace orbit_gl {

// Can be used to test basic functionality of classes inheriting from CaptureViewElement to assure
// all required methods are implemented / overriden correctly.
// See CaptureViewElementTest.cpp for usage.
class CaptureViewElementTester {
 public:
  void RunTests(CaptureViewElement* element);

  Viewport* GetViewport() { return &viewport_; }
  TimeGraphLayout* GetLayout() { return &layout_; }

 protected:
  Viewport viewport_ = Viewport(100, 100);
  TimeGraphLayout layout_;

 private:
  void TestWidthPropagationToChildren(CaptureViewElement* element);
};

}  // namespace orbit_gl

#endif