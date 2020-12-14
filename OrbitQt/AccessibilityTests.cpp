// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAccessibleWidget>
#include <QApplication>
#include <list>

#include "AccessibilityAdapter.h"
#include "AccessibleObjectFake.h"
#include "orbitglwidget.h"

namespace orbit_qt {

using orbit_gl::AccessibilityRect;
using orbit_gl::AccessibilityRole;
using orbit_gl::AccessibilityState;
using orbit_gl::AccessibleObjectFake;
using orbit_gl::GlAccessibleInterface;

TEST(Accessibility, CreationAndManagement) {
  auto obj = std::make_unique<AccessibleObjectFake>(nullptr);
  QAccessibleInterface* a1 = AccessibilityAdapter::GetOrCreateAdapter(obj.get());
  EXPECT_TRUE(a1->isValid());

  QAccessibleInterface* a2 = AccessibilityAdapter::GetOrCreateAdapter(obj.get());
  EXPECT_TRUE(a2->isValid());
  EXPECT_EQ(a1, a2);

  obj.reset();
  EXPECT_EQ(AccessibilityAdapter::RegisteredAdapterCount(), 0);
}

TEST(Accessibility, ExternalWidget) {
  QObject object;

  /*
   * This should usually happen automatically - OrbitGlWidgetAccessible will register
   * and unregister on construction and deletion. This can currently not be tested though because we
   * cannot create a OrbitGlWidgetAccessible without an OrbitGlWidget, and this has too many
   * dependencies to be instantiated in a unit test at this point
   */
  std::unique_ptr<AccessibleObjectFake> obj = std::make_unique<AccessibleObjectFake>(nullptr);
  AccessibilityAdapter::RegisterAdapter(obj.get(), QAccessible::queryAccessibleInterface(&object));
  EXPECT_EQ(AccessibilityAdapter::RegisteredAdapterCount(), 1);
  AccessibilityAdapter::QAccessibleDeleted(QAccessible::queryAccessibleInterface(&object));
  EXPECT_EQ(AccessibilityAdapter::RegisteredAdapterCount(), 0);
}

TEST(Accessibility, Hierarchy) {
  AccessibleObjectFake root(nullptr);
  root.Children().push_back(std::make_unique<AccessibleObjectFake>(&root));
  root.Children().push_back(std::make_unique<AccessibleObjectFake>(&root));

  QAccessibleInterface* root_adapter = AccessibilityAdapter::GetOrCreateAdapter(&root);
  EXPECT_EQ(root_adapter->text(QAccessible::Text::Name),
            QString::fromStdString(root.AccessibleName()));
  EXPECT_EQ(root_adapter->role(), static_cast<QAccessible::Role>(root.AccessibleRole()));
  EXPECT_EQ(root_adapter->childCount(), 2);
  EXPECT_TRUE(root_adapter->child(0)->isValid());
  EXPECT_TRUE(root_adapter->child(1)->isValid());
  EXPECT_EQ(root_adapter->child(0),
            AccessibilityAdapter::GetOrCreateAdapter(root.AccessibleChild(0)));
  EXPECT_EQ(root_adapter->child(1),
            AccessibilityAdapter::GetOrCreateAdapter(root.AccessibleChild(1)));
  EXPECT_EQ(root_adapter->childAt(0, 0),
            AccessibilityAdapter::GetOrCreateAdapter(root.Children()[0].get()));
  EXPECT_EQ(root_adapter->childAt(0, 1),
            AccessibilityAdapter::GetOrCreateAdapter(root.Children()[1].get()));

  EXPECT_EQ(root_adapter->child(0)->parent(), root_adapter);
  EXPECT_EQ(root_adapter->child(1)->parent(), root_adapter);
  EXPECT_EQ(root_adapter->indexOfChild(root_adapter->child(0)), 0);
  EXPECT_EQ(root_adapter->indexOfChild(root_adapter->child(1)), 1);
}

}  // namespace orbit_qt