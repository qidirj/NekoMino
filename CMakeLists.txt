cmake_minimum_required(VERSION 3.28)
project(NekoMino LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
  $<$<CONFIG:Debug>:-g>
  $<$<CONFIG:Release>:-O2>
)

# Define DEBUG only in Debug builds
# Use cmake --build . --config Release for multi-config generators
add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

# include(FetchContent)
# FetchContent_Declare(SFML
#     GIT_REPOSITORY https://github.com/SFML/SFML.git
#     GIT_TAG 3.1.0
#     GIT_SHALLOW ON
#     EXCLUDE_FROM_ALL
#     SYSTEM)
# FetchContent_MakeAvailable(SFML)

add_executable(nekomino
  src/main.cpp
  src/config/config.cpp
  src/tetris_core/board.cpp
  src/tetris_core/core.cpp
  src/tetris_core/minoes.cpp
  src/tetris_core/rs.cpp
  src/ui_core/interface.cpp
  src/util/error_handling.cpp
  src/util/localization.cpp)
target_compile_features(nekomino PRIVATE cxx_std_17)
find_package(SFML 3.1 COMPONENTS Graphics REQUIRED)
target_link_libraries(nekomino PRIVATE SFML::Graphics)

# Allow `make debug` and `make release` after running `cmake .`
add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build .
    USES_TERMINAL
)

add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build .
    USES_TERMINAL
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT nekomino)
