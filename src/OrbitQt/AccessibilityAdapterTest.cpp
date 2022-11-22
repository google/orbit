// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAccessible>
#include <QAccessibleInterface>
#include <QString>
#include <memory>
#include <vector>

#include "OrbitAccessibility/AccessibleObjectFake.h"
#include "OrbitQt/AccessibilityAdapter.h"

namespace orbit_qt {

using orbit_accessibility::AccessibleObjectFake;

TEST(AdapterRegistry, CreationAndManagement) {
  InstallAccessibilityFactories();

  auto obj = std::make_unique<AccessibleObjectFake>(nullptr);
  QAccessibleInterface* a1 = AdapterRegistry::Get().GetOrCreateAdapter(obj.get());
  EXPECT_TRUE(a1->isValid());

  QAccessibleInterface* a2 = AdapterRegistry::Get().GetOrCreateAdapter(obj.get());
  EXPECT_TRUE(a2->isValid());
  EXPECT_EQ(a1, a2);
}

TEST(AccessibilityAdapter, Hierarchy) {
  InstallAccessibilityFactories();

  AccessibleObjectFake root(nullptr);
  root.Children().push_back(std::make_unique<AccessibleObjectFake>(&root));
  root.Children().push_back(std::make_unique<AccessibleObjectFake>(&root));

  QAccessibleInterface* root_adapter = AdapterRegistry::Get().GetOrCreateAdapter(&root);
  EXPECT_EQ(root_adapter->text(QAccessible::Text::Name),
            QString::fromStdString(root.AccessibleName()));
  EXPECT_EQ(root_adapter->role(), static_cast<QAccessible::Role>(root.AccessibleRole()));
  EXPECT_EQ(root_adapter->childCount(), 2);
  EXPECT_TRUE(root_adapter->child(0)->isValid());
  EXPECT_TRUE(root_adapter->child(1)->isValid());
  EXPECT_EQ(root_adapter->child(0),
            AdapterRegistry::Get().GetOrCreateAdapter(root.AccessibleChild(0)));
  EXPECT_EQ(root_adapter->child(1),
            AdapterRegistry::Get().GetOrCreateAdapter(root.AccessibleChild(1)));
  EXPECT_EQ(root_adapter->childAt(0, 0),
            AdapterRegistry::Get().GetOrCreateAdapter(root.Children()[0].get()));
  EXPECT_EQ(root_adapter->childAt(0, 1),
            AdapterRegistry::Get().GetOrCreateAdapter(root.Children()[1].get()));

  EXPECT_EQ(root_adapter->child(0)->parent(), root_adapter);
  EXPECT_EQ(root_adapter->child(1)->parent(), root_adapter);
  EXPECT_EQ(root_adapter->indexOfChild(root_adapter->child(0)), 0);
  EXPECT_EQ(root_adapter->indexOfChild(root_adapter->child(1)), 1);
}

}  // namespace orbit_qt