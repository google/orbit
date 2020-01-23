#include "ggpinstanceitemmodel.h"
#include <qabstractitemmodel.h>
#include <qnamespace.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <cassert>
#include "ggpinstance.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  std::vector<GgpInstance> demoInstances{};
  {
    GgpInstance instance{};
    instance.displayName = "displayName1";
    instance.id = "id1";
    instance.ipAddress = "10.10.0.1";
    instance.lastUpdated =
        QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
    instance.owner = "hebecker@";
    instance.pool = "fra-gen1-anything";
    demoInstances.emplace_back(instance);
  }
  {
    GgpInstance instance{};
    instance.displayName = "displayName2";
    instance.id = "id2";
    instance.ipAddress = "10.10.0.2";
    instance.lastUpdated =
        QDateTime::fromString("2020-02-02T00:42:42Z", Qt::ISODate);
    instance.owner = "programmer@";
    instance.pool = "fra-gen42-anything";
    demoInstances.emplace_back(instance);
  }

  GgpInstanceItemModel model{demoInstances};

  assert(model.rowCount() == demoInstances.size());
  assert(model.columnCount() == 6);
  assert(model.index(0, 0, {}).isValid());
  assert(model.index(1, 0, {}).isValid());
  assert(!model.index(2, 0, {}).isValid());
  assert(!model.index(0, 6, {}).isValid());
  assert(model.index(0, 5, {}).isValid());

  const auto firstCell = model.index(0, 0, {});
  assert(firstCell.isValid());
  assert(!model.index(0, 0, firstCell).isValid());

  const auto firstCellUserData = firstCell.data(Qt::UserRole);
  assert(firstCellUserData.userType() == qMetaTypeId<GgpInstance>());

  const auto firstGgpInstance = firstCellUserData.value<GgpInstance>();
  assert(firstGgpInstance == demoInstances.front());

  assert(firstCell.data().toString() == firstGgpInstance.displayName);

  const auto compareCell = [&](int row, int col, const auto& val) -> bool {
    const auto cell = model.index(row, col);
    return cell.isValid() && cell.data().toString() == val;
  };

  assert(compareCell(0, 1, firstGgpInstance.id));
  assert(compareCell(0, 2, firstGgpInstance.ipAddress));
  assert(compareCell(
      0, 3, firstGgpInstance.lastUpdated.toString(Qt::TextDate)));
  assert(compareCell(0, 4, firstGgpInstance.owner));
  assert(compareCell(0, 5, firstGgpInstance.pool));

  const auto secondCell = model.index(1, 0, {});
  assert(secondCell.isValid());

  const auto secondCellUserData = secondCell.data(Qt::UserRole);
  assert(secondCellUserData.userType() == qMetaTypeId<GgpInstance>());

  const auto secondGgpInstance = secondCellUserData.value<GgpInstance>();
  assert(secondGgpInstance == demoInstances.at(1));

  assert(compareCell(1, 1, secondGgpInstance.id));
  assert(compareCell(1, 2, secondGgpInstance.ipAddress));
  assert(compareCell(
      1, 3,
      secondGgpInstance.lastUpdated.toString(Qt::TextDate)));
  assert(compareCell(1, 4, secondGgpInstance.owner));
  assert(compareCell(1, 5, secondGgpInstance.pool));

  int dataChangedCounter = 0;
  int rowsAddedCounter = 0;
  int rowsRemovedCounter = 0;

  const auto resetCounters = [&]() {
    dataChangedCounter = 0;
    rowsAddedCounter = 0;
    rowsRemovedCounter = 0;
  };

  QObject::connect(&model, &QAbstractItemModel::dataChanged, &model,
                   [&]() { ++dataChangedCounter; });
  QObject::connect(&model, &QAbstractItemModel::rowsInserted, &model,
                   [&]() { ++rowsAddedCounter; });
  QObject::connect(&model, &QAbstractItemModel::rowsRemoved, &model,
                   [&]() { ++rowsRemovedCounter; });

  model.setInstances(demoInstances);
  assert(dataChangedCounter == 0);
  assert(rowsAddedCounter == 0);
  assert(rowsRemovedCounter == 0);

  {
    GgpInstance instance{};
    instance.displayName = "displayName3";
    instance.id = "id3";
    instance.ipAddress = "10.10.0.3";
    instance.lastUpdated =
        QDateTime::fromString("2020-03-03T00:42:42Z", Qt::ISODate);
    instance.owner = "me@";
    instance.pool = "fra-gen42-anything";
    demoInstances.emplace_back(instance);
  }

  model.setInstances(demoInstances);
  assert(dataChangedCounter == 0);
  assert(rowsAddedCounter == 1);
  assert(rowsRemovedCounter == 0);

  resetCounters();
  demoInstances[0].displayName = "Another display name";
  model.setInstances(demoInstances);
  assert(dataChangedCounter == 1);
  assert(rowsAddedCounter == 0);
  assert(rowsRemovedCounter == 0);

  {
    GgpInstance instance{};
    instance.displayName = "displayName4";
    instance.id = "id11";
    instance.ipAddress = "10.10.0.4";
    instance.lastUpdated =
        QDateTime::fromString("2020-03-03T00:42:42Z", Qt::ISODate);
    instance.owner = "me@";
    instance.pool = "fra-gen42-anything";
    demoInstances.emplace_back(instance);
  }

  resetCounters();
  demoInstances[1].displayName = "Another display name2";
  model.setInstances(demoInstances);
  assert(dataChangedCounter == 1);
  assert(rowsAddedCounter == 1);
  assert(rowsRemovedCounter == 0);

  demoInstances.erase(demoInstances.begin());
  resetCounters();
  model.setInstances(demoInstances);
  assert(dataChangedCounter == 0);
  assert(rowsAddedCounter == 0);
  assert(rowsRemovedCounter == 1);

  {
    GgpInstance instance{};
    instance.displayName = "displayName5";
    instance.id = "id112";
    instance.ipAddress = "10.10.0.5";
    instance.lastUpdated =
        QDateTime::fromString("2020-03-03T00:42:42Z", Qt::ISODate);
    instance.owner = "me@";
    instance.pool = "fra-gen42-anything";
    demoInstances.insert(demoInstances.begin() + 2, instance);
  }
  {
    GgpInstance instance{};
    instance.displayName = "displayName6";
    instance.id = "id42";
    instance.ipAddress = "10.10.0.42";
    instance.lastUpdated =
        QDateTime::fromString("2020-03-03T00:42:42Z", Qt::ISODate);
    instance.owner = "me@";
    instance.pool = "fra-gen42-anything";
    demoInstances.insert(demoInstances.begin() + 1, instance);
  }

  demoInstances.erase(demoInstances.begin());

  resetCounters();
  model.setInstances(demoInstances);
  assert(dataChangedCounter == 0);
  assert(rowsAddedCounter == 2);
  assert(rowsRemovedCounter == 1);

  qInfo() << "Finished.";
}