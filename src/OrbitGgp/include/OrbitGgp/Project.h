// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GPP_PROJECT_H_
#define ORBIT_GPP_PROJECT_H_

#include <QByteArray>
#include <QString>
#include <QVector>

#include "OrbitBase/Result.h"

namespace orbit_ggp {

struct Project {
  QString display_name;
  QString id;

  static ErrorMessageOr<QVector<Project>> GetListFromJson(const QByteArray& json);
};

}  // namespace orbit_ggp

#endif  // ORBIT_GPP_PROJECT_H_
