###############################################################################
#
# Copyright (C) 2014 MaidSafe.net Limited
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
###############################################################################

project(crux-example)

include_directories(".")

###############################################################################
# Async examples
###############################################################################

add_executable(async_echo_server
  async/echo_server.cpp
)
add_dependencies(async_echo_server crux)
target_link_libraries(async_echo_server crux ${EXTRA_LIBS})

add_executable(async_echo_client
  async/echo_client.cpp
)
add_dependencies(async_echo_client crux)
target_link_libraries(async_echo_client crux ${EXTRA_LIBS})

###############################################################################
# Coroutine examples
###############################################################################

add_executable(coroutine_echo_client
  coroutine/coroutine_echo_client.cpp
)
add_dependencies(coroutine_echo_client crux)
target_link_libraries(coroutine_echo_client crux ${EXTRA_LIBS})
if(MSVC)
  set_target_properties(coroutine_echo_client PROPERTIES LINK_FLAGS "${LINK_FLAGS} /SAFESEH:NO")
endif()

###############################################################################
# Future examples
###############################################################################

add_executable(future_echo_client
  future/future_echo_client.cpp
)
add_dependencies(future_echo_client crux)
target_link_libraries(future_echo_client crux ${EXTRA_LIBS})
