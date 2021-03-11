# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(Vulkan::ValidationLayers INTERFACE IMPORTED GLOBAL)
target_include_directories(Vulkan::ValidationLayers SYSTEM
                           INTERFACE third_party/vulkan)
target_link_libraries(Vulkan::ValidationLayers INTERFACE CONAN_PKG::vulkan-headers)
