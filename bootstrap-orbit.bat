REM Copyright (c) 2020 The Orbit Authors. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.

@echo off
call powershell "& %~dp0\bootstrap-orbit.ps1 %*"

echo[
echo -----------------------------------------------------------------------------
echo - By the way: "bootstrap-orbit.bat" is deprecated and will be removed soon. -
echo - Please use "bootstrap-orbit.ps1" - the PowerShell equivalent - instead.   -
echo -----------------------------------------------------------------------------
