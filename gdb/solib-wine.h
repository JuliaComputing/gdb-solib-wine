/* Handle shared libraries for GDB, the GNU Debugger.

   Copyright (C) 2000-2021 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef SOLIB_WINE_H
#define SOLIB_WINE_H

#include "solist.h"

struct objfile;
struct target_so_ops;

extern struct target_so_ops wine_so_ops;
extern void set_solib_wine(struct gdbarch *gdbarch, bool (*get_current_tib_addr)(CORE_ADDR*));

#endif
