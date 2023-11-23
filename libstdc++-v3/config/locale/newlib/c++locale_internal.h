// Locale internal implementation header -*- C++ -*-

// Copyright (C) 2002-2023 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file bits/c++locale_internal.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{locale}
 */

// Written by Jakub Jelinek <jakub@redhat.com>
// Modified by Takashi Yano <takashi.yano@nifty.ne.jp>

#include <bits/c++config.h>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <langinfo.h>

#include <vector>
#include <string.h>	// ::strdup

#include <ext/concurrence.h>

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  struct Catalog_info
  {
    Catalog_info(messages_base::catalog __id, const char* __domain,
		 locale __loc)
      : _M_id(__id), _M_domain(strdup(__domain)), _M_locale(__loc)
    { }

    ~Catalog_info()
    { free(_M_domain); }

    messages_base::catalog _M_id;
    char* _M_domain;
    locale _M_locale;

  private:
    Catalog_info(const Catalog_info&);

    Catalog_info&
    operator=(const Catalog_info&);
  };

  class Catalogs
  {
  public:
    Catalogs() : _M_catalog_counter(0) { }
    ~Catalogs();

    messages_base::catalog
    _M_add(const char* __domain, locale __l);

    void
    _M_erase(messages_base::catalog __c);

    const Catalog_info*
    _M_get(messages_base::catalog __c) const;

  private:
    mutable __gnu_cxx::__mutex _M_mutex;
    messages_base::catalog _M_catalog_counter;
    vector<Catalog_info*> _M_infos;
  };

  Catalogs&
  get_catalogs();

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace
