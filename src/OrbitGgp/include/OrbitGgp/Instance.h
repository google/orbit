// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GPP_INSTANCE_H_
#define ORBIT_GPP_INSTANCE_H_

#include <QByteArray>
#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QVector>
#include <tuple>

#include "OrbitBase/Result.h"

namespace orbit_ggp {

struct Instance {
  QString display_name;
  QString id;
  QString ip_address;
  QDateTime last_updated;
  QString owner;
  QString pool;
  QString state;

  static ErrorMessageOr<QVector<Instance>> GetListFromJson(const QByteArray& json);
  static ErrorMessageOr<Instance> CreateFromJson(const QByteArray& json);
  static bool CmpById(const Instance& lhs, const Instance& rhs);

  friend bool operator==(const Instance& lhs, const Instance& rhs) {
    return std::tie(lhs.display_name, lhs.id, lhs.ip_address, lhs.last_updated, lhs.owner, lhs.pool,
                    lhs.state) == std::tie(rhs.display_name, rhs.id, rhs.ip_address,
                                           rhs.last_updated, rhs.owner, rhs.pool, rhs.state);
  }

  friend bool operator!=(const Instance& lhs, const Instance& rhs) { return !(lhs == rhs); }
};

}  // namespace orbit_ggp

Q_DECLARE_METATYPE(orbit_ggp::Instance)

#endif  // ORBIT_GPP_INSTANCE_H_