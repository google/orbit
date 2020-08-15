// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_STATUS_LISTENER_H_
#define ORBIT_GL_STATUS_LISTENER_H_

/**
 * This interface is used to communicate status of a background
 * task to the UI. Since there could be multiple background tasks
 * working at once the caller should keep track of status ids
 * and use them to update or clear status messages.
 *
 * The implementation of this class is not supposed to be thread-safe
 * it assumes methods are invoked on the main thread.
 *
 * Example usage:
 *
 * uint64_t status_id = listener->AddStatus("");
 */
class StatusListener {
 public:
  virtual ~StatusListener() = default;

  [[nodiscard]] virtual uint64_t AddStatus(std::string message) = 0;
  virtual void ClearStatus(uint64_t status_id) = 0;
  virtual void UpdateStatus(uint64_t status_id, std::string message) = 0;
};

#endif  // ORBIT_GL_STATUS_LISTENER_H_
