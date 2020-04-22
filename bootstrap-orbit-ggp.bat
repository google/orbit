@echo off
call powershell "& %~dp0\bootstrap-orbit-ggp.ps1 %*"

echo[
echo ---------------------------------------------------------------------------------
echo - By the way: "bootstrap-orbit-ggp.bat" is deprecated and will be removed soon. -
echo - Please use "bootstrap-orbit-ggp.ps1" - the PowerShell equivalent - instead.   -
echo ---------------------------------------------------------------------------------