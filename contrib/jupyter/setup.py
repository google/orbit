"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

from setuptools import setup

setup(
    name='orbitutils',
    version='0.0.1',
    author='The Orbit Authors',
    packages=['orbitutils'],
    url='http://github.com/google/orbit',
    license='BSD 2-Clause License',
    description='Orbit capture utilities',
    # This should stay in sync with the proto version we use in C++ (see conanfile.py).
    install_requires=["protobuf==3.9.1"],
)
