# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

$conan_version_required = "1.58.0"

function Check-Conan-Version-Sufficient {
  $version = [System.Version]$args[0]
  $required = [System.Version]$args[1]
  return $version -ge $required
}

function conan {
  & py -3 -m conans.conan $args
}
function pip3 {
  & py -3 -m pip $args
}

conan --version >null 2>&1

if ($LastExitCode -ne 0) {
  Write-Host "Conan not found. Trying to install it via python-pip..."
	
	
  pip3 --version >null 2>&1

  if ($LastExitCode -ne 0) {
    Write-Error -ErrorAction Stop @"
It seems you don't have Python3 (or PIP) installed (py -3 -m pip --version fails).
Please install Python and make it available in the path.
"@
  }

  pip3 install --upgrade --user conan==$conan_version_required
  if ($LastExitCode -ne 0) {
    Throw "Error while installing conan via PIP."
  }
} else {
  Write-Host "Conan found. Checking version..."

  $conan_version = (conan --version).split(" ")[2]

  if (!Check-Conan-Version-Sufficient $conan_version $conan_version_required) {
    Write-Host "Your conan version $conan_version is too old. Let's try to update it."
    pip3 install --upgrade conan==$conan_version_required
    if ($LastExitCode -ne 0) {
      Throw "Error while upgrading conan via PIP. Probably you have conan installed differently." + 
            " Please manually update conan to a at least version $conan_version_required"
    }

    Write-Host "Successfully updated conan!"
  } else {
    Write-Host "Found conan version $conan_version. That fulfills the requirements!"
  }
}

# Install conan config
#
# We always pass the `-recreate-default-profiles` option here which removes all the previously automatically
# created default profiles. This avoids problems with stale default profiles which can occur when the OS has been
# updated and the previous default compiler is not available anymore.
#
# All changes made by the user to these profiles will be lost which is not ideal, but when calling bootstrap
# we expect the user wants a clean and working build no matter what. There is no need to make changes to the
# default profiles unless you try something out of the order and probably know what you are doing.
& "$PSScriptRoot\third_party\conan\configs\install.ps1" -recreate_default_profiles

# Start build
if ($args) {
  & "$PSScriptRoot\build.ps1" $args
} else {
  & "$PSScriptRoot\build.ps1"
}
