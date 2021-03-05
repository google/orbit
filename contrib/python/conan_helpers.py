# coding=utf-8

"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import os


def _parse_chromium_readme(file_path):
    result = dict()

    with open(file_path, 'r', encoding='UTF-8') as fd:
        for line in fd:
            parts = line.split(": ", 2)
            if len(parts) != 2:
                break

            result[parts[0].lower()] = parts[1].strip()

    return result


def _patch_chromium_readme(license_info):
    """ There are some typos in the chromium supplied license info.
    These are fixed by this function.
    """

    if license_info.get("license file") == "MIT":
        license_file = license_info.get("license")
        license_info["license"] = license_info["license file"]
        license_info["license file"] = license_file

    if license_info.get("license file") == "LICENCSE":
        license_info["license file"] = "LICENCE"


def gather_chromium_licenses(rootpath):
    """ Takes a directory path and recursively searches this path for Chromium
    license metadata files (README.chromium), parses the metafields into a
    dictionary and returns a list of all the found metadata files.
    """
    result = []

    for root, _, files in os.walk(rootpath):
        if "README.chromium" in files:
            license_info = _parse_chromium_readme(
                os.path.join(root, "README.chromium"))
            _patch_chromium_readme(license_info)

            if "license file" not in license_info:
                continue

            if license_info["license file"] == "NOT_SHIPPED":
                continue

            license_info["license file"] = os.path.normpath(
                os.path.join(root, license_info.get("license file")))

            if os.path.isfile(license_info["license file"]):
                result.append(license_info)

    return result
