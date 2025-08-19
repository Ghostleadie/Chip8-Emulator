# CMakeList.txt : CMake project for RaylibApp, include source and define
# project specific logic here.
#

include_directories(${PROJECT_SOURCE_DIR}/src)

# Add source to this project's executable.
add_executable(Chip8-Emulator "chip8.cpp" "chip8.h" "log/log.h" "main.cpp" "gui.cpp" "gui.h" "display.cpp" "display.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Chip8-Emulator PROPERTY CXX_STANDARD 20)
endif()

target_precompile_headers(Chip8-Emulator PUBLIC pch.h)


target_include_directories(${PROJECT_NAME} PRIVATE ${nativefiledialog-extended_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib nfd::nfd)

# Add DEBUG_BUILD only when building the Debug configuration
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)

# Adds MINGW_BUILD define to this target only when using MinGW
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<BOOL:${MINGW}>:MINGW_BUILD>)

# TODO: Add tests and install targets if needed.
