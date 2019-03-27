enable_language(ASM_MASM)

set(HEADERS OrbitAsm.h)
set(SOURCES Orbit.asm OrbitAsm.cpp)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(SOURCES Orbit.asm OrbitAsm.cpp)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(SOURCES OrbitAsm.cpp)
endif()

set(HDE_FILES
    ${EXTERN_ROOT}/minhook/src/HDE/hde32.h
    ${EXTERN_ROOT}/minhook/src/HDE/hde32.c
    ${EXTERN_ROOT}/minhook/src/HDE/hde64.h
    ${EXTERN_ROOT}/minhook/src/HDE/hde64.c
    ${EXTERN_ROOT}/minhook/src/HDE/table32.h
    ${EXTERN_ROOT}/minhook/src/HDE/table64.h
    ${EXTERN_ROOT}/minhook/src/HDE/pstdint.h
)

set(MINHOOK_FILES
    ${EXTERN_ROOT}/minhook/include/MinHook.h
    ${EXTERN_ROOT}/minhook/src/buffer.h
    ${EXTERN_ROOT}/minhook/src/buffer.c
    ${EXTERN_ROOT}/minhook/src/hook.c
    ${EXTERN_ROOT}/minhook/src/trampoline.h
    ${EXTERN_ROOT}/minhook/src/trampoline.c
)

source_group("HDE"          FILES ${HDE_FILES})
source_group("MinHook"      FILES ${MINHOOK_FILES})
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library(OrbitAsm STATIC 
	${SOURCES} ${HEADERS} ${HDE_FILES} ${MINHOOK_FILES}
	)

#set_source_files_properties(Orbit.asm PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
