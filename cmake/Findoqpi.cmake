add_library(oqpi INTERFACE)

set(DIR external/oqpi)
target_include_directories(oqpi SYSTEM INTERFACE ${DIR}/include)

target_compile_definitions(oqpi INTERFACE WIN32_LEAN_AND_MEAN)
target_compile_definitions(oqpi INTERFACE VC_EXTRALEAN)

add_library(oqpi::oqpi ALIAS oqpi)
