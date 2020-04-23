set(DIR external/DIA2Dump)

add_library(DIA2Dump OBJECT)
target_sources(DIA2Dump PUBLIC ${DIR}/Callback.h ${DIR}/DIA2Dump.h
                               ${DIR}/PrintSymbol.h ${DIR}/regs.h)

target_sources(DIA2Dump PRIVATE ${DIR}/dia2dump.cpp ${DIR}/PrintSymbol.cpp
                                ${DIR}/regs.cpp)

target_include_directories(DIA2Dump SYSTEM PUBLIC ${DIR})
target_link_libraries(DIA2Dump PUBLIC DIASDK::DIASDK)

# FIXME: DIA2Dump depends on OrbitCore and OrbitGl, hence it should be pulled
# into OrbitGl.
target_link_libraries(
  DIA2Dump PUBLIC multicore::multicore concurrentqueue::concurrentqueue
                  oqpi::oqpi xxHash::xxHash cereal::cereal)
target_include_directories(DIA2Dump PRIVATE "OrbitCore/" "OrbitBase/include/")

add_library(DIA2Dump::DIA2Dump ALIAS DIA2Dump)
