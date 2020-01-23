#ifndef ORBIT_ORBITQT_GPPINSTANCE_H
#define ORBIT_ORBITQT_GPPINSTANCE_H

#include <QDateTime>
#include <QMetaType>
#include <string>
#include <vector>

struct GgpInstance {
  QString displayName;
  QString id;
  QString ipAddress;
  QDateTime lastUpdated;
  QString owner;
  QString pool;

  static std::vector<GgpInstance> GetListFromJson(const QByteArray& json);

  QString GetSummary() const;

  [[nodiscard]] friend bool operator==(const GgpInstance& lhs,
                                       const GgpInstance& rhs) {
    return std::tie(lhs.displayName, lhs.id, lhs.ipAddress, lhs.lastUpdated,
                    lhs.owner, lhs.pool) ==
           std::tie(rhs.displayName, rhs.id, rhs.ipAddress, rhs.lastUpdated,
                    rhs.owner, rhs.pool);
  }

  [[nodiscard]] friend bool operator!=(const GgpInstance& lhs,
                                       const GgpInstance& rhs) {
    return !(lhs == rhs);
  }
};

[[nodiscard]] bool cmpById(const GgpInstance& lhs, const GgpInstance& rhs);

Q_DECLARE_METATYPE(GgpInstance)

#endif  // ORBIT_ORBITQT_GPPINSTANCE_H