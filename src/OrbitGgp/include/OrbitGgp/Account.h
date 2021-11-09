// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GPP_ACCOUNT_H_
#define ORBIT_GPP_ACCOUNT_H_

#include <QString>

#include "OrbitBase/Result.h"

namespace orbit_ggp {

struct Account {
  QString email;

  static ErrorMessageOr<Account> GetDefaultAccountFromJson(const QByteArray& json);
};

}  // namespace orbit_ggp

#endif  // ORBIT_GPP_ACCOUNT_H_