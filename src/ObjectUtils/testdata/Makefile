# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This Makefile documents how the test binaries have been generated.
# When someone wants to reproduce that, hard-coded address values in the tests need to be updated.

# For the MinGW-based compilation step, MinGW needs to be installed with
# sudo apt-get install gcc-mingw-w64-x86-64

CC=clang
PHONY=clean
MINGW_CC=x86_64-w64-mingw32-gcc

line_info_test_binary: LineInfoTestBinary.cpp
	${CC} LineInfoTestBinary.cpp -g -O3 -o line_info_test_binary -fdebug-prefix-map=$(shell pwd)=.

line_info_test_binary_compressed: LineInfoTestBinary.cpp
	${CC} LineInfoTestBinary.cpp -g -gz -O3 -o line_info_test_binary_compressed -fdebug-prefix-map=$(shell pwd)=.

test_library: ../TestLibrary.cpp
	$(CC) ../TestLibrary.cpp -shared -fPIC -O3 -Wl,-soname,libtest.so -o libtest-1.0.so

test_library_dll: ../TestLibrary.cpp
	${MINGW_CC} ../TestLibrary.cpp -shared -g -gdwarf -o libtest.dll

clean:
	rm -f line_info_test_binary line_info_test_binary_compressed libtest-1.0.so libtest.dll
