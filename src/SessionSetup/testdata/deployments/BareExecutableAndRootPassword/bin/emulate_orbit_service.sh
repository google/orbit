#!/bin/sh
# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -eu

# OrbitService needs to run as root, so we abort here if our UID is not 0.
if [ $(id -u) -ne 0 ]; then
  echo "OrbitService needs to run as root" >&2
  exit 42
fi

# This is the usual echo server that we have been using for testing in other cases as well.
# It needs to run in the background, but we also need to get access to its stderr output.
# So we use a FIFO for that.
FIFO_DIR="$(mktemp -d)"
FIFO="$FIFO_DIR/stderr"
trap "rm -f $FIFO; rmdir $FIFO_DIR" EXIT
mkfifo $FIFO || exit 43

socat -dd TCP-LISTEN:44765,fork exec:'/bin/cat' 2>$FIFO &
SOCAT_PID=$!
trap "rm -f $FIFO; rmdir $FIFO_DIR; kill $SOCAT_PID" EXIT

# We have to wait until socat has printed it's first debug output line because this
# happens when the port is open. Afterwards we can print the "READY" keyword.
while read -r line; do
  break
done <$FIFO

echo "READY"

# OrbitService shuts itself down when STDIN gets closed. So we simulate the same behaviour here.
while read line; do
  # Do nothing
  :
done