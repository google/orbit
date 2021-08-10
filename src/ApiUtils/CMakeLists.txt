add_library(ApiUtils STATIC)
target_compile_options(ApiUtils PRIVATE ${STRICT_COMPILE_FLAGS})
target_compile_features(ApiUtils PUBLIC cxx_std_17)

target_sources(ApiUtils PUBLIC
        include/ApiUtils/Event.h
        include/ApiUtils/EncodedEvent.h
        include/ApiUtils/EncodedString.h)
target_sources(ApiUtils PRIVATE
        EncodedString.cpp)
target_link_libraries(ApiUtils PUBLIC
        GrpcProtos
        OrbitBase)
target_include_directories(ApiUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(ApiUtilsTests)

target_compile_options(ApiUtilsTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(ApiUtilsTests PRIVATE
        EncodedEventTest.cpp
        EncodedStringTest.cpp)

target_link_libraries(ApiUtilsTests PRIVATE
        ApiUtils
        GTest::Main)

register_test(ApiUtilsTests)