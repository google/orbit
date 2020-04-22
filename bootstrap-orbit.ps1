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

  $result = Start-Process -FilePath $pip3.Path -ArgumentList "install","conan" -Wait -NoNewWindow -ErrorAction Stop -PassThru
  if ($result.ExitCode -ne 0) {
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
  Write-Host "Conan found. Skipping installation..."
}

# Install conan config
$result = Start-Process -FilePath $conan.Path -ArgumentList "config","install","$PSScriptRoot\contrib\conan\configs\windows" -Wait -NoNewWindow -ErrorAction Stop -PassThru
if ($result.ExitCode -ne 0) {
  Throw "Error while installing conan config."
}

# Start build
if ($args) {
  & "$PSScriptRoot\build.ps1" $args
} else {
  & "$PSScriptRoot\build.ps1"
}