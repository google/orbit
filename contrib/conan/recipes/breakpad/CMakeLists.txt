cmake_minimum_required(VERSION 3.8)
project(symbols_uploading)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(
    -DNOMINMAX
    -DUNICODE
    -DWIN32_LEAN_AND_MEAN
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_LIBCPP_VERSION
)

if(WIN32)
	set (DIA_SDK_DIR 
		"${VSINSTALLDIR}/DIA SDK"
	)
	include_directories(src/ src/common/windows ${DIA_SDK_DIR}/include)

	set(COMMON_LIB_SRC
		src/common/windows/dia_util.cc
		src/common/windows/guid_string.cc
		src/common/windows/http_upload.cc
		src/common/windows/omap.cc
		src/common/windows/pdb_source_line_writer.cc
		src/common/windows/pe_source_line_writer.cc
		src/common/windows/pe_util.cc
		src/common/windows/string_utils.cc
		src/common/windows/symbol_collector_client.cc
	)
	find_library(DIAGUIDS_LIB diaguids PATHS ${DIA_SDK_DIR}/lib/amd64)
	set (COMMON_LIB_LIB
		${DIAGUIDS_LIB}
		Imagehlp
	)
	
	set(DUMP_SYMS_SRC
		src/tools/windows/dump_syms/dump_syms.cc
	)
	
	set(SYMUPLOAD_SRC
		src/tools/windows/symupload/symupload.cc
	)
	set (SYMUPLOAD_LIB
		Wininet
		Version
	)
elseif(UNIX)
	add_definitions(
		-DHAVE_A_OUT_H
		-DHAVE_GETCONTEXT
	)
	include_directories(src/ src/common src/common/dwarf src/common/linux)
	
	set(COMMON_LIB_SRC
		src/common/convert_UTF.cc
		src/common/dwarf/bytereader.cc
		src/common/dwarf/cfi_assembler.cc
		src/common/dwarf/dwarf2diehandler.cc
		src/common/dwarf/dwarf2reader.cc
		src/common/dwarf/elf_reader.cc
		src/common/dwarf/functioninfo.cc
		src/common/dwarf_cfi_to_module.cc
		src/common/dwarf_cu_to_module.cc
		src/common/dwarf_line_to_module.cc
		src/common/dwarf_range_list_handler.cc
		src/common/language.cc
		src/common/linux/crc32.cc
		src/common/linux/dump_symbols.cc
		src/common/linux/elf_core_dump.cc
		src/common/linux/elf_symbols_to_module.cc
		src/common/linux/elfutils.cc
		src/common/linux/file_id.cc
		src/common/linux/google_crashdump_uploader.cc
		src/common/linux/guid_creator.cc
		src/common/linux/http_upload.cc
		src/common/linux/libcurl_wrapper.cc
		src/common/linux/linux_libc_support.cc
		src/common/linux/memory_mapped_file.cc
		src/common/linux/safe_readlink.cc
		src/common/linux/synth_elf.cc
		src/common/linux/symbol_upload.cc
		src/common/long_string_dictionary.cc
		src/common/md5.cc
		src/common/module.cc
		src/common/path_helper.cc
		src/common/simple_string_dictionary.cc
		src/common/stabs_reader.cc
		src/common/stabs_to_module.cc
		src/common/string_conversion.cc
		src/common/test_assembler.cc
	)

	set(DUMP_SYMS_SRC
		src/tools/linux/dump_syms/dump_syms.cc
	)

	set(SYMUPLOAD_SRC
		src/tools/linux/symupload/sym_upload.cc
	)
	set (SYMUPLOAD_LIB
		dl
	)
endif()

add_library(common STATIC ${COMMON_LIB_SRC})
target_link_libraries(common ${COMMON_LIB_LIB})

add_executable(dump_syms ${DUMP_SYMS_SRC})
target_link_libraries(dump_syms common)

add_executable(symupload ${SYMUPLOAD_SRC})
target_link_libraries(symupload common ${SYMUPLOAD_LIB})