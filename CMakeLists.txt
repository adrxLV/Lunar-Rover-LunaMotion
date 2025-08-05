cmake_minimum_required(VERSION 3.29)

include(cmake/prelude.cmake)
include(FetchContent)

FetchContent_Declare(cmake_git_version_tracking                   
  GIT_REPOSITORY https://github.com/andrew-hardin/cmake-git-version-tracking.git
  GIT_TAG 6c0cb87edd029ddfb403a8e24577c144a03605a6
)

project(
    lunar-rover-nav
    VERSION 0.1.0
    DESCRIPTION "Lunar Rover Navigation"
    LANGUAGES CXX
)

FetchContent_MakeAvailable(cmake_git_version_tracking)

include(cmake/variables.cmake)

if (MSVC)
  add_compile_definitions(_WIN32_WINNT=0xA000)
endif()

find_package(Boost 1.86.0 COMPONENTS asio json log program_options REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ---- Declare library ----

add_library(
    lrn_lib OBJECT
)

target_sources(lrn_lib
    PRIVATE
    liblrn/lrn-config.cpp
    liblrn/lrn.cpp
    liblrn/potential-field-navigator.cpp
    liblrn/potential-field-navigator-config.cpp
    liblrn/rover-config.cpp
    liblrn/rover-platform-config.cpp
    liblrn/rover-remote-config.cpp
    liblrn/rover-executor.cpp
    liblrn/rover.cpp
)

target_sources(lrn_lib
  PUBLIC
  FILE_SET HEADERS
    TYPE HEADERS
    FILES  
    liblrn/json-extract.hpp
    liblrn/lrn-config.hpp
    liblrn/lrn.hpp
    liblrn/potential-field-navigator.hpp
    liblrn/potential-field-navigator-config.hpp
    liblrn/rover-config.hpp
    liblrn/rover-platform-config.hpp
    liblrn/rover-remote-config.hpp
    liblrn/rover-executor.hpp
    liblrn/rover.hpp
)

target_link_libraries(lrn_lib 
  PUBLIC
    Boost::boost
    Boost::json
    Boost::log
    Boost::asio
    fmt::fmt
    cppzmq
    nlohmann_json::nlohmann_json
)



# ---- Declare executable ----

add_executable(lrn_exe lrn/lrn.cpp)
add_executable(lrn::exe ALIAS lrn_exe)

set_property(TARGET lrn_exe PROPERTY OUTPUT_NAME lunar-rover-nav)

target_compile_features(lrn_exe PRIVATE cxx_std_20)

target_link_libraries(lrn_exe PRIVATE
  lrn_lib
  Boost::boost
  Boost::log
  Boost::program_options
  Boost::json
  fmt::fmt
)
target_link_libraries(lrn_exe PRIVATE cmake_git_version_tracking)

# ---- Developer mode ----

if(NOT lrn_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of lrn"
  )
endif()

include(cmake/dev-mode.cmake)
