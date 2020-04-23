REM Copyright (c) 2020 The Orbit Authors. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.

@echo off
call powershell "& %~dp0\bootstrap-orbit-ggp.ps1 %*"
