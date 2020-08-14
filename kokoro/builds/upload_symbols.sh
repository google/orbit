#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
set -e

function install_oauth2l {
  if [ "$(uname -s)" == "Linux" ]; then
    local os="linux"
  else
    local os="windows"
  fi

  curl -o oauth2l.tgz https://storage.googleapis.com/oauth2l/latest/${os}_amd64.tgz
  tar -zxvf oauth2l.tgz --strip-components 1
  chmod +x oauth2l
  rm -f oauth2l.tgz
}

function retrieve_oauth_token_header {
  ./oauth2l header cloud-platform userinfo.email
}

function get_api_key {
  local header=$1
  local gcurl="curl -H \"${header}\" -H \"Content-Type: application/json\""
  local keys=$(eval $gcurl https://apikeys.googleapis.com/v2beta1/projects/60941241589/keys)
  if [ -z "$keys" ] || [ $(echo "$keys" | jq 'has("keys")') == "false" ]; then
    echo "Can't get list of API keys"
    echo $keys
    exit 1
  fi

  local crash_symbol_key_full_name=$(jq -n "$keys" | jq '.keys[] | select(.displayName == "Crash Symbol Collector API key") | .name' | tr -d '"')
  if [ -z "$crash_symbol_key_full_name" ]; then
    echo "'Crash Symbol Collector API key' not found"
    exit 1
  fi

  eval $gcurl https://apikeys.googleapis.com/v2beta1/$crash_symbol_key_full_name/keyString | jq '.keyString' | tr -d '"'
}

function remove_oauth2l {
  rm -f oauth2l
}

function install_breakpad_tools {
  if [ ! -d breakpad ]; then
    conan install -g deploy breakpad/2ffe116@orbitdeps/stable
  fi
}

function upload_symbol_file {
  local api_key=$1
  local symbol_file_path=$2

  if [ "$(uname -s)" == "Linux" ]; then
    local symbol_name="$(basename $symbol_file_path)"
    local sym_file_path=breakpad/symbols/$symbol_name.sym
    breakpad/bin/dump_syms $symbol_file_path > $sym_file_path
    breakpad/bin/symupload -p sym-upload-v2 -k $api_key $sym_file_path \
      https://prod-crashsymbolcollector-pa.googleapis.com
  else
    breakpad/bin/symupload -p $symbol_file_path \
      https://prod-crashsymbolcollector-pa.googleapis.com $api_key
  fi
}

function upload_debug_symbols {
  local api_key=$1
  local bin_folder=$2
  
  install_breakpad_tools
  if [ ! -d breakpad/symbols ]; then
    mkdir breakpad/symbols
  fi

  if [ "$(uname -s)" == "Linux" ]; then
    upload_symbol_file $api_key $bin_folder/OrbitService
  else
    upload_symbol_file $api_key $bin_folder/Orbit.exe                             
    upload_symbol_file $api_key $bin_folder/Qt5Core.dll                           
    upload_symbol_file $api_key $bin_folder/Qt5Gui.dll                            
    upload_symbol_file $api_key $bin_folder/Qt5Network.dll                        
    upload_symbol_file $api_key $bin_folder/Qt5Widgets.dll                        
    upload_symbol_file $api_key $bin_folder/bearer/qgenericbearer.dll             
    upload_symbol_file $api_key $bin_folder/imageformats/qgif.dll                 
    upload_symbol_file $api_key $bin_folder/imageformats/qico.dll                 
    upload_symbol_file $api_key $bin_folder/imageformats/qjpeg.dll                
    upload_symbol_file $api_key $bin_folder/platforms/qwindows.dll                
    upload_symbol_file $api_key $bin_folder/styles/qwindowsvistastyle.dll
  fi
}
  
