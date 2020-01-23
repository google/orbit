#include "ggpinstance.h"
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <iterator>

namespace {
GgpInstance GetInstanceFromJson(const QJsonObject& obj) {
  GgpInstance inst{};

  if (const auto val = obj.value("displayName"); !val.isUndefined()) {
    inst.displayName = val.toString();
  }

  if (const auto val = obj.value("id"); !val.isUndefined()) {
    inst.id = val.toString();
  }

  if (const auto val = obj.value("ipAddress"); !val.isUndefined()) {
    inst.ipAddress = val.toString();
  }

  if (const auto val = obj.value("lastUpdated"); !val.isUndefined()) {
    inst.lastUpdated = QDateTime::fromString(val.toString(), Qt::ISODate);
  }

  if (const auto val = obj.value("owner"); !val.isUndefined()) {
    inst.owner = val.toString();
  }

  if (const auto val = obj.value("pool"); !val.isUndefined()) {
    inst.pool = val.toString();
  }

  return inst;
}
}  // namespace

std::vector<GgpInstance> GgpInstance::GetListFromJson(const QByteArray& json) {
  const auto doc = QJsonDocument::fromJson(json);

  if (!doc.isArray()) return {};

  const auto arr = doc.array();

  std::vector<GgpInstance> list;

  std::transform(arr.begin(), arr.end(), std::back_inserter(list),
                 [](const QJsonValue& val) -> GgpInstance {
                   if (!val.isObject()) return {};

                   const auto obj = val.toObject();

                   return GetInstanceFromJson(obj);
                 });

  return list;
}

QString GgpInstance::GetSummary() const {
  return QString("%1 in %2 by %3").arg(ipAddress, pool, owner);
}

bool cmpById(const GgpInstance &lhs, const GgpInstance &rhs) {
  return lhs.id < rhs.id;
}