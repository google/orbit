#!/usr/bin/env python3
#
# Copyright (C) 2021 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse, subprocess, re, os, glob, array, gzip

DESCRIPTION = "This tool reduces ELF size using stripping and compression"

STRIP_SECTIONS = [".text", ".rodata"]

READELF_FORMAT = """
  \s+(?P<index>[0-9\[\] ]+)
  \s+(?P<name>[a-z_.]+)
  \s+(?P<type>[A-Z_]+)
  \s+(?P<address>[0-9a-f]+)
  \s+(?P<offset>[0-9a-f]+)
  \s+(?P<size>[0-9a-f]+)
"""

def strip(path):
  proc = subprocess.run(["readelf", "--file-header", "--sections", path],
                        stdout=subprocess.PIPE, universal_newlines=True)
  assert(proc.returncode == 0)  # readelf command failed
  sections = {m["name"] : m for m in re.finditer(READELF_FORMAT, proc.stdout, re.VERBOSE)}
  for name in STRIP_SECTIONS:
    if name == ".text" and os.path.basename(path) in ["vdso", "vdso.so", "libc.so"]:
      continue  # Stripping these libraries breaks signal handler unwinding.
    section = sections.get(name)
    if not section:
      print("Warning: {} not found in {}".format(name, path))
    if section and section["type"] != "NOBITS":
      offset, size = int(section["offset"], 16), int(section["size"], 16) & ~1
      with open(path, "r+b") as f:
        f.seek(offset)
        data = array.array('H')  # 16-bit unsigned integer array.
        data.frombytes(f.read(size))
        # Preserve top bits for thumb so that we can still determine instruction size.
        is_thumb = (name == ".text" and re.search("Machine:\s+ARM", proc.stdout))
        for i in range(len(data)):
          data[i] = 0xffff if is_thumb and (data[i] & 0xe000) == 0xe000 else 0
        f.seek(offset)
        f.write(data.tobytes())

  # gzip-compress the file to take advantage of the zeroed sections.
  with open(path, 'rb') as src, gzip.open(path + ".gz", 'wb') as dst:
    dst.write(src.read())
  os.remove(path)

def main():
  parser = argparse.ArgumentParser(description=DESCRIPTION)
  parser.add_argument('target', nargs='+', help="ELF file or whole directory to strip")
  args = parser.parse_args()

  for path in args.target:
    if os.path.isdir(path):
      for path in glob.glob(os.path.join(path, "**/*"), recursive=True):
        if os.path.isfile(path) and open(path, "rb").read(4) == b"\x7FELF":
          strip(path)
    else:
      strip(path)

if __name__ == '__main__':
  main()
