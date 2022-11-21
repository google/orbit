# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

$conan_version_required = "1.53.0"

function Check-Conan-Version-Sufficient {
  $version = $args[0]
  $required = $args[1]

  $version_major = $version.split(".")[0] -as [int]
  $required_major = $required.split(".")[0] -as [int]
  if ($required_major -gt $version_major) {
    return 1
  }
  if ($required_major -lt $version_major) {
    return 0
  }

  $version_minor = $version.split(".")[1] -as [int]
  $required_minor = $required.split(".")[1] -as [int]
  if ($required_minor -gt $version_minor) {
    return 1
  }
  if ($required_minor -lt $version_minor) {
    return 0
  }

  $version_patch = $version.split(".")[2] -as [int]
  $required_patch = $required.split(".")[2] -as [int]
  if ($required_patch -gt $version_patch) {
    return 1
  }
  return 0
}

$conan = Get-Command -ErrorAction Ignore conan

if (!$conan) {
  Write-Host "Conan not found. Trying to install it via python-pip..."
	
  $pip3 = Get-Command -ErrorAction Ignore pip3
	
  if(!$pip3) {	
    Write-Error -ErrorAction Stop @"
It seems you don't have Python3 installed (pip3.exe not found).
Please install Python or make it available in the path.
Alternatively you could also just install conan manually and make
it available in the path.
"@
  }

  & $pip3.Path install conan==$conan_version_required
  if ($LastExitCode -ne 0) {
    Throw "Error while installing conan via pip3."
  }
    
  $conan = Get-Command -ErrorAction Ignore conan
  if (!$conan) {
    Write-Error -ErrorAction Stop @"
It seems we installed conan sucessfully, but it is not available
in the path. Please ensure that your Python user-executable folder is
in the path and call this script again.
You can call 'pip3 show -f conan' to figure out where conan.exe was placed.
"@
  }
} else {
  Write-Host "Conan found. Checking version..."

  $conan_version = (& $conan.Path --version).split(" ")[2]

  $sufficient = Check-Conan-Version-Sufficient $conan_version $conan_version_required
  if ($sufficient -ne 0) {
    Write-Host "Your conan version $conan_version is too old. Let's try to update it."
    Try {
      $pip3 = Get-Command pip3
      & $pip3.Path install --upgrade conan==$conan_version_required
    } Catch {
      Throw "Error while upgrading conan via pip3. Probably you have conan installed differently." + 
            " Please manually update conan to a at least version $conan_version_required"
    }

    Write-Host "Successfully updated conan!"
  } else {
    Write-Host "Found conan version $conan_version. That fulfills the requirements!"
  }
}

# Install conan config
& "$PSScriptRoot\third_party\conan\configs\install.ps1"

# Start build
if ($args) {
  & "$PSScriptRoot\build.ps1" $args
} else {
  & "$PSScriptRoot\build.ps1"
}
