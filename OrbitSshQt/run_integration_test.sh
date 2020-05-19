#!/bin/bash
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [ -z "$1" ]; then
  echo "Please provide a path to the integration test executable"
  exit 1
fi

ggp ssh init -s | jq '(.host + " " + .port + " " + .user + " " + .knownHostsPath + " " + .keyPath)' | tr -d '"' | sed -e 's|\\\\|/|g' | xargs "$1"
