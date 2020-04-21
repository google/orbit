#ifndef ORBITQT_GPP_INSTANCE_H_
#define ORBITQT_GPP_INSTANCE_H_

#include <QDateTime>
#include <QMetaType>
#include <QVector>
#include <string>

struct GgpInstance {
  QString display_name;
  QString id;
  QString ip_address;
  QDateTime last_updated;
  QString owner;
  QString pool;

  static QVector<GgpInstance> GetListFromJson(const QByteArray& json);
  static bool CmpById(const GgpInstance& lhs, const GgpInstance& rhs);

  friend bool operator==(const GgpInstance& lhs, const GgpInstance& rhs) {
    return std::tie(lhs.display_name, lhs.id, lhs.ip_address, lhs.last_updated,
                    lhs.owner, lhs.pool) ==
           std::tie(rhs.display_name, rhs.id, rhs.ip_address, rhs.last_updated,
                    rhs.owner, rhs.pool);
  }

  friend bool operator!=(const GgpInstance& lhs, const GgpInstance& rhs) {
    return !(lhs == rhs);
  }
};

Q_DECLARE_METATYPE(GgpInstance)

#endif  // ORBITQT_GPP_INSTANCE_H_