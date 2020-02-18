add_library(OrbitPlugin STATIC)

target_sources(OrbitPlugin PRIVATE UserPlugin.h UserPlugin.cpp)
target_link_libraries(OrbitPlugin PUBLIC OrbitCore imgui::imgui)
