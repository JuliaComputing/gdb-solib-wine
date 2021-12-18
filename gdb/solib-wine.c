/* Handle wine shared libraries for GDB, the GNU Debugger.

   Copyright (C) 1990-2021 Free Software Foundation, Inc.

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

#include "defs.h"

#include "symtab.h"
#include "charset.h"
#include "bfd.h"
#include "symfile.h"
#include "objfiles.h"
#include "gdbcore.h"
#include "target.h"
#include "inferior.h"
#include "infrun.h"
#include "regcache.h"
#include "gdbthread.h"
#include "observable.h"

#include "solist.h"
#include "solib.h"
#include "solib-wine.h"

#include "solib-winabi.c"

struct target_so_ops wine_so_ops;

/* Per-architecture data key.  */
static struct gdbarch_data *solib_wine_data;

struct solib_wine_ops
{
  const struct target_so_ops *host_so_ops;
  bool (*get_current_tib_addr)(CORE_ADDR*);
};

/* Per pspace wine specific data.  */
struct wine_info
{
  struct so_list *host_so_list;
};

/* Per-program-space data key.  */
static const struct program_space_key<wine_info> solib_wine_pspace_data;

/* Get the wine data for program space PSPACE.  If none is found yet, add it now.
   This function always returns a valid object.  */

static struct wine_info *
get_wine_info (program_space *pspace)
{
  struct wine_info *info = solib_wine_pspace_data.get (pspace);

  if (info == NULL)
    info = solib_wine_pspace_data.emplace (pspace);

  return info;
}

/* Return a default for the architecture-specific operations.  */
static void *
solib_wine_init (struct obstack *obstack)
{
  struct solib_wine_ops *ops;

  ops = OBSTACK_ZALLOC (obstack, struct solib_wine_ops);
  ops->host_so_ops = NULL;
  return ops;
}

static void
wine_relocate_section_addresses (struct so_list *so,
				 struct target_section *sec)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  if (so->so_ops == &wine_so_ops)
  {
    sec->addr = sec->addr + (uintptr_t)so->lm_info;
    sec->endaddr = sec->endaddr + (uintptr_t)so->lm_info;
  } else {
    return ops->host_so_ops->relocate_section_addresses (so, sec);
  }
}

static void
wine_free_so(struct so_list *so)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->free_so (so);
}

static void
wine_clear_so(struct so_list *so)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->clear_so (so);
}

static void
wine_clear_solib (void)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->clear_solib ();
}

static void
wine_solib_create_inferior_hook (int from_tty)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->solib_create_inferior_hook (from_tty);
}

/*
    MIT License

    Copyright (c) Microsoft Corporation.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE
  */

template <typename ptr_t> struct LIST_ENTRY {
   ptr_t *Flink;
   ptr_t *Blink;
};

template <typename ptr_t> struct UNICODE_STRING
{
  uint16_t Length;
  uint16_t MaximumLength;
  ptr_t Buffer;
};

template <typename ptr_t> struct LDR_DATA_TABLE_ENTRY {
  LIST_ENTRY<ptr_t> InLoadOrderLinks;
  LIST_ENTRY<ptr_t> InMemoryOrderLinks;
  LIST_ENTRY<ptr_t> InInitializationOrderLinks;
  ptr_t DllBase;
  ptr_t EntryPoint;
  ptr_t SizeOfImage;
  UNICODE_STRING<ptr_t> FullDllName;
};

static void wine_read_so_list(CORE_ADDR list_head, bool iswin64, enum bfd_endian byte_order, struct so_list ***link_ptr_ptr)
{
  CORE_ADDR next_entry = 0;
  for (CORE_ADDR entry = list_head; next_entry != list_head; entry = next_entry)
  {
    so_list_up newobj (XCNEW (struct so_list));
    if (iswin64)
    {
      LDR_DATA_TABLE_ENTRY<uint64_t> ldr;
      if (target_read_memory (entry, (gdb_byte*)&ldr, sizeof(ldr)) != 0) {
        warning (_("Error reading shared library list entry at %s"),
          paddress (target_gdbarch (), entry));
        break;
      }

      std::vector<gdb_byte> full_name(ldr.FullDllName.Length);
      if (target_read_memory (ldr.FullDllName.Buffer, full_name.data(), full_name.size()) != 0) {
        warning (_("Error reading shared library full name at %s"),
          paddress (target_gdbarch (), ldr.FullDllName.Buffer));
        break;
      }

      auto_obstack converted_name;
      convert_between_encodings ("UTF-16", host_charset(),
              full_name.data(),
              full_name.size(),sizeof(uint16_t),&converted_name,
              translit_none);
      obstack_1grow (&converted_name, '\0');
      strncpy (newobj->so_original_name, (char*)obstack_base (&converted_name), SO_NAME_MAX_PATH_SIZE - 1);

      strncpy (newobj->so_name, (char*)obstack_base (&converted_name), SO_NAME_MAX_PATH_SIZE - 1);
      // TODO: Path translation, for now just rewrite path separators
      char *path = newobj->so_name;
      while (*path) {
        if (*path == '\\')
          *path = '/';
        path++;
      }

      next_entry = extract_unsigned_integer((gdb_byte*)&ldr.InLoadOrderLinks.Flink, 8, byte_order);
    }
    else
    {
      LDR_DATA_TABLE_ENTRY<uint32_t> ldr;
      if (target_read_memory (entry, (gdb_byte*)&ldr, sizeof(ldr)) != 0)
        warning (_("Error reading shared library list entry at %s"),
          paddress (target_gdbarch (), entry));
      error("TODO");
    }
    newobj->next = 0;
    newobj->so_ops = &wine_so_ops;
    /* Don't free it now.  */
    **link_ptr_ptr = newobj.release ();
    *link_ptr_ptr = &(**link_ptr_ptr)->next;
  }
}

static struct so_list *wine_current_sos (void)
{
  wine_info *info = get_wine_info (current_program_space);
  (void)info;

  struct gdbarch *gdbarch = target_gdbarch ();
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  // Load host sos
  struct so_list *head = NULL;
  struct so_list **link_ptr = &head;

  bool win64 = gdbarch_ptr_bit (gdbarch) == 64;

  // Load wine sos
  CORE_ADDR tib;
  if (!ops->get_current_tib_addr (&tib)) {
    warning (_("Error reading TIB"));
    return head;
  }

  CORE_ADDR peb;
  if (!winabi_target_get_peb(tib, win64, byte_order, &peb)) {
    warning (_("Error reading PEB addr from TIB at address %s"), paddress (target_gdbarch (), tib));
    return head;
  }

  CORE_ADDR lm;
  if (!winabi_ldr_in_load_order_list(peb, win64, byte_order, &lm)) {
    warning (_("Error reading shared library list from PEB at address %s"), paddress (target_gdbarch (), peb));
    return head;
  }

  wine_read_so_list (lm, win64, byte_order, &link_ptr);

  *link_ptr = ops->host_so_ops->current_sos ();

  return head;
}

static int
wine_in_dynsym_resolve_code (CORE_ADDR pc)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->in_dynsym_resolve_code (pc);
}

static int
wine_same (struct so_list *gdb, struct so_list *inferior)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->same (gdb, inferior);
}

static int
wine_keep_data_in_core (CORE_ADDR vaddr, unsigned long size)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->keep_data_in_core (vaddr, size);
}

static void
wine_update_breakpoints ()
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->update_breakpoints ();
}

static void
wine_handle_event (void)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->handle_event ();
}

void set_solib_wine (struct gdbarch *gdbarch, bool (*get_current_tib_addr)(CORE_ADDR*)) {
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);
  ops->host_so_ops = solib_ops(gdbarch);
  ops->get_current_tib_addr = get_current_tib_addr;
  set_solib_ops (gdbarch, &wine_so_ops);
}

static int
wine_open_symbol_file_object (int from_tty)
{
  struct gdbarch *gdbarch = target_gdbarch ();
  struct solib_wine_ops *ops
    = (struct solib_wine_ops *) gdbarch_data (gdbarch, solib_wine_data);

  return ops->host_so_ops->open_symbol_file_object (from_tty);
}

void _initialize_wine_solib ();
void
_initialize_wine_solib ()
{
  solib_wine_data = gdbarch_data_register_pre_init (solib_wine_init);

  wine_so_ops.relocate_section_addresses = wine_relocate_section_addresses;
  wine_so_ops.free_so = wine_free_so;
  wine_so_ops.clear_so = wine_clear_so;
  wine_so_ops.clear_solib = wine_clear_solib;
  wine_so_ops.solib_create_inferior_hook = wine_solib_create_inferior_hook;
  wine_so_ops.current_sos = wine_current_sos;
  wine_so_ops.open_symbol_file_object = wine_open_symbol_file_object;
  wine_so_ops.in_dynsym_resolve_code = wine_in_dynsym_resolve_code;
  wine_so_ops.bfd_open = solib_bfd_open;
  wine_so_ops.same = wine_same;
  wine_so_ops.keep_data_in_core = wine_keep_data_in_core;
  wine_so_ops.update_breakpoints = wine_update_breakpoints;
  wine_so_ops.handle_event = wine_handle_event;

/*
  gdb::observers::free_objfile.attach (wine_free_objfile_observer,
				       "solib-wine");
*/
}
