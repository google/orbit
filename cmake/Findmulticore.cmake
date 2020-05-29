# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(DIR third_party/multicore/common)

add_library(
  multicore OBJECT
  ${DIR}/inmemorylogger.cpp
  ${DIR}/autoresetevent.h
  ${DIR}/autoreseteventcondvar.h
  ${DIR}/benaphore.h
  ${DIR}/bitfield.h
  ${DIR}/diningphilosophers.h
  ${DIR}/inmemorylogger.h
  ${DIR}/rwlock.h
  ${DIR}/sema.h)

target_include_directories(multicore SYSTEM PUBLIC ${DIR})
target_compile_features(multicore PUBLIC cxx_std_11)
target_link_libraries(multicore PUBLIC Threads::Threads)

add_library(multicore::multicore ALIAS multicore)
