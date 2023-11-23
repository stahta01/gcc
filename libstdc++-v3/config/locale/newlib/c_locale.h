// Wrapper for underlying C-language localization -*- C++ -*-

// Copyright (C) 2001-2023 Free Software Foundation, Inc.
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

/** @file bits/c++locale.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{locale}
 */

//
// ISO C++ 14882: 22.8  Standard locale categories.
//

// Written by Benjamin Kosnik <bkoz@redhat.com>
// Modified by Takashi Yano <takashi.yano@nifty.ne.jp>

#ifndef _GLIBCXX_CXX_LOCALE_H
#define _GLIBCXX_CXX_LOCALE_H 1

#pragma GCC system_header

#include <clocale>

#if __POSIX_VISIBLE < 200809
struct __locale_t;
typedef struct __locale_t *locale_t;

#define LC_ALL_MASK		(1 << LC_ALL)
#define LC_COLLATE_MASK		(1 << LC_COLLATE)
#define LC_CTYPE_MASK		(1 << LC_CTYPE)
#define LC_MONETARY_MASK	(1 << LC_MONETARY)
#define LC_NUMERIC_MASK		(1 << LC_NUMERIC)
#define LC_TIME_MASK		(1 << LC_TIME)
#define LC_MESSAGES_MASK	(1 << LC_MESSAGES)

extern "C" locale_t newlocale(int, const char *, locale_t);
extern "C" void freelocale(locale_t);
extern "C" locale_t duplocale(locale_t);
extern "C" locale_t uselocale(locale_t);
#endif /* __POSIX_VISIBLE < 200809 */

#define _GLIBCXX_NUM_CATEGORIES 0

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  typedef locale_t __c_locale;

  // Convert numeric value of type double and long double to string and
  // return length of string.  If vsnprintf is available use it, otherwise
  // fall back to the unsafe vsprintf which, in general, can be dangerous
  // and should be avoided.
  inline int
  __convert_from_v(const __c_locale& __cloc __attribute__ ((__unused__)),
		   char* __out,
		   const int __size __attribute__ ((__unused__)),
		   const char* __fmt, ...)
  {
    __c_locale __old = uselocale(__cloc);

    __builtin_va_list __args;
    __builtin_va_start(__args, __fmt);

#if _GLIBCXX_USE_C99_STDIO
    const int __ret = __builtin_vsnprintf(__out, __size, __fmt, __args);
#else
    const int __ret = __builtin_vsprintf(__out, __fmt, __args);
#endif

    __builtin_va_end(__args);

    uselocale(__old);
    return __ret;
  }

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

#endif
