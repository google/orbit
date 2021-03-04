"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import os, sys
import subprocess

def install_orbitutils():
    # This must be the folder that contains the setup.py file and the "orbitutils"
    # folder. Since build.py is next to setup.py, we can just use the folder of
    # the current file.
    orbitutils_setup_path = os.path.dirname(os.path.abspath(__file__))
    subprocess.check_call(
        [sys.executable, "-m", "pip", "install", "-e", orbitutils_setup_path])

def main():
    current_directory = os.path.dirname(os.path.abspath(__file__))
    orbitutils_path = os.path.join(current_directory, "orbitutils")

    # We need to specify the "root" of the src tree. With this choice, we can then
    # import the proto as "import ClientProtos.capture_data_pb2.py"
    proto_path = os.path.join(current_directory, "../../src/")

    # This is the only .proto file we currently need for our python code.
    capture_data_proto = os.path.join(
        current_directory, "../../src/ClientProtos/capture_data.proto")

    command = "protoc --python_out=" + orbitutils_path + " --proto_path=" + proto_path + " " + capture_data_proto
    ret = os.system(command)
    if ret:
        print("Command failed: ", command)
        sys.exit(ret)

    install_orbitutils()

    subprocess.check_call(
        [sys.executable,
         os.path.join(current_directory, "run_tests.py")])


if __name__ == "__main__":
    main()