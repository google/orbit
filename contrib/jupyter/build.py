"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import os, sys
import orbitutils.run_all_unittests

file_directory = os.path.dirname(os.path.abspath(__file__))
orbitutils_path = os.path.join(file_directory, "orbitutils")

# We need to specify the "root" of the src tree. With this choice, we can then
# import the proto as "import ClientProtos.capture_data_pb2.py"
proto_path = os.path.join(file_directory, "../../src/")

# This is the only .proto file we currently need for our python code.
capture_data_proto = os.path.join(file_directory,
                                  "../../src/ClientProtos/capture_data.proto")

command = "protoc --python_out=" + orbitutils_path + " --proto_path=" + proto_path + " " + capture_data_proto
ret = os.system(command)
if ret:
    print("Command failed: ", command)
    sys.exit(ret)

# Verify by running all unit tests
orbitutils.run_all_unittests.main()