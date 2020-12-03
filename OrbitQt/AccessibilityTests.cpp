// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <list>

#include "GlAccessibility.h"

namespace orbit_qt {

class TestA11yImpl : public orbit_gl::GlA11yInterface {
 public:
  TestA11yImpl(TestA11yImpl* parent) : parent_(parent) {}
  int AccessibleChildCount() const override { return children_.size(); }
  TestA11yImpl* AccessibleChild(int index) const override { return children_[index].get(); }
  TestA11yImpl* AccessibleChildAt(int x, int y) const override {
    return y >= 0 && y < children_.size() ? children_[y].get() : nullptr;
  }

  TestA11yImpl* AccessibleParent() const { return parent_; }
  orbit_gl::A11yRole AccessibleRole() const { return orbit_gl::A11yRole::Grouping; }

  orbit_gl::A11yRect AccessibleLocalRect() const {
    orbit_gl::A11yRect result;

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

  std::string AccessibleName() const { return "Test"; }

  std::vector<std::unique_ptr<TestA11yImpl>>& Children() { return children_; }

 private:
  std::vector<std::unique_ptr<TestA11yImpl>> children_;
  TestA11yImpl* parent_ = nullptr;
};

TEST(Accessibility, CreationAndManagement) {
  TestA11yImpl obj(nullptr);
  A11yAdapter* a1 = A11yAdapter::GetOrCreateAdapter(&obj);
  EXPECT_TRUE(a1->isValid());

  A11yAdapter* a2 = A11yAdapter::GetOrCreateAdapter(&obj);
  EXPECT_TRUE(a2->isValid());
  EXPECT_EQ(a1, a2);

  // TODO: Clearing of existing adapters is missing
  A11yAdapter::ClearAdapterCache();
  a2 = A11yAdapter::GetOrCreateAdapter(&obj);
  EXPECT_TRUE(a2->isValid());
}

TEST(Accessibility, Hierarchy) {
  TestA11yImpl root(nullptr);
  root.Children().push_back(std::make_unique<TestA11yImpl>(&root));
  root.Children().push_back(std::make_unique<TestA11yImpl>(&root));

  A11yAdapter* root_adapter = A11yAdapter::GetOrCreateAdapter(&root);
  EXPECT_EQ(root_adapter->text(QAccessible::Text::Name),
            QString::fromStdString(root.AccessibleName()));
  EXPECT_EQ(root_adapter->role(), static_cast<QAccessible::Role>(root.AccessibleRole()));
  EXPECT_EQ(root_adapter->childCount(), 2);
  EXPECT_TRUE(root_adapter->child(0)->isValid());
  EXPECT_TRUE(root_adapter->child(1)->isValid());
  EXPECT_EQ(root_adapter->child(0), A11yAdapter::GetOrCreateAdapter(root.AccessibleChild(0)));
  EXPECT_EQ(root_adapter->child(1), A11yAdapter::GetOrCreateAdapter(root.AccessibleChild(1)));
  EXPECT_EQ(root_adapter->childAt(0, 0),
            A11yAdapter::GetOrCreateAdapter(root.AccessibleChildAt(0, 0)));
  EXPECT_EQ(root_adapter->childAt(0, 1),
            A11yAdapter::GetOrCreateAdapter(root.AccessibleChildAt(0, 1)));

  EXPECT_EQ(root_adapter->child(0)->parent(), root_adapter);
  EXPECT_EQ(root_adapter->child(1)->parent(), root_adapter);
  EXPECT_EQ(root_adapter->indexOfChild(root_adapter->child(0)), 0);
  EXPECT_EQ(root_adapter->indexOfChild(root_adapter->child(1)), 1);
}

}  // namespace orbit_qt