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

add_executable(test_core
  core.cpp
)
add_definitions (-DBOOST_TEST_DYN_LINK)
add_dependencies(test_core crux)
target_link_libraries(test_core crux ${TEST_LIBS})

