add_library(minhook OBJECT)

set(DIR external/minhook)

target_sources(
  minhook
  PRIVATE ${DIR}/src/HDE/hde32.c
          ${DIR}/src/HDE/hde64.c
          ${DIR}/src/buffer.h
          ${DIR}/src/buffer.c
          ${DIR}/src/hook.c
          ${DIR}/src/trampoline.h
          ${DIR}/src/trampoline.c)
target_sources(minhook PUBLIC ${DIR}/include/MinHook.h)

target_include_directories(minhook SYSTEM PUBLIC ${DIR}/include)

add_library(minhook::minhook ALIAS minhook)
