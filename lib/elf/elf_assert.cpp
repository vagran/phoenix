/*
 assert.c - assert function for libelf.
 Copyright (C) 1999 Michael Riepe

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "private.h"

#ifdef EFI_APP
extern "C" i64 DbgAssert (const char *file, i64 lineno, const char *string);
#endif /* EFI_APP */

void __elf_assert(const char *file, unsigned line, const char *cond)
{
    //FIXME Standardized assert implementation required
    //__assert(file, line, cond);

#ifdef EFI_APP
    DbgAssert(file, line, cond);
#endif /* EFI_APP */
}
