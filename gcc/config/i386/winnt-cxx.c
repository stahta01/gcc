/* Target support for C++ classes on Windows.
   Contributed by Danny Smith (dannysmith@users.sourceforge.net)
   Copyright (C) 2005
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "output.h"
#include "tree.h"
#include "flags.h"
#include "tm_p.h"
#include "cp/cp-tree.h"
#include "toplev.h"

int
i386_pe_class_dllimport_p (tree decl)
{
      /* We ignore the dllimport attribute for inline member functions.
	 This differs from MSVC behavior which treats it like GNUC
	 'extern inline' extension.  Also ignore for temnlate instantiations
         with linkonce semantics and artificial methods.  */
       if (TREE_CODE (decl) ==  FUNCTION_DECL
	   && (DECL_INLINE (decl)
	       || DECL_TEMPLATE_INSTANTIATION (decl)
	       || DECL_ARTIFICIAL (decl)))
	  return 0;

      /* Don't mark defined functions as dllimport.  If the definition
	 itself was marked with dllimport, than ix86_handle_dll_attribute
	 reports an error. This handles the case when the definition
	 overrides an earlier declaration.  */
      else if (TREE_CODE (decl) ==  FUNCTION_DECL
		&& (DECL_INITIAL (decl)))
	{
	  /* Avoid multiple warnings on clones.  */
          if (!DECL_CLONED_FUNCTION_P (decl)
	      || DECL_NAME (decl) == complete_ctor_identifier
	      || DECL_NAME (decl) == complete_dtor_identifier)
	    warning ("%Jnon-inline function '%D' is defined after prior declaration "
		     "as dllimport: attribute ignored", decl, decl);

	  return 0;
	}

      /*  Don't allow definitions of static data members in dllimport class,
	  If vtable data is marked as DECL_EXTERNAL, import it; otherwise just
          ignore the attribute.  */
      else if (TREE_CODE (decl) == VAR_DECL
	       && TREE_STATIC (decl) && TREE_PUBLIC (decl)
	       && !DECL_EXTERNAL (decl))
	{
	  if (!DECL_VIRTUAL_P (decl))
            error ("%Jdefinition of static data member '%D' of "
		   "dllimport'd class", decl, decl);
	  return 0;
	}

      /* Since we can't treat a pointer to a dllimport'd symbol as a
	 constant address, we turn off the attribute on C++ virtual
	 methods to allow creation of vtables using thunks.  */
      else if (TREE_CODE (TREE_TYPE (decl)) == METHOD_TYPE
	       && DECL_VIRTUAL_P (decl))
        return 0;

      return 1;
}
