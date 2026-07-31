# Build static lib only
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

FetchContent_Declare(sdl2_net
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL_net"
    GIT_TAG "release-2.2.x"
)

FetchContent_GetProperties(sdl2_net)
if (NOT sdl2_net_POPULATED)
    FetchContent_Populate(sdl2_net)
endif()

set(SDL2_DIR ${sdl2_BINARY_DIR} CACHE PATH "" FORCE)

add_subdirectory(${sdl2_net_SOURCE_DIR} ${sdl2_net_BINARY_DIR} EXCLUDE_FROM_ALL)

set(SDL2_NET_INCLUDE_DIRS ${sdl2_net_SOURCE_DIR} PARENT_SCOPE)
set(SDL2_NET_LIBRARIES SDL2_net PARENT_SCOPE)