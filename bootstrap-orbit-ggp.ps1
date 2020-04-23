# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

& "$PSScriptRoot\bootstrap-orbit.ps1" "ggp_release"

Write-Host @'

---------------------------------------------------------------------------------------
- By the way: "bootstrap-orbit-ggp.*" is not needed anymore and will be removed soon. -
- Please use "bootstrap-orbit.ps1 ggp_release" instead.                               -
---------------------------------------------------------------------------------------
'@
