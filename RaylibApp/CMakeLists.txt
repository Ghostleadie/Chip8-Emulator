# CMakeList.txt : CMake project for RaylibApp, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (RaylibApp "RaylibApp.cpp" "RaylibApp.h" "log/log.cpp" "log/log.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RaylibApp PROPERTY CXX_STANDARD 20)
endif()

target_precompile_headers(RaylibApp PUBLIC pch.h)

target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} spdlog)
# TODO: Add tests and install targets if needed.
