/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <GL/glew.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main ()
{
    assert(glewGetString(GLEW_VERSION));
    printf("Bincrafters GLEW %s\n", glewGetString(GLEW_VERSION));
    return EXIT_SUCCESS;
}
