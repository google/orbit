add_library(OrbitPlugin STATIC)

target_sources(OrbitPlugin PRIVATE OrbitData.h OrbitSDK.h OrbitUserData.h UserPlugin.h)
target_sources(OrbitPlugin PUBLIC UserPlugin.cpp)

target_link_libraries(OrbitPlugin PUBLIC OrbitCore imgui::imgui)