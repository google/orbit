#!/bin/bash

# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# A pre-commit hook that applies clang-format-9 to all the changes
# (staged and unstaged).
# After the formatting has been applied, it will ask you which changes should

echo "Formatting all changed files now..."
git clang-format-14 -f
# Only show this to the user of there are unstaged changes.
if ! (git diff -- --quiet)
then
  exec < /dev/tty
  echo "\n\n"
  echo "Please select the changes to be included in this commit ('a' in include all):"
  git add -p .
fi

