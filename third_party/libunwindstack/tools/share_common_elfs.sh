#!/bin/bash
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

### Usage: shares_common_elfs.sh [-m] [-a] <path/to/dump/directory/or/elf>
###
### This script is used to eliminate copies of the same ELF file(s) across
### subdirectories of 'offline_files/' by placing common ELF(s) in
### 'offline_files/common'. The build id is appended to each ELF name to allow
### for different versions of the same shared library. Unless -a is used, the
### path must be a valid ELF file.
###
### Options:
### -m
###    If the ELF(s) does not currently reside in the 'common' directory, move
###     the elf to 'common' and rename it.
### -a
###    Eliminate copies for all ELFs in a directory. The path specified point
###    to a directory and NOT an individual ELF file.

usage() {
  grep '^###' $0 | sed -e 's/^###//'
  exit 1
}

update_elf() {
  local elf_path="$1"
  local move_if_not_in_common="$2"

  local basename=$(basename "$elf_path")
  local maps_path=$(dirname "$elf_path")"/maps.txt"
  local build_id=$(readelf -n "$elf_path" | grep -oP 'Build ID: \K([\w\d]+)')
  local new_elf_name="${basename}_${build_id}"
  local common_dir="${ANDROID_BUILD_TOP}/system/unwinding/libunwindstack/offline_files/common/"

  if [[ -f "${common_dir}${new_elf_name}" ]]; then
    # If the ELF is already found in the common directory, delete the local copy.
    git rm "$elf_path"
  elif [[ $move_if_not_in_common == true ]]; then
    # If the ELF is not found in the common directory and the `move_if_not_common`
    # flag was set, move the local ELF to the common directory.
    git mv "$elf_path" "${common_dir}${new_elf_name}"
  else
    # The ELF was not found in the common directory so exit this function.
    return 0
  fi

  # Replace the name of the elf we just deleted/moved to the relative
  # path to that ELF in the common directory.
  local elf_dir_path=$(dirname "$elf_path")
  local rel_path_to_common=$(realpath --relative-to="$elf_dir_path" "$common_dir")
  sed -i -e "s/${basename}/${rel_path_to_common}\/${new_elf_name}/g" "$maps_path"
}

is_an_elf() {
  if [[ $(head -c 4 $1 | cut -c2-) == "ELF" ]]; then
    echo true
  else
    echo false
  fi
}

main () {
  set -e # abort the script if some error occurs.
  local move_if_not_in_common=false
  local update_all_elfs=false
  while getopts ":hma" arg; do
    case $arg in
      m)
        move_if_not_in_common=true
        ;;
      a)
        update_all_elfs=true
        ;;
      h | *)
        usage
        ;;
    esac
  done
  path=${@:$OPTIND:1}
  if [[ -z $path ]]; then
    usage
  fi

  if [[ $update_all_elfs == true ]]; then
    if [[ ! -d $path || "${path: -1}" != "/" ]]; then
      echo "$path is not a valid path to a directory." >&2
      usage
    fi
    for elf_path in "${path}"*; do
      if [[ $(is_an_elf $elf_path) == true ]]; then
        update_elf $elf_path $move_if_not_in_common
      fi
    done
  else
    if [[ ! -f $path ]]; then
      echo "$path is not a valid path to a file." >&2
      usage
    elif [[ $(is_an_elf $path) == false ]]; then
      echo "$path is not a valid ELF file." >&2
    else
      update_elf $path $move_if_not_in_common
    fi
  fi
}

main "$@"