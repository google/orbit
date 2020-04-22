$conan = Get-Command conan -ErrorAction Stop

function conan_profile_exists($profile) {
  $process = Start-Process $conan.Path -windowstyle Hidden -ArgumentList "profile","show",$profile -PassThru -Wait  
  return ($process.ExitCode -eq 0)
}

function conan_create_profile($profile) {
  $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "profile","new","--detect",$profile
  if ($process.ExitCode -ne 0) { Throw "Error while creating conan profile." }
  
  $arch = If ($profile.EndsWith("x86")) { "x86" } Else { "x86_64" }
  $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "profile","update","settings.arch=$arch",$profile
  if ($process.ExitCode -ne 0) { Throw "Error while creating conan profile." }
  
  $profile_path = "$Env:USERPROFILE\.conan\profiles\$profile"
  (Get-Content $profile_path) -replace '\[build_requires\]', "[build_requires]`r`ncmake/3.16.4@" | Out-File -encoding ASCII $profile_path
}

$profiles = if ($args.Count) { $args } else { @("default_release") }

foreach ($profile in $profiles) {

  if ($profile.StartsWith("default_") -and (-not (conan_profile_exists $profile))) {    
    Write-Host "Creating conan profile $profile"
    conan_create_profile $profile
  }
  
  Write-Host "Building Orbit in build_$profile/ with conan profile $profile"

  $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "install","-pr",$profile,"-o","system_qt=False","-if","build_$profile/","--build","outdated",$PSScriptRoot
  if ($process.ExitCode -ne 0) {
    Throw "Error while running conan install."
  }
  
  $process = Start-Process $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "build","-bf","build_$profile/",$PSScriptRoot
  if ($process.ExitCode -ne 0) {
    Throw "Error while running conan build."
  }
}
