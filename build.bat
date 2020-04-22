@echo off
call powershell "& %~dp0\build.ps1 %*"

echo[
echo -------------------------------------------------------------------
echo - By the way: "build.bat" is deprecated and will be removed soon. -
echo - Please use "build.ps1" - the PowerShell equivalent - instead.   -
echo -------------------------------------------------------------------