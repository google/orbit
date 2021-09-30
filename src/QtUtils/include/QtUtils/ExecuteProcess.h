// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QT_UTILS_EXECUTE_PROCESS_H_
#define QT_UTILS_EXECUTE_PROCESS_H_

#include <absl/time/time.h>

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>

#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"

namespace orbit_qt_utils {

constexpr absl::Duration kExecuteProcessDefaultTimeout = absl::Seconds(5);

orbit_base::Future<ErrorMessageOr<QByteArray>> ExecuteProcess(
    const QString& program, const QStringList& arguments, QObject* parent = nullptr,
    absl::Duration timeout = kExecuteProcessDefaultTimeout);

}  // namespace orbit_qt_utils

#endif  // QT_UTILS_EXECUTE_PROCESS_H_