# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

$py = Get-Command py -ErrorAction Stop

# Installs conan config (build settings, public remotes, conan profiles, conan options). Have a look in \third_party\conan\configs\windows for more information.
$process = Start-Process $py.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "-3","-m","conans.conan","config","install","$PSScriptRoot\windows"
if ($process.ExitCode -ne 0) { Throw "Error while installing conan config." }
