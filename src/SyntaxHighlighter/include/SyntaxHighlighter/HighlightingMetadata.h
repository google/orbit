// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HIGHLIGHTING_METADATA_H_
#define HIGHLIGHTING_METADATA_H_

#include <QTextBlockUserData>

namespace orbit_syntax_highlighter {

class HighlightingMetadata : public QTextBlockUserData {
 public:
  [[nodiscard]] virtual bool IsMainContentLine() const = 0;
};

}  // namespace orbit_syntax_highlighter

#endif