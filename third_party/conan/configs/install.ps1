# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

param (
  [switch]$recreate_default_profiles = $false
)

$conan_dir = if ( $Env:CONAN_USER_HOME ) { $Env:CONAN_USER_HOME } else { "$HOME\.conan" }
$profiles_dir = "$conan_dir\profiles"

if ($recreate_default_profiles) {
  Remove-Item $profiles_dir\* -Include default,default_release,default_relwithdebinfo,default_debug -Force
}

function conan {
  & py -3 -m conans.conan $args
}

# Installs conan config (build settings, public remotes, conan profiles, conan options).
# Have a look in \third_party\conan\configs\windows for more information.
conan config install "$PSScriptRoot\windows"
if ($LastExitCode -ne 0) {
   Throw "Error while installing conan config."
}
