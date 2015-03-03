///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_CONFIG_HPP
#define MAIDSAFE_CRUX_DETAIL_CONFIG_HPP

#include <boost/config.hpp>

#if (defined(MAIDSAFE_CRUX_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && \
    !defined(MAIDSAFE_CRUX_STATIC_LINK)

# if defined(MAIDSAFE_CRUX_SOURCE)
#  undef MAIDSAFE_CRUX_HEADERS_ONLY
#  define MAIDSAFE_CRUX_DECL BOOST_SYMBOL_EXPORT
#  define MAIDSAFE_CRUX_BUILD_DLL
# else
#  define MAIDSAFE_CRUX_DECL
# endif
#else
# define MAIDSAFE_CRUX_DECL
#endif

#if !defined(MAIDSAFE_CRUX_SOURCE) && !defined(BOOST_ALL_NO_LIB) && \
    !defined(MAIDSAFE_CRUX_NO_LIB) && !MAIDSAFE_CRUX_HEADERS_ONLY

#define BOOST_LIB_NAME maidsafe_crux

// tell the auto-link code to select a dll when required:
#if defined(BOOST_ALL_DYN_LINK) || defined(MAIDSAFE_CRUX_DYN_LINK)
#define BOOST_DYN_LINK
#endif

#include <boost/config/auto_link.hpp>

#endif  // auto-linking disabled

#endif
