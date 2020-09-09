#Copyright(c) 2020 The Orbit Authors.All rights reserved.
#Use of this source code is governed by a BSD - style license that can be
#found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

    project(OrbitCore) add_library(OrbitCore STATIC)

        target_compile_options(OrbitCore PRIVATE ${STRICT_COMPILE_FLAGS})

            target_sources(OrbitCore PUBLIC BlockChain.h Callstack.h CallstackData.h CallstackTypes
                               .h CaptureData.h EventBuffer.h FunctionUtils.h OrbitModule
                               .h OrbitProcess.h Params.h Path.h Pdb.h RingBuffer.h SamplingProfiler
                               .h ScopeTimer.h StringManager.h SymbolHelper.h Threading
                               .h TracepointCustom.h TracepointInfoManager.h Utils.h)

                target_sources(OrbitCore PRIVATE
                                   CallstackData.cpp CaptureData.cpp EventBuffer.cpp FunctionUtils
                                       .cpp OrbitModule.cpp OrbitProcess.cpp Params.cpp Path.cpp Pdb
                                       .cpp SamplingProfiler.cpp ScopeTimer.cpp StringManager
                                       .cpp SymbolHelper.cpp TracepointInfoManager.cpp Utils.cpp)

                    target_include_directories(OrbitCore PUBLIC ${CMAKE_CURRENT_LIST_DIR})

                        target_link_libraries(
                            OrbitCore PUBLIC ElfUtils OrbitBase OrbitClientProtos OrbitProtos
                                xxHash::xxHash concurrentqueue::concurrentqueue multicore::multicore
                                    oqpi::oqpi abseil::abseil llvm_object::llvm_object
                                        Outcome::Outcome)

                            if (WIN32) target_link_libraries(OrbitCore PRIVATE dbghelp.lib) target_compile_definitions(
                                OrbitCore PUBLIC -
                                    D_WIN32_WINNT = 0x0700) target_compile_definitions(OrbitCore
                                                                                               PUBLIC -
                                                                                           DNTDDI_VERSION =
                                                                                           0x06030000) endif()

                                target_compile_definitions(
                                    OrbitCore PUBLIC
                                        WIN32_LEAN_AND_MEAN) target_compile_definitions(OrbitCore PUBLIC
                                                                                            INITGUID)
                                    target_compile_definitions(OrbitCore PUBLIC NOMINMAX) target_compile_definitions(
                                        OrbitCore PUBLIC
                                            UNICODE) target_compile_definitions(OrbitCore PUBLIC
                                                                                    _UNICODE)

                                        target_compile_features(OrbitCore PUBLIC cxx_std_11)

                                            add_executable(OrbitCoreTests)

                                                target_compile_options(OrbitCoreTests PRIVATE ${
                                                    STRICT_COMPILE_FLAGS})

                                                    target_sources(
                                                        OrbitCoreTests PRIVATE
                                                            BlockChainTest
                                                                .cpp PathTest.cpp RingBufferTest
                                                                .cpp StringManagerTest
                                                                .cpp SymbolHelperTest.cpp UtilsTest
                                                                .cpp)

                                                        if (NOT WIN32) target_sources(
                                                            OrbitCoreTests PRIVATE
                                                                OrbitModuleTest.cpp) endif()

                                                            target_link_libraries(
                                                                OrbitCoreTests PRIVATE OrbitCore GTest::Main
                                                                    llvm_object::llvm_object abseil::
                                                                        abseil)

                                                                add_custom_command(
                                                                    TARGET OrbitCoreTests POST_BUILD COMMAND
                                                                                ${CMAKE_COMMAND} -
                                                                            E remove_directory $ <
                                                                        TARGET_FILE_DIR
                                                                    : OrbitCoreTests >
                                                                          / testdata / OrbitCore)

                                                                    add_custom_command(
                                                                        TARGET OrbitCoreTests POST_BUILD COMMAND
                                                                                    ${CMAKE_COMMAND} -
                                                                                E copy_directory ${
                                                                                    CMAKE_SOURCE_DIR} /
                                                                                    OrbitCore /
                                                                                    testdata / $ <
                                                                            TARGET_FILE_DIR
                                                                        : OrbitCoreTests >
                                                                              / testdata /
                                                                                  OrbitCore)

                                                                        register_test(
                                                                            OrbitCoreTests)

                                                                            add_fuzzer(
                                                                                ModuleLoadSymbolsFuzzer
                                                                                    ModuleLoadSymbolsFuzzer
                                                                                        .cpp)
                                                                                target_link_libraries(
                                                                                    ModuleLoadSymbolsFuzzer
                                                                                        PRIVATE OrbitCore
                                                                                            libprotobuf -
                                                                                    mutator::
                                                                                        libprotobuf -
                                                                                    mutator)
