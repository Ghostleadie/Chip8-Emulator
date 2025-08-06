# CMakeList.txt : CMake project for RaylibApp, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Chip8-Emulator "chip8.cpp" "chip8.h" "log/log.cpp" "log/log.h" "main.cpp" "font.cpp" "font.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Chip8-Emulator PROPERTY CXX_STANDARD 20)
endif()

target_precompile_headers(Chip8-Emulator PUBLIC pch.h)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib spdlog)

# TODO: Add tests and install targets if needed.
