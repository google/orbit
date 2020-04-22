@echo off
call powershell "& %~dp0\bootstrap-orbit.ps1 %*"

echo[
echo -----------------------------------------------------------------------------
echo - By the way: "bootstrap-orbit.bat" is deprecated and will be removed soon. -
echo - Please use "bootstrap-orbit.ps1" - the PowerShell equivalent - instead.   -
echo -----------------------------------------------------------------------------