// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_CONTROL_INTERFACE_H_
#define ORBIT_GL_TRACK_CONTROL_INTERFACE_H_

#include <string>

#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

// Interface to query and control track behavior. Used to decouple Track from TrackHeader.
// It is expected that TrackHeader will receive more functionality (e.g. hiding / pinning tracks) in
// the future, which will move more methods into this interface.
class TrackControlInterface {
 public:
  virtual ~TrackControlInterface() = default;

  [[nodiscard]] virtual bool IsPinned() const = 0;
  virtual void SetPinned(bool value) = 0;

  [[nodiscard]] virtual std::string GetLabel() const = 0;
  [[nodiscard]] virtual std::string GetName() const = 0;
  [[nodiscard]] virtual int GetNumberOfPrioritizedTrailingCharacters() const = 0;
  [[nodiscard]] virtual Color GetTrackBackgroundColor() const = 0;
  [[nodiscard]] virtual uint32_t GetIndentationLevel() const = 0;

  [[nodiscard]] virtual bool IsCollapsible() const = 0;
  [[nodiscard]] virtual bool Draggable() = 0;

  [[nodiscard]] virtual bool IsTrackSelected() const = 0;
  virtual void SelectTrack() = 0;

  virtual void DragBy(float delta_y) = 0;
};
}  // namespace orbit_gl

#endif