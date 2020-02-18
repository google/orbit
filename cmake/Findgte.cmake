add_library(gte INTERFACE IMPORTED GLOBAL)
target_include_directories(gte SYSTEM INTERFACE external/gte)
target_compile_features(gte INTERFACE cxx_std_11)

add_library(gte::gte ALIAS gte)
