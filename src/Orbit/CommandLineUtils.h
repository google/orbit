// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _ORBIT_MAIN_H__
#define _ORBIT_MAIN_H__

#include <QStringList>
#include <string>
#include <vector>

// Extract command line flags by filtering the positional arguments out from the command line
// arguments.
QStringList ExtractCommandLineFlags(const std::vector<std::string>& command_line_args,
                                    const std::vector<char*>& positional_args);
QStringList RemoveFlagsNotPassedToMainWindow(const QStringList& flags);

#endif