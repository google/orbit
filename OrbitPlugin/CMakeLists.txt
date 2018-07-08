set(HEADERS 
    OrbitData.h
    OrbitSDK.h
    OrbitUserData.h
    UserPlugin.h
)
set(SOURCES UserPlugin.cpp)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library(OrbitPlugin STATIC ${SOURCES})
