// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_RESULT_TYPE_H_
#define ORBIT_SSH_RESULT_TYPE_H_

namespace OrbitSsh {

// ResultType is a type which is used as return type for functions that can be
// used in a blocking and non blocking mode.
// * kSuccess indicates the function has returned successfully.
// * kError indicates an error occurred and the function wasn't able to
// return successfully
// * kAgain indicates that the function would would have blocked when used in a
// blocking mode. This function needs to be called again with the same arguments
// at a later time to return successfully.
// TODO(antonrohr) implement better ResultType that can hold a return type in
// case of kSuccess (maybe via outcome::result)
enum class ResultType { kSuccess, kError, kAgain };

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_RESULT_TYPE_H_