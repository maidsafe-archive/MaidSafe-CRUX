###############################################################################
#
# Copyright (C) 2014 MaidSafe.net Limited
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
###############################################################################

if(CMAKE_PROJECT_NAME STREQUAL maidsafe)
  include("MaidSafe CMakeLists.txt")
  return()
endif()

cmake_minimum_required(VERSION 2.8)
project(crux CXX)

set(CRUX_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(CRUX_BUILD_DIR ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CRUX_BUILD_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CRUX_BUILD_DIR}/bin)

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -Wall")
endif()
#add_definitions(-fmax-errors=1) # gcc
#add_definitions(-ferror-limit=1) # clang

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

###############################################################################
# Boost package
###############################################################################

set(Boost_COMPONENTS coroutine context system thread)

# If on Windows, rather than be annoyed by constant failure to find DLL errors
# during test debugging simply force all static linking of Boost libraries
if(WIN32)
  set(Boost_USE_STATIC_LIBS ON)
else()
  set(Boost_COMPONENTS ${Boost_COMPONENTS} unit_test_framework)
  add_definitions (-DBOOST_TEST_DYN_LINK=1)
endif()

# FIXME: Probably ok to use older version
find_package(Boost 1.54.0 COMPONENTS ${Boost_COMPONENTS})
if (NOT ${Boost_FOUND})
  message(FATAL_ERROR "${Boost_ERROR_REASON}\nNote on Windows you must stage Boost like this for cmake to find it:\n    b2 link=shared stage\nAnd set the environment variable BOOST_ROOT to the base of your Boost installation.")
endif()
include_directories(BEFORE ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES})
set(TEST_LIBS ${Boost_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
if(MINGW)
  set(EXTRA_LIBS ${EXTRA_LIBS} ws2_32)
  set(TEST_LIBS ${TEST_LIBS} ws2_32)
endif()
# BSD doesn't link pthread by default, so make it explicit
if(NOT WIN32)
  set(EXTRA_LIBS ${EXTRA_LIBS} pthread)
  set(TEST_LIBS ${TEST_LIBS} pthread)
endif()

# Workaround
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 3.5.0)
    # Asio uses an outdated check for std::array, which fails to detect support
    # of std::array in clang 3.5 with libstdc++
    add_definitions(-DBOOST_ASIO_HAS_STD_ARRAY=1)
    add_definitions(-DBOOST_ASIO_HAS_STD_CHRONO=1)
  elseif(CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 3.4.0)
    add_definitions(-DBOOST_ASIO_HAS_STD_ARRAY=1)
    add_definitions(-DBOOST_ASIO_HAS_STD_CHRONO=1)
  endif()
endif()

if (WIN32)
  add_definitions(/D_WIN32_WINNT=0x0501)
endif()

###############################################################################
# Crux package
###############################################################################

set(CRUX_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include_directories(BEFORE include)
link_directories(${CRUX_LIB_DIR})

add_library(crux STATIC
  src/service.cpp
)

###############################################################################
# Submodules
###############################################################################

add_subdirectory(test)
add_subdirectory(example)
