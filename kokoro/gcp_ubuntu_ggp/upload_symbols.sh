#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
set -e

function install_oauth2l {
  if [ ! -f oauth2l.tgz ]; then
    curl -o oauth2l.tgz https://storage.googleapis.com/oauth2l/latest/linux_amd64.tgz
  fi

  if [ ! -d oauth2l ]; then
    mkdir oauth2l
  fi

  if [ ! -x oauth2l/oauth2l ]; then
    tar -zxvf oauth2l.tgz -C oauth2l --strip-components 1
    chmod +x oauth2l/oauth2l
  fi
}

function install_jq {
  if [ ! "$(command -v jq)" ]; then
    sudo apt-get install --yes jq
  fi
}

function set_api_key {
  local gcurl='curl -H "$(oauth2l/oauth2l header cloud-platform userinfo.email)" -H "Content-Type: application/json"'
  local keys=$(eval $gcurl https://apikeys.googleapis.com/v2beta1/projects/60941241589/keys)
  if [ ! $(jq -n "$keys" | jq 'select(has("keys"))') ]; then
    echo "Can't get list of API keys"
    echo $keys
    exit 1
  fi

  local crash_symbol_key_full_name=$(jq -n "$keys" | jq '.keys[] | select(.displayName == "Crash Symbol Collector API key") | .name' | tr -d '"')
  if [ -z "$crash_symbol_key_full_name" ]; then
    echo "'Crash Symbol Collector API key' not found"
    exit 1
  fi

  api_key=$(eval $gcurl https://apikeys.googleapis.com/v2beta1/$crash_symbol_key_full_name/keyString | jq '.keyString' | tr -d '"')
}

function install_breakpad_tools {
  if [ ! -d breakpad ]; then
    conan install -g deploy breakpad/2ffe116@orbitdeps/stable
  fi
}

function upload_symbol_file {
  local symbol_file_path=$1
  local symbol_name=$2
  local sym_file_path=breakpad/symbols/$symbol_name.sym
  breakpad/bin/dump_syms $symbol_file_path > $sym_file_path
  breakpad/bin/symupload -p sym-upload-v2 -k $api_key $sym_file_path https://prod-crashsymbolcollector-pa.googleapis.com
}

function upload_debug_symbols {
  local bin_folder=$1
  
  install_oauth2l
  install_jq

  set_api_key
  if [ -z "$api_key" ]; then
    echo "Error on getting API key"
    exit 1
  fi

  install_breakpad_tools
  if [ ! -d breakpad/symbols ]; then
    mkdir breakpad/symbols
  fi

  upload_symbol_file $bin_folder/OrbitService OrbitService
}

upload_debug_symbols $1
