@echo off

SET REPO_ROOT=%~dp0

setlocal
cd %REPO_ROOT%
git submodule update --init --recursive
if %errorlevel% neq 0 exit /b %errorlevel%
endlocal

conan config install %REPO_ROOT%\contrib\conan\config
if %errorlevel% neq 0 exit /b %errorlevel%

call build.bat msvc2019_release_x64
