#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

"${DIR}/bootstrap-orbit.sh" ggp_release

echo ''
echo '----------------------------------------------------------------------------------------'
echo '- By the way: "bootstrap-orbit-ggp.sh" is not needed anymore and will be removed soon. -'
echo '- Please use "bootstrap-orbit.sh ggp_release" instead.                                 -'
echo '----------------------------------------------------------------------------------------'
