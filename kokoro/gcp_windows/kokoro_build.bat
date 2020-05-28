@echo off
REM Copyright (c) 2020 The Orbit Authors. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.

:: Code under repo is checked out to %KOKORO_ARTIFACTS_DIR%\github.
:: The final directory name in this path is determined by the scm name specified
:: in the job configuration

SET REPO_ROOT=%KOKORO_ARTIFACTS_DIR%\github\orbitprofiler

:: Install conan config
call powershell "& %REPO_ROOT%\contrib\conan\configs\install.ps1"
if %errorlevel% neq 0 exit /b %errorlevel%

:: Building conan
set /P CRASHDUMP_SERVER=<%KOKORO_ARTIFACTS_DIR%\keystore\74938_orbitprofiler_crashdump_collection_server
call conan install -pr msvc2017_relwithdebinfo -if %REPO_ROOT%\build\ --build outdated -o crashdump_server="%CRASHDUMP_SERVER%" %REPO_ROOT%
if %errorlevel% neq 0 exit /b %errorlevel%

call conan build -bf %REPO_ROOT%\build\ %REPO_ROOT%
if %errorlevel% neq 0 exit /b %errorlevel%

call conan package -bf %REPO_ROOT%\build\ %REPO_ROOT%
if %errorlevel% neq 0 exit /b %errorlevel%

:: Package build artifacts into a zip for integration in the installer.
cd %REPO_ROOT%\build\package
Xcopy /E /I bin Orbit
del bin\Orbit.pdb
copy /Y THIRD_PARTY_LICENSES.txt Orbit\THIRD_PARTY_LICENSES.txt
copy /Y LICENSE Orbit\LICENSE.txt
zip -r Orbit.zip Orbit
rd /s /q Orbit

:: Uploading prebuilt packages of our dependencies
set /P ACCESS_TOKEN=<%KOKORO_ARTIFACTS_DIR%\keystore\74938_orbitprofiler_artifactory_access_token
if EXIST %KOKORO_ARTIFACTS_DIR%\keystore\74938_orbitprofiler_artifactory_access_token (
  call conan user -r artifactory -p "%ACCESS_TOKEN%" kokoro
  if %errorlevel% neq 0 exit /b %errorlevel%

  REM The llvm-package is very large and not needed as a prebuilt because it is not consumed directly.
  call conan remove 'llvm/*@orbitdeps/stable'
  if %errorlevel% neq 0 exit /b %errorlevel%

  call conan upload -r artifactory --all --no-overwrite all --confirm  --parallel *
  if %errorlevel% neq 0 exit /b %errorlevel%
) else (
  echo No access token available. Skipping package upload.
)
