// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <cstdio>

int main(int argc, char* argv[]) {
  printf("Running main() from %s\n", __FILE__);
  ::testing::InitGoogleTest(&argc, argv);

  QCoreApplication app{argc, argv};
  int result = RUN_ALL_TESTS();
  return result;
}