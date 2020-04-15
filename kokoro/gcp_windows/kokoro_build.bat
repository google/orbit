:: Code under repo is checked out to %KOKORO_ARTIFACTS_DIR%\github.
:: The final directory name in this path is determined by the scm name specified
:: in the job configuration
@echo off

SET REPO_ROOT=%KOKORO_ARTIFACTS_DIR%\github\orbitprofiler

conan config install %REPO_ROOT%\contrib\conan\configs\windows
if %errorlevel% neq 0 exit /b %errorlevel%

REM We replace the default remotes by our internal artifactory server
REM which acts as a secure source for prebuilt dependencies.
copy /Y %REPO_ROOT%\kokoro\conan\config\remotes.json %HOME%\.conan\remotes.json
if %errorlevel% neq 0 exit /b %errorlevel%

conan config set general.revisions_enabled=True
if %errorlevel% neq 0 exit /b %errorlevel%

conan config set general.parallel_download=8
if %errorlevel% neq 0 exit /b %errorlevel%

:: Building conan
call conan install -pr msvc2019_release -if %REPO_ROOT%\build\ --build outdated %REPO_ROOT%
if %errorlevel% neq 0 exit /b %errorlevel%

call conan build -bf %REPO_ROOT%\build\ %REPO_ROOT%
if %errorlevel% neq 0 exit /b %errorlevel%

call conan package -bf %REPO_ROOT%\build\ %REPO_ROOT%
if %errorlevel% neq 0 exit /b %errorlevel%

:: Package build artifacts into a zip for integration in the installer.
cd %REPO_ROOT%\build\package
ren bin Orbit
zip -r Orbit.zip Orbit

:: Uploading prebuilt packages of our dependencies
if EXIST %KOKORO_ARTIFACTS_DIR%\keystore\74938_orbitprofiler_artifactory_access_token (
  set /p ACCESS_TOKEN=<%KOKORO_ARTIFACTS_DIR%\keystore\74938_orbitprofiler_artifactory_access_token

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
