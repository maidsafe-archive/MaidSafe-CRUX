###############################################################################
#
# Copyright (C) 2014 MaidSafe.net Limited
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
###############################################################################

project(crux-test)

include_directories(".")

###############################################################################
# Tests
###############################################################################

add_executable(crux_test
  runner.cpp
  timer.cpp
  sequence_number.cpp
  concatenate.cpp
  cumulative_set_suite.cpp
  sequence_number.cpp
  socket.cpp
)
add_definitions (-DBOOST_TEST_DYN_LINK)
add_dependencies(crux_test crux)
target_link_libraries(crux_test crux ${TEST_LIBS})

