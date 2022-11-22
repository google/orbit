// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCHER_H_
#define ORBIT_GL_BATCHER_H_

#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/TranslationStack.h"

namespace orbit_gl {

// Collects primitives to be rendered at a later point in time.
//
// This abstract class extends BatcherInterface by taking care of holding batcher_id and
// translations that will be called by CaptureViewElements.
class Batcher : public BatcherInterface {
 public:
  explicit Batcher(BatcherId batcher_id) : batcher_id_(batcher_id) {}

  [[nodiscard]] BatcherId GetBatcherId() const { return batcher_id_; }

  void PushTranslation(float x, float y, float z = 0.f) { translations_.PushTranslation(x, y, z); }
  void PopTranslation() { translations_.PopTranslation(); }

 protected:
  orbit_gl::TranslationStack translations_;

 private:
  BatcherId batcher_id_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_BATCHER_H_
