cmake_minimum_required(VERSION 3.12)

function(grpc_helper)
  find_program(_HELPER_PROTOC protoc)
  find_program(_HELPER_GRPC_CPP_PLUGIN grpc_cpp_plugin)

  get_target_property(_sources ${ARGV0} SOURCES)

  foreach(source_file ${_sources})
    if(source_file MATCHES ".*\.proto")
      string(REGEX REPLACE "[./:;]" "_" target_name ${source_file})
      set(target_name "_grpc_${target_name}")

      if(NOT TARGET ${target_name})
        get_filename_component(basename ${source_file} NAME_WE)

        get_filename_component(src_filepath ${source_file} ABSOLUTE)
        get_filename_component(src_folder ${src_filepath} PATH)

        set(bin_dir "${CMAKE_CURRENT_BINARY_DIR}/grpc_codegen")
        set(proto_cpp "${bin_dir}/${basename}.pb.cc")
        set(proto_h "${bin_dir}/${basename}.pb.h")
        set(grpc_cpp "${bin_dir}/${basename}.grpc.pb.cc")
        set(grpc_h "${bin_dir}/${basename}.grpc.pb.h")

        add_custom_command(
          OUTPUT "${bin_dir}" COMMAND ${CMAKE_COMMAND} ARGS -E make_directory
                                      "${bin_dir}")

        add_custom_command(
          OUTPUT "${proto_cpp}" "${proto_h}" "${grpc_cpp}" "${grpc_h}"
          COMMAND
            ${_HELPER_PROTOC} ARGS --grpc_out "${bin_dir}" --cpp_out
            "${bin_dir}" -I ${src_folder}
            --plugin=protoc-gen-grpc="${_HELPER_GRPC_CPP_PLUGIN}"
            "${src_filepath}"
          MAIN_DEPENDENCY "${src_filepath}"
          DEPENDS "${bin_dir}")

        add_library(${target_name} OBJECT)
        target_include_directories(${target_name} PUBLIC ${bin_dir})
        target_link_libraries(
          ${target_name} PUBLIC gRPC::grpc++_reflection gRPC::grpc++_unsecure
                                protobuf::libprotobuf)
        target_sources(${target_name} PRIVATE "${proto_cpp}" "${grpc_cpp}")
        target_compile_definitions(${target_name} PRIVATE -D_WIN32_WINNT=0x0700)
        target_compile_definitions(${target_name} PRIVATE -DNTDDI_VERSION=0x06030000)

      endif()

      target_link_libraries(${ARGV0} PUBLIC ${target_name})
      add_dependencies(${ARGV0} ${target_name})
    endif()
  endforeach()

  list(FILTER _sources EXCLUDE REGEX ".*\.proto")
  set_target_properties(${ARGV0} PROPERTIES SOURCES ${_sources})
endfunction()
