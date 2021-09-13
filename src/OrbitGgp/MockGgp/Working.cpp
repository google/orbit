// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <chrono>
#include <iostream>
#include <string_view>
#include <thread>

int GgpVersion(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Wrong amount of arguments" << std::endl;
    return 1;
  }
  if (std::string_view{argv[1]} != "version") {
    std::cout << "arguments are formatted wrong" << std::endl;
    return 1;
  }

  std::cout << "12345.1.67.0 Mon 12 Dec 2012 12:12:12 PM UTC" << std::endl;
  return 0;
}

int GgpInstanceList(int argc, char* argv[]) {
  if (argc < 4 || argc > 5) {
    std::cout << "Wrong amount of arguments" << std::endl;
    return 1;
  }
  if (std::string_view{argv[1]} != "instance" || std::string_view{argv[2]} != "list" ||
      std::string_view{argv[3]} != "-s") {
    std::cout << "arguments are formatted wrong" << std::endl;
    return 1;
  }

  if (argc == 5 && std::string_view{argv[4]} != "--all-reserved") {
    std::cout << "arguments are formatted wrong" << std::endl;
    return 1;
  }

  std::cout << R"([
 {
  "displayName": "displayName-1",
  "id": "id/of/instance1",
  "ipAddress": "123.456.789.012",
  "lastUpdated": "2012-12-12T12:12:12Z",
  "owner": "owner@",
  "pool": "pool-of-test_instance_1",
  "state": "RESERVED"
 },
 {
  "displayName": "displayName-2",
  "id": "id/of/instance2",
  "ipAddress": "123.456.789.012",
  "lastUpdated": "2012-12-12T12:12:12Z",
  "owner": "owner@",
  "pool": "pool-of-test_instance_2",
  "state": "CONFIGURING"
 }
])" << std::endl;
  return 0;
}

int GgpSshInit(int argc, char* argv[]) {
  if (argc != 6) {
    std::cout << "Wrong amount of arguments" << std::endl;
    return 1;
  }
  if (std::string_view{argv[1]} != "ssh" || std::string_view{argv[2]} != "init" ||
      std::string_view{argv[3]} != "-s" || std::string_view{argv[4]} != "--instance" ||
      std::string_view{argv[5]} != "instance/test/id") {
    std::cout << "arguments are wrong" << std::endl;
    return 1;
  }
  std::cout << R"({
 "host": "123.456.789.012",
 "keyPath": "example/path/to/a/key",
 "knownHostsPath": "example/path/to/known_hosts",
 "port": "12345",
 "user": "example_user"
})" << std::endl;
  return 0;
}

int GgpProjectList(int argc, char* argv[]) {
  if (argc != 4) {
    std::cout << "Wrong amount of arguments" << std::endl;
    return 1;
  }
  if (std::string_view{argv[1]} != "project" || std::string_view{argv[2]} != "list" ||
      std::string_view{argv[3]} != "-s") {
    std::cout << "arguments are formatted wrong" << std::endl;
    return 1;
  }
  std::cout << R"([
 {
  "displayName": "displayName-1",
  "id": "id/of/project1"
 },
 {
  "displayName": "displayName-2",
  "id": "id/of/project2"
 }
])" << std::endl;
  return 0;
}

int main(int argc, char* argv[]) {
  // This sleep is here for 2 reasons:
  // 1. The ggp cli which this program is mocking, does have quite a bit of delay, hence having a
  // delay in this mock program, mimics the behaviour of the real ggp cli more closely
  // 2. To test the timeout functionaliy in OrbitGgp::Client
  std::this_thread::sleep_for(std::chrono::milliseconds{50});

  if (argc <= 1) {
    std::cout << "Wrong amount of arguments" << std::endl;
    return 1;
  }

  if (std::string_view{argv[1]} == "version") {
    return GgpVersion(argc, argv);
  }

  if (std::string_view{argv[1]} == "ssh") {
    return GgpSshInit(argc, argv);
  }

  if (std::string_view{argv[1]} == "instance") {
    return GgpInstanceList(argc, argv);
  }

  if (std::string_view{argv[1]} == "project") {
    return GgpProjectList(argc, argv);
  }

  std::cout << "arguments are formatted wrong" << std::endl;
  return 1;
}