#!/bin/sh
# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -eu

# This script only emulates the real behaviour as much as needed.
# In the real tool the first CLI argument is a path to a debian package.
# But for testing here we expect this an executable file, that we can copy
# to the final destination without the need to unpack things.
#
# The script also expects a "signature" under $1.asc. We don't expect this
# to be a real cryptographic signature. Instead the signature is considered
# valid iff the first line consists of the word "SIGNATURE".

read line <"$1.asc"
if [ "$line" != "SIGNATURE" ]; then
  exit 2
fi

mkdir -p /opt/developer/tools
cp -v "$1" /opt/developer/tools/OrbitService
chmod +x /opt/developer/tools/OrbitService