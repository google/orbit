# Copyright (c) 2020 The Orbit Authors. All rights reserved. Use of this source
# code is governed by a BSD-style license that can be found in the LICENSE file.

cmake_minimum_required(VERSION 3.12)

function(grpc_helper)
  find_program(_HELPER_PROTOC protoc)
  find_program(_HELPER_GRPC_CPP_PLUGIN grpc_cpp_plugin)

  get_target_property(_sources ${ARGV0} SOURCES)
  set(new_sources "")
  set(bin_dir "${CMAKE_CURRENT_BINARY_DIR}/grpc_codegen/include/${ARGV0}")

  foreach(source_file ${_sources})
    if(source_file MATCHES ".*\.proto")
      get_filename_component(basename ${source_file} NAME_WE)

      get_filename_component(src_filepath ${source_file} ABSOLUTE)
      get_filename_component(src_folder ${src_filepath} PATH)

      set(proto_cpp "${bin_dir}/${basename}.pb.cc")
      set(proto_h "${bin_dir}/${basename}.pb.h")
      set(grpc_cpp "${bin_dir}/${basename}.grpc.pb.cc")
      set(grpc_h "${bin_dir}/${basename}.grpc.pb.h")

      add_custom_command(
        OUTPUT "${proto_cpp}" "${proto_h}" "${grpc_cpp}" "${grpc_h}"
        COMMAND ${CMAKE_COMMAND} ARGS -E make_directory "${bin_dir}"
        COMMAND
          ${_HELPER_PROTOC} ARGS --grpc_out "${bin_dir}" --cpp_out "${bin_dir}"
          -I ${src_folder} --plugin=protoc-gen-grpc="${_HELPER_GRPC_CPP_PLUGIN}"
          "${src_filepath}"
        MAIN_DEPENDENCY "${src_filepath}")

      list(APPEND new_sources "${proto_cpp}")
      list(APPEND new_sources "${grpc_cpp}")

    endif()
  endforeach()

  list(FILTER _sources EXCLUDE REGEX ".*\.proto")
  list(APPEND _sources ${new_sources})
  set_target_properties(${ARGV0} PROPERTIES SOURCES "${_sources}")

  target_link_libraries(${ARGV0} PUBLIC grpc::grpc)
  target_include_directories(${ARGV0} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/grpc_codegen/include/)

  # include-what-you-use needs full source-code visibility, which means
  # we don't need to compile all the code, but we need to run all code
  # generators. The following snippet adds auto-generated grpc files
  # as build dependencies to the include-what-you-use target.
  if(TARGET iwyu)
    add_custom_target(${ARGV0}_iwyu_grpc DEPENDS ${new_sources})
    add_dependencies(iwyu ${ARGV0}_iwyu_grpc)
  endif()
endfunction()
