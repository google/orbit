# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This Makefile documents how the test binaries have been generated.
# When someone wants to reproduce that, hard-coded address values in the tests need to be updated.

CC=clang
PHONY=clean

line_info_test_binary: ../LineInfoTestBinary.cpp
	${CC} ../LineInfoTestBinary.cpp -g -O3 -o line_info_test_binary -fdebug-prefix-map=$(shell pwd)=.

line_info_test_binary_compressed: ../LineInfoTestBinary.cpp
	${CC} ../LineInfoTestBinary.cpp -g -gz -O3 -o line_info_test_binary_compressed -fdebug-prefix-map=$(shell pwd)=.

test_library: ../TestLibrary.cpp
	$(CC) ../TestLibrary.cpp -shared -fPIC -O3 -Wl,-soname,libtest.so -o libtest-1.0.so

clean:
	rm -f line_info_test_binary line_info_test_binary_compressed libtest-1.0.so
