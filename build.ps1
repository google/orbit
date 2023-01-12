function conan {
  & py -3 -m conans.conan $args
}

function conan_profile_exists($profile) {
  conan profile show $profile >null 2>&1
  return ($LastExitCode -eq 0)
}

function conan_create_profile($profile) {
  if (! (conan profile show default)) {
    if (! (conan profile new --detect default)) {
      exit $?
    }
  }

  $compiler = conan profile show default | Select-String -Pattern "compiler=" | ForEach-Object { ([string] $_).split("=")[1] }
  $compiler_version = conan profile show default | Select-String -Pattern "compiler.version=" | ForEach-Object { ([string] $_).split("=")[1] }

  if ($compiler -ne "Visual Studio") {
    Throw "It seems conan couldn't detect your Visual Studio installation. Do you have Visual Studio installed? At least Visual Studio 2019 is required!"
  }

  if ([int]$compiler_version -ne 16 -and [int]$compiler_version -ne 17) {
    Throw "Your version of Visual Studio is not supported. We currently only support VS2019 and VS2022."
  }

  $compiler_version = if ($compiler_version -eq 16) { "msvc2019" } else { "msvc2022" }

  $conan_dir = if ($env:CONAN_USER_HOME) { $Env:CONAN_USER_HOME } else { $Env:USERPROFILE }
  $conan_dir += "\.conan"

  $build_type = $profile -replace "default_"
  $profile_path = "$conan_dir\profiles\$profile"

  if (!$Env:Qt5_DIR) {
    Write-Host ""
    Write-Host "Qt5_DIR environment variable not set - Please provide path to Qt distribution`r`n"
    Write-Host "=============================================================================`r`n"
    Write-Host "Orbit depends on the Qt framework which has to be installed separately either using the official installer,`r`n"
    Write-Host "compiled manually from source or installed using a third party installer like aqtinstall.`r`n"
    Write-Host "Check out DEVELOPMENT.md for more details on how to use an official Qt distribution.`r`n"
    Write-Host "Press Ctrl+C to stop here and install Qt first. Make sure the Qt5_DIR environment variable is pointing to your`r`n"
    Write-Host "Qt installation. Then call this script again.`r`n"
    Exit 1
  }

  "include(${compiler_version}_${build_type})`r`n" | Set-Content -Path $profile_path
  "[settings]`r`n[options]" | Add-Content -Path $profile_path

  if ($Env:Qt5_DIR) {
    Write-Host "Found Qt5_DIR environment variable. Using system provided Qt distribution from $Env:Qt5_DIR."
    "`r`n[env]`r`nOrbitProfiler:Qt5_DIR=`"$Env:Qt5_DIR`"" | Add-Content -Path $profile_path
  }
}

# That's the profile that is used for tools that run on the build machine (Nasm, CMake, Ninja, etc.)
$build_profile = "default_release"
if (-not (conan_profile_exists $build_profile)) {
  Write-Host "Creating conan profile $build_profile"
  conan_create_profile $build_profile
}


$profiles = if ($args.Count) { $args } else { @("default_release") }

foreach ($profile in $profiles) {

  if ($profile.StartsWith("default_") -and (-not (conan_profile_exists $profile))) {    
    Write-Host "Creating conan profile $profile"
    conan_create_profile $profile
  }
 
  Write-Host "Building Orbit in build_$profile/ with conan profile $profile"

  conan install -if build_$profile\ --build outdated "-pr:b" $build_profile "-pr:h" $profile -u "$PSScriptRoot"

  if ($LastExitCode -ne 0) {
    Throw "Error while running conan install."
  }
  
  conan build -bf "build_$profile/" $PSScriptRoot
  if ($LastExitCode -ne 0) {
    Throw "Error while running conan build."
  }

  if ($profile.StartsWith("default_")) {
    Write-Host "The build finished successfully. Start Orbit with .\build_$profile\bin\Orbit!"
  }
}
