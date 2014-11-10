###############################################################################
#
# Copyright (C) 2014 MaidSafe.net Limited
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
###############################################################################

cmake_minimum_required(VERSION 2.8)
project(crux CXX)

set(CRUX_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(CRUX_BUILD_DIR ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CRUX_BUILD_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CRUX_BUILD_DIR}/bin)

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -Wall")
endif()
add_definitions(-g)
add_definitions(-std=c++11)
#add_definitions(-fmax-errors=1) # gcc
#add_definitions(-ferror-limit=1) # clang

###############################################################################
# Boost package
###############################################################################

# FIXME: Probably ok to use older version
find_package(Boost 1.54.0 COMPONENTS coroutine context system thread unit_test_framework)
if (NOT ${Boost_FOUND})
  message(FATAL_ERROR "Boost not found (or too old)")
endif()
include_directories(BEFORE ${Boost_INCLUDE_DIR})
set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES})
set(TEST_LIBS ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_LIBRARIES})

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
