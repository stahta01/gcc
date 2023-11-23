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

//
// ISO C++ 14882: 22.8  Standard locale categories.
//

// Written by Benjamin Kosnik <bkoz@redhat.com>
// Modified by Takashi Yano <takashi.yano@nifty.ne.jp>

#define _GNU_SOURCE 1
#include <cstdio>
#include <locale>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <langinfo.h>
#include <bits/c++locale_internal.h>

#include <backward/auto_ptr.h>

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

/* Newlib constant */
#define ENCODING_LEN 31
#define CATEGORY_LEN 11
#define NUM_CATEGORIES 6
#define xstr(s) str(s)
#define str(s) #s
#define FMT_LOCALE "%" xstr(CATEGORY_LEN) "[^=]=%" xstr(ENCODING_LEN) "[^;]"

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  template<>
    void
    __convert_to_v(const char* __s, float& __v, ios_base::iostate& __err,
		   const __c_locale& __cloc) throw()
    {
      char* __sanity;
      __v = strtof_l(__s, &__sanity, __cloc);

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 23. Num_get overflow result.
      if (__sanity == __s || *__sanity != '\0')
	{
	  __v = 0.0f;
	  __err = ios_base::failbit;
	}
      else if (__v == numeric_limits<float>::infinity())
	{
	  __v = numeric_limits<float>::max();
	  __err = ios_base::failbit;
	}
      else if (__v == -numeric_limits<float>::infinity())
	{
	  __v = -numeric_limits<float>::max();
	  __err = ios_base::failbit;
	}
    }

  template<>
    void
    __convert_to_v(const char* __s, double& __v, ios_base::iostate& __err,
		   const __c_locale& __cloc) throw()
    {
      char* __sanity;
      __v = strtod_l(__s, &__sanity, __cloc);

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 23. Num_get overflow result.
      if (__sanity == __s || *__sanity != '\0')
	{
	  __v = 0.0;
	  __err = ios_base::failbit;
	}
      else if (__v == numeric_limits<double>::infinity())
	{
	  __v = numeric_limits<double>::max();
	  __err = ios_base::failbit;
	}
      else if (__v == -numeric_limits<double>::infinity())
	{
	  __v = -numeric_limits<double>::max();
	  __err = ios_base::failbit;
	}
    }

  template<>
    void
    __convert_to_v(const char* __s, long double& __v, ios_base::iostate& __err,
		   const __c_locale& __cloc) throw()
    {
      char* __sanity;
      __v = strtold_l(__s, &__sanity, __cloc);

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 23. Num_get overflow result.
      if (__sanity == __s || *__sanity != '\0')
	{
	  __v = 0.0l;
	  __err = ios_base::failbit;
	}
      else if (__v == numeric_limits<long double>::infinity())
	{
	  __v = numeric_limits<long double>::max();
	  __err = ios_base::failbit;
	}
      else if (__v == -numeric_limits<long double>::infinity())
	{
	  __v = -numeric_limits<long double>::max();
	  __err = ios_base::failbit;
	}
    }

  void
  locale::facet::_S_create_c_locale(__c_locale& __cloc, const char* __s,
				    __c_locale __old)
  {
    __cloc = newlocale(1 << LC_ALL, __s, __old);
    if (__cloc)
      return;

    /* Newlib does not support locale strig such as
       "LC_CTYPE=en_US.UTF8;LC_NUMERIC=de_DE.UTF8;..." */
    const char *__p = __s;
    __cloc = __old;
    char __cat[CATEGORY_LEN + 1], __loc[ENCODING_LEN + 1];
    while (2 == sscanf(__p, FMT_LOCALE, __cat, __loc))
      {
	struct {const char *__cat; int __mask;} __tbl[NUM_CATEGORIES + 1] =
	  {
	      {"LC_CTYPE", LC_CTYPE_MASK},
	      {"LC_NUMERIC", LC_NUMERIC_MASK},
	      {"LC_TIME", LC_TIME_MASK},
	      {"LC_COLLATE", LC_COLLATE_MASK},
	      {"LC_MONETARY", LC_MONETARY_MASK},
	      {"LC_MESSAGES", LC_MESSAGES_MASK},
	      {NULL, 0}
	  };
	for (int __i = 0; __tbl[__i].__cat; __i++)
	  if (strcmp(__tbl[__i].__cat, __cat) == 0)
	    {
	      __cloc = newlocale(__tbl[__i].__mask, __loc, __cloc);
	      break;
	    }
	if (!__cloc)
	  break;
	if ((__p = strchr(__p, ';')) == NULL)
	  break;
	__p++;
      }
    if (!__cloc)
      {
	// This named locale is not supported by the underlying OS.
	__throw_runtime_error(__N("locale::facet::_S_create_c_locale "
				  "name not valid"));
      }
  }

  void
  locale::facet::_S_destroy_c_locale(__c_locale& __cloc)
  {
    if (__cloc && _S_get_c_locale() != __cloc)
      freelocale(__cloc);
  }

  __c_locale
  locale::facet::_S_clone_c_locale(__c_locale& __cloc) throw()
  { return duplocale(__cloc); }

  __c_locale
  locale::facet::_S_lc_ctype_c_locale(__c_locale __cloc, const char* __s)
  {
    __c_locale __dup = duplocale(__cloc);
    if (__dup == __c_locale(0))
      __throw_runtime_error(__N("locale::facet::_S_lc_ctype_c_locale "
				"duplocale error"));
    __c_locale __changed = newlocale(LC_CTYPE_MASK, __s, __dup);
    if (__changed == __c_locale(0))
      {
	/* Newlib does not support locale strig such as
	   "LC_CTYPE=en_US.UTF8;LC_NUMERIC=de_DE.UTF8;..." */
	const char *__p = __s;
	char __cat[CATEGORY_LEN + 1], __loc[ENCODING_LEN + 1];
	while (2 == sscanf(__p, FMT_LOCALE, __cat, __loc))
	  {
	    if (strcmp("LC_CTYPE", __cat) == 0)
	      {
		__changed = newlocale(LC_CTYPE_MASK, __loc, __dup);
		break;
	      }
	    if ((__p = strchr(__p, ';')) == NULL)
	      break;
	    __p++;
	  }
      }
    if (__changed == __c_locale(0))
      {
	freelocale(__dup);
	__throw_runtime_error(__N("locale::facet::_S_lc_ctype_c_locale "
				  "newlocale error"));
      }
    return __changed;
  }

  struct _CatalogIdComp
  {
    bool
    operator()(messages_base::catalog __cat, const Catalog_info* __info) const
    { return __cat < __info->_M_id; }

    bool
    operator()(const Catalog_info* __info, messages_base::catalog __cat) const
    { return __info->_M_id < __cat; }
  };

  Catalogs::~Catalogs()
  {
    for (vector<Catalog_info*>::iterator __it = _M_infos.begin();
	 __it != _M_infos.end(); ++__it)
      delete *__it;
  }

  messages_base::catalog
  Catalogs::_M_add(const char* __domain, locale __l)
  {
    __gnu_cxx::__scoped_lock lock(_M_mutex);

    // The counter is not likely to roll unless catalogs keep on being
    // opened/closed which is consider as an application mistake for the
    // moment.
    if (_M_catalog_counter == numeric_limits<messages_base::catalog>::max())
      return -1;

    auto_ptr<Catalog_info> info(new Catalog_info(_M_catalog_counter++,
						 __domain, __l));

    // Check if we managed to allocate memory for domain.
    if (!info->_M_domain)
      return -1;

    _M_infos.push_back(info.get());
    return info.release()->_M_id;
  }

  void
  Catalogs::_M_erase(messages_base::catalog __c)
  {
    __gnu_cxx::__scoped_lock lock(_M_mutex);

    vector<Catalog_info*>::iterator __res =
      lower_bound(_M_infos.begin(), _M_infos.end(), __c, _CatalogIdComp());
    if (__res == _M_infos.end() || (*__res)->_M_id != __c)
      return;

    delete *__res;
    _M_infos.erase(__res);

    // Just in case closed catalog was the last open.
    if (__c == _M_catalog_counter - 1)
      --_M_catalog_counter;
  }

  const Catalog_info*
  Catalogs::_M_get(messages_base::catalog __c) const
  {
    __gnu_cxx::__scoped_lock lock(_M_mutex);

    vector<Catalog_info*>::const_iterator __res =
      lower_bound(_M_infos.begin(), _M_infos.end(), __c, _CatalogIdComp());

    if (__res != _M_infos.end() && (*__res)->_M_id == __c)
      return *__res;

    return 0;
  }

  Catalogs&
  get_catalogs()
  {
    static Catalogs __catalogs;
    return __catalogs;
  }

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

namespace __gnu_cxx _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  const char* const category_names[NUM_CATEGORIES] =
    {
      "LC_CTYPE",
      "LC_NUMERIC",
      "LC_TIME",
      "LC_COLLATE",
      "LC_MONETARY",
      "LC_MESSAGES",
    };

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

static char *
__setlocale_for_newlib(int __category, const char *__locale)
{
  char *__ret;
  if ((__ret = setlocale(__category, __locale)) == NULL
      && __category == LC_ALL)
    {
      /* Newlib does not support locale strig such as
	 "LC_CTYPE=en_US.UTF8;LC_NUMERIC=de_DE.UTF8;..." */
      char __buf[NUM_CATEGORIES][ENCODING_LEN + 1];
      const char *__p = __locale;
      char __cat[CATEGORY_LEN + 1], __loc[ENCODING_LEN + 1];
      while (2 == sscanf(__p, FMT_LOCALE, __cat, __loc))
	{
	  const char *__tbl[NUM_CATEGORIES] =
	    {
	      "LC_COLLATE",
	      "LC_CTYPE",
	      "LC_MONETARY",
	      "LC_NUMERIC",
	      "LC_TIME",
	      "LC_MESSAGES"
	    };
	  for (int __i = 0; __i < NUM_CATEGORIES; __i++)
	    if (strcmp(__tbl[__i], __cat) == 0)
	      {
		strncpy(__buf[__i], __loc, sizeof(__buf[__i]));
		__buf[__i][ENCODING_LEN] = '\0';
		break;
	      }
	  if ((__p = strchr(__p, ';')) == NULL)
	    break;
	  __p ++;
	}
      char __newloc[(ENCODING_LEN + 1) * NUM_CATEGORIES];
      sprintf(__newloc, "%s/%s/%s/%s/%s/%s",
	      __buf[0], __buf[1], __buf[2], __buf[3], __buf[4], __buf[5]);
      __ret = setlocale(__category, __newloc);

    }
  return __ret;
}

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  const char* const* const locale::_S_categories = __gnu_cxx::category_names;
  const __typeof(setlocale) *__setlocale = __setlocale_for_newlib;

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

// XXX GLIBCXX_ABI Deprecated
#ifdef _GLIBCXX_LONG_DOUBLE_COMPAT
#define _GLIBCXX_LDBL_COMPAT(dbl, ldbl) \
  extern "C" void ldbl (void) __attribute__ ((alias (#dbl)))
_GLIBCXX_LDBL_COMPAT(_ZSt14__convert_to_vIdEvPKcRT_RSt12_Ios_IostateRKPi, _ZSt14__convert_to_vIeEvPKcRT_RSt12_Ios_IostateRKPi);
#endif // _GLIBCXX_LONG_DOUBLE_COMPAT

/* Retain compatibility with the version without newlib locale support. */
#pragma GCC diagnostic ignored "-Wattribute-alias"
#define DEF_COMPAT(func, target) \
extern "C" __attribute__ ((alias (#target))) void func (void)
DEF_COMPAT(_ZNSt6locale5facet17_S_clone_c_localeERPi, _ZNSt6locale5facet17_S_clone_c_localeERP10__locale_t);
DEF_COMPAT(_ZNSt6locale5facet18_S_create_c_localeERPiPKcS1_, _ZNSt6locale5facet18_S_create_c_localeERP10__locale_tPKcS2_);
DEF_COMPAT(_ZNSt6locale5facet19_S_destroy_c_localeERPi, _ZNSt6locale5facet19_S_destroy_c_localeERP10__locale_t);
DEF_COMPAT(_ZNSt6locale5facet20_S_lc_ctype_c_localeEPiPKc, _ZNSt6locale5facet20_S_lc_ctype_c_localeEP10__locale_tPKc);
DEF_COMPAT(_ZSt14__convert_to_vIdEvPKcRT_RSt12_Ios_IostateRKPi, _ZSt14__convert_to_vIdEvPKcRT_RSt12_Ios_IostateRKP10__locale_t);
DEF_COMPAT(_ZSt14__convert_to_vIeEvPKcRT_RSt12_Ios_IostateRKPi, _ZSt14__convert_to_vIeEvPKcRT_RSt12_Ios_IostateRKP10__locale_t);
DEF_COMPAT(_ZSt14__convert_to_vIfEvPKcRT_RSt12_Ios_IostateRKPi, _ZSt14__convert_to_vIfEvPKcRT_RSt12_Ios_IostateRKP10__locale_t);
DEF_COMPAT(_ZSt16__convert_from_vRKPiPciPKcz, _ZSt16__convert_from_vRKP10__locale_tPciPKcz);
#define DEF_COMPAT_EXT(func, target) \
extern "C" __attribute__ ((weak)) void target (void) {} DEF_COMPAT(func, target)
DEF_COMPAT_EXT(_ZNSt10moneypunctIcLb0EE24_M_initialize_moneypunctEPiPKc, _ZNSt10moneypunctIcLb0EE24_M_initialize_moneypunctEP10__locale_tPKc);
DEF_COMPAT_EXT(_ZNSt10moneypunctIcLb0EEC1EPiPKcm, _ZNSt10moneypunctIcLb0EEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt10moneypunctIcLb0EEC2EPiPKcm, _ZNSt10moneypunctIcLb0EEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt10moneypunctIcLb1EE24_M_initialize_moneypunctEPiPKc, _ZNSt10moneypunctIcLb1EE24_M_initialize_moneypunctEP10__locale_tPKc);
DEF_COMPAT_EXT(_ZNSt10moneypunctIcLb1EEC1EPiPKcm, _ZNSt10moneypunctIcLb1EEC1EPSt18__moneypunct_cacheIcLb1EEm);
DEF_COMPAT_EXT(_ZNSt10moneypunctIcLb1EEC2EPiPKcm, _ZNSt10moneypunctIcLb1EEC2EPSt18__moneypunct_cacheIcLb1EEm);
DEF_COMPAT_EXT(_ZNSt10moneypunctIwLb0EE24_M_initialize_moneypunctEPiPKc, _ZNSt10moneypunctIwLb0EE24_M_initialize_moneypunctEP10__locale_tPKc);
DEF_COMPAT_EXT(_ZNSt10moneypunctIwLb0EEC1EPiPKcm, _ZNSt10moneypunctIwLb0EEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt10moneypunctIwLb0EEC2EPiPKcm, _ZNSt10moneypunctIwLb0EEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt10moneypunctIwLb1EE24_M_initialize_moneypunctEPiPKc, _ZNSt10moneypunctIwLb1EE24_M_initialize_moneypunctEP10__locale_tPKc);
DEF_COMPAT_EXT(_ZNSt10moneypunctIwLb1EEC1EPiPKcm, _ZNSt10moneypunctIwLb1EEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt10moneypunctIwLb1EEC2EPiPKcm, _ZNSt10moneypunctIwLb1EEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt11__timepunctIcE23_M_initialize_timepunctEPi, _ZNSt11__timepunctIcE23_M_initialize_timepunctEP10__locale_t);
DEF_COMPAT_EXT(_ZNSt11__timepunctIcEC1EPiPKcm, _ZNSt11__timepunctIcEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt11__timepunctIcEC2EPiPKcm, _ZNSt11__timepunctIcEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt11__timepunctIwE23_M_initialize_timepunctEPi, _ZNSt11__timepunctIwE23_M_initialize_timepunctEP10__locale_t);
DEF_COMPAT_EXT(_ZNSt11__timepunctIwEC1EPiPKcm, _ZNSt11__timepunctIwEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt11__timepunctIwEC2EPiPKcm, _ZNSt11__timepunctIwEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt5ctypeIcEC1EPiPKcbm, _ZNSt5ctypeIcEC1EP10__locale_tPKcbm);
DEF_COMPAT_EXT(_ZNSt5ctypeIcEC2EPiPKcbm, _ZNSt5ctypeIcEC2EP10__locale_tPKcbm);
DEF_COMPAT_EXT(_ZNSt5ctypeIwEC1EPim, _ZNSt5ctypeIwEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt5ctypeIwEC2EPim, _ZNSt5ctypeIwEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIcLb0EE24_M_initialize_moneypunctEPiPKc, _ZNSt7__cxx1110moneypunctIcLb0EE24_M_initialize_moneypunctEP10__locale_tPKc);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIcLb0EEC1EPiPKcm, _ZNSt7__cxx1110moneypunctIcLb0EEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIcLb0EEC2EPiPKcm, _ZNSt7__cxx1110moneypunctIcLb0EEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIcLb1EE24_M_initialize_moneypunctEPiPKc, _ZNSt7__cxx1110moneypunctIcLb1EE24_M_initialize_moneypunctEP10__locale_tPKc);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIcLb1EEC1EPiPKcm, _ZNSt7__cxx1110moneypunctIcLb1EEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIcLb1EEC2EPiPKcm, _ZNSt7__cxx1110moneypunctIcLb1EEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIwLb0EE24_M_initialize_moneypunctEPiPKc, _ZNSt7__cxx1110moneypunctIwLb0EE24_M_initialize_moneypunctEP10__locale_tPKc);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIwLb0EEC1EPiPKcm, _ZNSt7__cxx1110moneypunctIwLb0EEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIwLb0EEC2EPiPKcm, _ZNSt7__cxx1110moneypunctIwLb0EEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIwLb1EE24_M_initialize_moneypunctEPiPKc, _ZNSt7__cxx1110moneypunctIwLb1EE24_M_initialize_moneypunctEP10__locale_tPKc);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIwLb1EEC1EPiPKcm, _ZNSt7__cxx1110moneypunctIwLb1EEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx1110moneypunctIwLb1EEC2EPiPKcm, _ZNSt7__cxx1110moneypunctIwLb1EEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx117collateIcEC1EPim, _ZNSt7__cxx117collateIcEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7__cxx117collateIcEC2EPim, _ZNSt7__cxx117collateIcEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7__cxx117collateIwEC1EPim, _ZNSt7__cxx117collateIwEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7__cxx117collateIwEC2EPim, _ZNSt7__cxx117collateIwEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7__cxx118messagesIcEC1EPiPKcm, _ZNSt7__cxx118messagesIcEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx118messagesIcEC2EPiPKcm, _ZNSt7__cxx118messagesIcEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx118messagesIwEC1EPiPKcm, _ZNSt7__cxx118messagesIwEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx118messagesIwEC2EPiPKcm, _ZNSt7__cxx118messagesIwEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt7__cxx118numpunctIcE22_M_initialize_numpunctEPi, _ZNSt7__cxx118numpunctIcE22_M_initialize_numpunctEP10__locale_t);
DEF_COMPAT_EXT(_ZNSt7__cxx118numpunctIcEC1EPim, _ZNSt7__cxx118numpunctIcEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7__cxx118numpunctIcEC2EPim, _ZNSt7__cxx118numpunctIcEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7__cxx118numpunctIwE22_M_initialize_numpunctEPi, _ZNSt7__cxx118numpunctIwE22_M_initialize_numpunctEP10__locale_t);
DEF_COMPAT_EXT(_ZNSt7__cxx118numpunctIwEC1EPim, _ZNSt7__cxx118numpunctIwEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7__cxx118numpunctIwEC2EPim, _ZNSt7__cxx118numpunctIwEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7codecvtIcc10_mbstate_tEC1EPim, _ZNSt7codecvtIcc10_mbstate_tEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7codecvtIcc10_mbstate_tEC2EPim, _ZNSt7codecvtIcc10_mbstate_tEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7codecvtIwc10_mbstate_tEC1EPim, _ZNSt7codecvtIwc10_mbstate_tEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7codecvtIwc10_mbstate_tEC2EPim, _ZNSt7codecvtIwc10_mbstate_tEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7collateIcEC1EPim, _ZNSt7collateIcEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7collateIcEC2EPim, _ZNSt7collateIcEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7collateIwEC1EPim, _ZNSt7collateIwEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt7collateIwEC2EPim, _ZNSt7collateIwEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt8messagesIcEC1EPiPKcm, _ZNSt8messagesIcEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt8messagesIcEC2EPiPKcm, _ZNSt8messagesIcEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt8messagesIwEC1EPiPKcm, _ZNSt8messagesIwEC1EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt8messagesIwEC2EPiPKcm, _ZNSt8messagesIwEC2EP10__locale_tPKcm);
DEF_COMPAT_EXT(_ZNSt8numpunctIcE22_M_initialize_numpunctEPi, _ZNSt8numpunctIcE22_M_initialize_numpunctEP10__locale_t);
DEF_COMPAT_EXT(_ZNSt8numpunctIcEC1EPim, _ZNSt8numpunctIcEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt8numpunctIcEC2EPim, _ZNSt8numpunctIcEC2EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt8numpunctIwE22_M_initialize_numpunctEPi, _ZNSt8numpunctIwE22_M_initialize_numpunctEP10__locale_t);
DEF_COMPAT_EXT(_ZNSt8numpunctIwEC1EPim, _ZNSt8numpunctIwEC1EP10__locale_tm);
DEF_COMPAT_EXT(_ZNSt8numpunctIwEC2EPim, _ZNSt8numpunctIwEC2EP10__locale_tm);
