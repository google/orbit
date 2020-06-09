// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GPP_INSTANCE_H_
#define ORBIT_GPP_INSTANCE_H_

#include <QDateTime>
#include <QMetaType>
#include <QVector>
#include <outcome.hpp>

namespace OrbitGgp {

struct Instance {
  QString display_name;
  QString id;
  QString ip_address;
  QDateTime last_updated;
  QString owner;
  QString pool;

  static outcome::result<QVector<Instance>> GetListFromJson(
      const QByteArray& json);
  static bool CmpById(const Instance& lhs, const Instance& rhs);

  friend bool operator==(const Instance& lhs, const Instance& rhs) {
    return std::tie(lhs.display_name, lhs.id, lhs.ip_address, lhs.last_updated,
                    lhs.owner, lhs.pool) ==
           std::tie(rhs.display_name, rhs.id, rhs.ip_address, rhs.last_updated,
                    rhs.owner, rhs.pool);
  }

  friend bool operator!=(const Instance& lhs, const Instance& rhs) {
    return !(lhs == rhs);
  }
};

}  // namespace OrbitGgp

Q_DECLARE_METATYPE(OrbitGgp::Instance)

#endif  // ORBIT_GPP_INSTANCE_H_