// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_HAS_RADIO_BUTTON_INTERFACE_
#define SESSION_SETUP_HAS_RADIO_BUTTON_INTERFACE_

#include <QRadioButton>

namespace orbit_session_setup {

class HasRadioButtonInterface {
 public:
  [[nodiscard]] virtual QRadioButton* GetRadioButton() = 0;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_HAS_RADIO_BUTTON_INTERFACE_