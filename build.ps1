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

  "include(${compiler_version}_${build_type})`r`n" | Set-Content -Path $profile_path
  "[settings]`r`n[options]" | Add-Content -Path $profile_path
  "[build_requires]`r`n[env]" | Add-Content -Path $profile_path
}

$profiles = if ($args.Count) { $args } else { @("default_relwithdebinfo") }

foreach ($profile in $profiles) {

  if ($profile.StartsWith("default_") -and (-not (conan_profile_exists $profile))) {    
    Write-Host "Creating conan profile $profile"
    conan_create_profile $profile
  }
  
  Write-Host "Building Orbit in build_$profile/ with conan profile $profile"

  $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "install","--update","-pr",$profile,"-o","system_qt=False","-if","build_$profile/","--build","outdated",$PSScriptRoot
  if ($process.ExitCode -ne 0) {
    Throw "Error while running conan install."
  }
  
  $process = Start-Process $conan.Path -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "build","-bf","build_$profile/",$PSScriptRoot
  $handle = $process.Handle # caching handle needed due to bug in .Net
  $process.WaitForExit()
  if ($process.ExitCode -ne 0) {
    Throw "Error while running conan build."
  }
}
