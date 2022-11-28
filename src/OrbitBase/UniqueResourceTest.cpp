// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>

#include <memory>
#include <type_traits>
#include <utility>

#include "OrbitBase/UniqueResource.h"

void my_delete(size_t) {}

TEST(UniqueResource, Construct) {
  orbit_base::unique_resource<size_t, void (*)(size_t)> ur1(123, my_delete);
  orbit_base::unique_resource ur2(123, my_delete);

  size_t was_deleted = 0;
  {
    orbit_base::unique_resource ur{123, [&was_deleted](size_t) { was_deleted++; }};
    ASSERT_TRUE(static_cast<bool>(ur));
    ASSERT_TRUE(static_cast<size_t>(ur.get()) == 123);
  }
  ASSERT_EQ(was_deleted, 1);
}

TEST(UniqueResource, Move) {
  size_t was_deleted = 0;
  {
    orbit_base::unique_resource ur1{123, [&was_deleted](size_t) { was_deleted++; }};
    ASSERT_TRUE(static_cast<bool>(ur1));

    {
      auto ur2 = std::move(ur1);
      ASSERT_TRUE(static_cast<bool>(ur2));
      ASSERT_FALSE(static_cast<bool>(ur1));
      ASSERT_EQ(was_deleted, 0);
    }
    ASSERT_EQ(was_deleted, 1);
  }
  ASSERT_EQ(was_deleted, 1);
}

TEST(UniqueResource, Release) {
  size_t was_deleted = 0;
  {
    orbit_base::unique_resource ur1{123, [&was_deleted](size_t) { was_deleted++; }};
    ur1.release();
  }
  ASSERT_EQ(was_deleted, 0);
}

TEST(UniqueResource, Reset) {
  size_t last_deleted_resource = 0;
  {
    orbit_base::unique_resource ur1{
        123, [&last_deleted_resource](size_t r) { last_deleted_resource = r; }};
    ur1.reset(456);
    ASSERT_EQ(last_deleted_resource, 123);
  }
  ASSERT_EQ(last_deleted_resource, 456);
}