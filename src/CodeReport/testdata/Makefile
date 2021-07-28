# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This Makefile documents how the test binaries have been generated.
# When someone wants to reproduce that, hard-coded address values in the tests need to be updated.

CC=clang
PHONY=clean

line_info_test_binary: LineInfoTestBinary.cpp
	${CC} LineInfoTestBinary.cpp -g -O3 -o line_info_test_binary -fdebug-prefix-map=$(shell pwd)=.

clean:
	rm -f line_info_test_binary
