// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitClientData/FunctionInfoSet.h"
#include "capture_data.pb.h"

using orbit_client_protos::FunctionInfo;

TEST(FunctionInfoSet, EqualFunctions) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.set_name("foo");
  right.set_pretty_name("void foo()");
  right.set_loaded_module_path("/path/to/module");
  right.set_module_base_address(42);
  right.set_address(12);
  right.set_load_bias(4);
  right.set_size(16);
  right.set_file("file.cpp");
  right.set_line(13);

  internal::EqualFunctionInfo eq;
  EXPECT_TRUE(eq(left, right));
  internal::HashFunctionInfo hash;
  EXPECT_EQ(hash(left), hash(right));
}

TEST(FunctionInfoSet, DifferentName) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_name("bar");

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, DifferentPrettyName) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_pretty_name("void bar()");

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, DifferentLoadedModulePath) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_loaded_module_path("/path/to/other");

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, DifferentModuleBaseAddress) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_module_base_address(43);

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, DifferentAddress) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_address(14);

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, DifferentLoadBias) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_load_bias(3);

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, DifferentSize) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_size(15);

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, DifferentFile) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_file("other.cpp");

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, DifferentLine) {
  FunctionInfo left;
  left.set_name("foo");
  left.set_pretty_name("void foo()");
  left.set_loaded_module_path("/path/to/module");
  left.set_module_base_address(42);
  left.set_address(12);
  left.set_load_bias(4);
  left.set_size(16);
  left.set_file("file.cpp");
  left.set_line(13);

  FunctionInfo right;
  right.CopyFrom(left);
  right.set_line(12);

  internal::EqualFunctionInfo eq;
  EXPECT_FALSE(eq(left, right));
}

TEST(FunctionInfoSet, Insertion) {
  FunctionInfo function;
  function.set_name("foo");
  function.set_pretty_name("void foo()");
  function.set_loaded_module_path("/path/to/module");
  function.set_module_base_address(42);
  function.set_address(12);
  function.set_load_bias(4);
  function.set_size(16);
  function.set_file("file.cpp");
  function.set_line(13);

  FunctionInfoSet functions;
  EXPECT_FALSE(functions.contains(function));
  functions.insert(function);
  EXPECT_TRUE(functions.contains(function));
  EXPECT_EQ(functions.size(), 1);

  FunctionInfo other;
  EXPECT_FALSE(functions.contains(other));
}

TEST(FunctionInfoSet, Deletion) {
  FunctionInfo function;
  function.set_name("foo");
  function.set_pretty_name("void foo()");
  function.set_loaded_module_path("/path/to/module");
  function.set_module_base_address(42);
  function.set_address(12);
  function.set_load_bias(4);
  function.set_size(16);
  function.set_file("file.cpp");
  function.set_line(13);

  FunctionInfoSet functions;
  functions.insert(function);
  EXPECT_TRUE(functions.contains(function));
  EXPECT_EQ(functions.size(), 1);

  FunctionInfo other;
  EXPECT_FALSE(functions.contains(other));
  functions.erase(other);
  EXPECT_FALSE(functions.contains(other));
  EXPECT_EQ(functions.size(), 1);

  functions.erase(function);
  EXPECT_FALSE(functions.contains(function));
  EXPECT_EQ(functions.size(), 0);
}