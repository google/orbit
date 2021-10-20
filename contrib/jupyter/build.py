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
    orbitutils_path = os.path.join(current_directory, "orbitutils/")

    proto_path = os.path.join(current_directory, "../../src/GrpcProtos")

    protos = ""
    protos += " " + os.path.join(
        current_directory, "../../src/GrpcProtos/capture.proto")
    protos += " " + os.path.join(
        current_directory, "../../src/GrpcProtos/module.proto")
    protos += " " + os.path.join(
        current_directory, "../../src/GrpcProtos/tracepoint.proto")

    command = "protoc --python_out=" + orbitutils_path + " --proto_path=" + proto_path + protos
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