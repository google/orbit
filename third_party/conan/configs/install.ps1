# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

$conan = Get-Command conan -ErrorAction Stop

function conan_disable_public_remotes() {
  $remotes = "bintray", "conan-center"
  foreach ($remote in $remotes) {
    $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "remote","disable",$remote
    if ($process.ExitCode -ne 0) { Throw "Error while disabling conan-remote $remote." }
  }
}

# Installs conan config (build settings, public remotes, conan profiles, conan options). Have a look in \third_party\conan\configs\windows for more information.
$process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "config","install","$PSScriptRoot\windows"
if ($process.ExitCode -ne 0) { Throw "Error while installing conan config." }

if (Test-Path Env:ORBIT_OVERRIDE_ARTIFACTORY_URL) {
  Write-Host "Artifactory override detected. Adjusting remotes..."

  $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "remote","add","-i","0","-f","artifactory",$Env:ORBIT_OVERRIDE_ARTIFACTORY_URL
  if ($process.ExitCode -ne 0) { Throw "Error while adding conan remote." }

  conan_disable_public_remotes
} else {
  try {
    $response = Invoke-WebRequest -URI http://invalid-hostname.internal/ -ErrorAction Ignore -MaximumRedirection 0 -UseBasicParsing
    Write-Host "Detected catch-all DNS configuration. Using public remotes for conan."
    exit
  } catch {
    # If we jump into this catch-block, that means there is no catch-all DNS.
  }

  try {
    $response = Invoke-WebRequest -URI http://artifactory.internal/ -ErrorAction Ignore -MaximumRedirection 0 -UseBasicParsing
    Write-Host "CI machine detected. Adjusting remotes..."

    $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "remote","add","-i","0","-f","artifactory","http://artifactory.internal/artifactory/api/conan/conan"
    if ($process.ExitCode -ne 0) { Throw "Error while adding conan remote." }

    conan_disable_public_remotes

    if (Test-Path -Path "$PSScriptRoot\windows_ci") {
      Write-Host "Found CI-specific Conan config. Installing that as well..."
      $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "config", "install", "$PSScriptRoot\windows_ci"
      if ($process.ExitCode -ne 0) { Throw "Error while installing conan config." }
    }
  } catch {
    try {
      $response = Invoke-WebRequest -URI http://orbit-artifactory/ -ErrorAction Ignore -MaximumRedirection 0 -UseBasicParsing
      Write-Host "Internal machine detected. Adjusting remotes..."

      $location = $response.Headers['Location'].Split("/")[2]

      $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "remote","add","-i","0","-f","artifactory","http://$location/artifactory/api/conan/conan"
      if ($process.ExitCode -ne 0) { Throw "Error while adding conan remote." }

      $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "config","set","proxies.http=`"$location=http://127.0.0.2:999`""
      if ($process.ExitCode -ne 0) { Throw "Error while adding proxy setting to conan." }

      conan_disable_public_remotes
    } catch {
      Write-Host "Using public remotes for conan."
    }
  }
}
