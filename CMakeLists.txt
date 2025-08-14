# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Chip8-Emulator")

#
# Dependencies
#

# Raylib
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
  endif()
endif()

# Raygui
find_package(raygui QUIET)
if (NOT raygui_FOUND)
  FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG master
  )
  FetchContent_GetProperties(raygui)
  if (NOT raygui_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raygui)
  endif()
endif()
include_directories(${raygui_SOURCE_DIR}/src)

# spdlog
find_package(spdlog QUIET)
if (NOT spdlog_FOUND)
  FetchContent_Declare(
    spdlog
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.15.2.zip
  )
  FetchContent_GetProperties(spdlog)
  if (NOT spdlog_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(spdlog)
  endif()
endif()
set_target_properties(spdlog PROPERTIES FOLDER "Dependencies")

# nativefiledialog-extended
find_package(nfd QUIET)
if (NOT nfd_FOUND)
  FetchContent_Declare(
    nativefiledialog-extended
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG master
  )
  FetchContent_GetProperties(nativefiledialog-extended)
  if (NOT nativefiledialog-extended_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(nativefiledialog-extended)
  endif()
endif()

# Include sub-projects.
add_subdirectory ("src")


