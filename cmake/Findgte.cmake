add_library(gte INTERFACE)

set(DIR external/gte)
target_include_directories(gte SYSTEM INTERFACE ${DIR})

add_library(gte::gte ALIAS gte)
