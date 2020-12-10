// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAccessibleWidget>
#include <QApplication>
#include <list>

#include "AccessibilityAdapter.h"
#include "AccessibilityInterfaceMock.h"
#include "orbitglwidget.h"

namespace orbit_qt {

using orbit_gl::GlAccessibleInterface, orbit_gl::A11yRole, orbit_gl::A11yRect, orbit_gl::A11yState;
using orbit_gl::TestA11yImpl;

TEST(Accessibility, CreationAndManagement) {
  TestA11yImpl* obj = new TestA11yImpl(nullptr);
  QAccessibleInterface* a1 = A11yAdapter::GetOrCreateAdapter(obj);
  EXPECT_TRUE(a1->isValid());

  QAccessibleInterface* a2 = A11yAdapter::GetOrCreateAdapter(obj);
  EXPECT_TRUE(a2->isValid());
  EXPECT_EQ(a1, a2);

  delete obj;
  EXPECT_EQ(A11yAdapter::RegisteredAdapterCount(), 0);
}

TEST(Accessibility, ExternalWidget) {
  QObject object;

  // This should usually happen automatically - OrbitGlWidgetAccessible will register
  // and unregister on construction and deletion. This can currently not be tested though because we
  // cannot create a OrbitGlWidgetAccessible without an OrbitGlWidget, and this has too many
  // dependencies to be instantiated in a unit test at this point
  TestA11yImpl* obj = new TestA11yImpl(nullptr);
  A11yAdapter::RegisterAdapter(obj, QAccessible::queryAccessibleInterface(&object));
  EXPECT_EQ(A11yAdapter::RegisteredAdapterCount(), 1);
  A11yAdapter::QAccessibleDeleted(QAccessible::queryAccessibleInterface(&object));
  EXPECT_EQ(A11yAdapter::RegisteredAdapterCount(), 0);
  delete obj;
}

TEST(Accessibility, Hierarchy) {
  TestA11yImpl root(nullptr);
  root.Children().push_back(std::make_unique<TestA11yImpl>(&root));
  root.Children().push_back(std::make_unique<TestA11yImpl>(&root));

  QAccessibleInterface* root_adapter = A11yAdapter::GetOrCreateAdapter(&root);
  EXPECT_EQ(root_adapter->text(QAccessible::Text::Name),
            QString::fromStdString(root.AccessibleName()));
  EXPECT_EQ(root_adapter->role(), static_cast<QAccessible::Role>(root.AccessibleRole()));
  EXPECT_EQ(root_adapter->childCount(), 2);
  EXPECT_TRUE(root_adapter->child(0)->isValid());
  EXPECT_TRUE(root_adapter->child(1)->isValid());
  EXPECT_EQ(root_adapter->child(0), A11yAdapter::GetOrCreateAdapter(root.AccessibleChild(0)));
  EXPECT_EQ(root_adapter->child(1), A11yAdapter::GetOrCreateAdapter(root.AccessibleChild(1)));
  EXPECT_EQ(root_adapter->childAt(0, 0), A11yAdapter::GetOrCreateAdapter(root.Children()[0].get()));
  EXPECT_EQ(root_adapter->childAt(0, 1), A11yAdapter::GetOrCreateAdapter(root.Children()[1].get()));

  EXPECT_EQ(root_adapter->child(0)->parent(), root_adapter);
  EXPECT_EQ(root_adapter->child(1)->parent(), root_adapter);
  EXPECT_EQ(root_adapter->indexOfChild(root_adapter->child(0)), 0);
  EXPECT_EQ(root_adapter->indexOfChild(root_adapter->child(1)), 1);
}

}  // namespace orbit_qt