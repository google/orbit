add_library(peparse OBJECT)

set(DIR external/peparse)

target_sources(peparse PRIVATE ${DIR}/buffer.cpp ${DIR}/dump.cpp
                               ${DIR}/parse.cpp)
target_sources(peparse PUBLIC ${DIR}/parse.h ${DIR}/nt-headers.h
                              ${DIR}/to_string.h)

target_include_directories(peparse SYSTEM PUBLIC ${DIR})

# FIXME: peparse depends on OrbitCore, hence it should be pulled into OrbitCore.
target_link_libraries(
  peparse PUBLIC multicore::multicore concurrentqueue::concurrentqueue
                 oqpi::oqpi xxHash::xxHash cereal::cereal)
target_include_directories(peparse PRIVATE "OrbitCore/" "OrbitBase/include/")

add_library(peparse::peparse ALIAS peparse)
