// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORIBIT_GL_APPLICATION_OPTIONS_H_
#define ORIBIT_GL_APPLICATION_OPTIONS_H_

#include <string>

// The structure used to store Orbit Client Application options
// The default values are set by main() and passed over to App
// class initialization method.
struct ApplicationOptions {
  // The host and port of the collection service
  std::string asio_server_address;
};

#endif  // ORIBIT_GL_APPLICATION_OPTIONS_H_
