// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QApplication>
#include <list>

#include "AccessibilityAdapter.h"
#include "orbitglwidget.h"

namespace orbit_qt_tests {

using orbit_gl::GlA11yControlInterface, orbit_gl::A11yRole, orbit_gl::A11yRect, orbit_gl::A11yState;
using orbit_qt::A11yAdapter;

class TestA11yImpl : public GlA11yControlInterface {
 public:
  TestA11yImpl(TestA11yImpl* parent) : parent_(parent) {}
  [[nodiscard]] int AccessibleChildCount() const override { return children_.size(); }
  [[nodiscard]] GlA11yControlInterface* AccessibleChild(int index) const override {
    return children_[index].get();
  }

  [[nodiscard]] GlA11yControlInterface* AccessibleParent() const { return parent_; }
  [[nodiscard]] A11yRole AccessibleRole() const { return A11yRole::Grouping; }
  [[nodiscard]] A11yState AccessibleState() const { return A11yState(); }

  [[nodiscard]] A11yRect AccessibleLocalRect() const {
    orbit_gl::A11yRect result;
    if (parent_ == nullptr) {
      return result;
    }

    int parent_idx = -1;
    for (int i = 0; i < parent_->children_.size(); ++i) {
      if (parent_->children_[i].get() == this) {
        parent_idx = i;
        break;
      }
    }

    result.left = 0;
    result.top = parent_idx;
    result.width = 1000;
    result.height = 1;
    return result;
  }

  [[nodiscard]] std::string AccessibleName() const { return "Test"; }

  [[nodiscard]] std::vector<std::unique_ptr<TestA11yImpl>>& Children() { return children_; }

 private:
  std::vector<std::unique_ptr<TestA11yImpl>> children_;
  TestA11yImpl* parent_ = nullptr;
};

TEST(Accessibility, CreationAndManagement) {
  TestA11yImpl obj(nullptr);
  QAccessibleInterface* a1 = A11yAdapter::GetOrCreateAdapter(&obj);
  EXPECT_TRUE(a1->isValid());

  QAccessibleInterface* a2 = A11yAdapter::GetOrCreateAdapter(&obj);
  EXPECT_TRUE(a2->isValid());
  EXPECT_EQ(a1, a2);

  // TODO: Clearing of existing adapters is missing
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

}  // namespace orbit_qt_tests