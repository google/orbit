// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

void BuildInCWithInstantiateInThisFile(void);
void BuildInCWithInstantiateInAnotherFile(void);

int main(void) {
  BuildInCWithInstantiateInThisFile();
  BuildInCWithInstantiateInAnotherFile();
  return 0;
}
