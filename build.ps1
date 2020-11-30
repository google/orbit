$conan = Get-Command conan -ErrorAction Stop

function conan_profile_exists($profile) {
  $process = Start-Process $conan.Path -windowstyle Hidden -ArgumentList "profile","show",$profile -PassThru -Wait  
  return ($process.ExitCode -eq 0)
}

function conan_create_profile($profile) {
  if (! (& $conan.Path profile show default)) {
    if (! (& $conan.Path profile new --detect default)) {
      exit $?
    }
  }

  $compiler = & $conan.Path profile show default | Select-String -Pattern "compiler=" | ForEach-Object { ([string] $_).split("=")[1] }
  $compiler_version = & $conan.Path profile show default | Select-String -Pattern "compiler.version=" | ForEach-Object { ([string] $_).split("=")[1] }

  if ($compiler -ne "Visual Studio") {
    Throw "It seems conan couldn't detect your Visual Studio installation. Do you have Visual Studio installed? At least Visual Studio 2017 is required!"
  }

  if ([int]$compiler_version -ne 15 -and [int]$compiler_version -ne 16) {
    Throw "Your version of Visual Studio is not supported. We currently only support VS2017 and VS2019."
  }

  $compiler_version = if ($compiler_version -eq 15) { "msvc2017" } else { "msvc2019" }

  $conan_dir = if ($env:CONAN_USER_HOME) { $env:CONAN_USER_HOME } else { $env:USERPROFILE }
  $conan_dir += "/.conan"

  $build_type = $profile -replace "default_"
  $profile_path = "$conan_dir/profiles/$profile"

  if (!$Env:Qt5_DIR) {
    Write-Host ""
    Write-Host "Compile Qt from source or use official distribution?"
    Write-Host "====================================================`r`n"
    Write-Host "Orbit depends on the Qt framework which can be either automatically compiled from source (can take several hours!)"
    Write-Host "or can be provided by an official Qt distribution. We recommend the latter which is less cumbersome."
    Write-Host "Check out DEVELOPMENT.md for more details on how to use an official Qt distribution.`r`n"
    Write-Host "Press Ctrl+C to stop here and install Qt first. Press Enter to continue with compiling Qt from source."
    Write-Host "Google-internal devs: Please press Enter!`r`n"
    Read-Host  "Answer"
  }

  "include(${compiler_version}_${build_type})`r`n" | Set-Content -Path $profile_path
  "[settings]`r`n[options]" | Add-Content -Path $profile_path

  if ($Env:Qt5_DIR) {
    Write-Host "Found Qt5_DIR environment variable. Using system provided Qt distribution."
    "OrbitProfiler:system_qt=True`r`n[build_requires]`r`n[env]" | Add-Content -Path $profile_path
    "OrbitProfiler:Qt5_DIR=`"$Env:Qt5_DIR`"" | Add-Content -Path $profile_path
  } else {
    "[build_requires]`r`n[env]" | Add-Content -Path $profile_path
  }
}

$profiles = if ($args.Count) { $args } else { @("default_relwithdebinfo") }

foreach ($profile in $profiles) {

  if ($profile.StartsWith("default_") -and (-not (conan_profile_exists $profile))) {    
    Write-Host "Creating conan profile $profile"
    conan_create_profile $profile
  }
 
  Write-Host "Building Orbit in build_$profile/ with conan profile $profile"
  New-Item -ItemType Directory -Force -Path ".\build_$profile\" | Out-Null

  & $conan.Path lock create "$PSScriptRoot\conanfile.py" --user=orbitdeps --channel=stable `
    --build=outdated `
    --lockfile="$PSScriptRoot\third_party\conan\lockfiles\base.lock" -u -pr $profile `
    --lockfile-out=.\build_$profile\conan.lock
  if ($LastExitCode -ne 0) {
    Throw "Error while running conan lock create."
  }
 
  & $conan.Path install -if build_$profile\ --build outdated --lockfile=build_$profile\conan.lock "$PSScriptRoot"

  if ($LastExitCode -ne 0) {
    Throw "Error while running conan install."
  }
  
  $process = Start-Process $conan.Path -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "build","-bf","build_$profile/",$PSScriptRoot
  $handle = $process.Handle # caching handle needed due to bug in .Net
  $process.WaitForExit()
  if ($process.ExitCode -ne 0) {
    Throw "Error while running conan build."
  }
}
