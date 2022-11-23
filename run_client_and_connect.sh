#!/bin/bash
# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

HOSTNAME=$1
PROCESS=$2

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
exec $DIR/build_default_relwithdebinfo/bin/Orbit --ssh_hostname="$HOSTNAME" --ssh_user="$USER" --ssh_key_path="$HOME/.ssh/id_ed25519" --ssh_known_host_path="$HOME/.ssh/known_hosts" --ssh_target_process="$PROCESS" --nodeploy
