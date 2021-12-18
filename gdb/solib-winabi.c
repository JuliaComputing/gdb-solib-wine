/* Definitions for targets which report shared library events.

   Copyright (C) 2007-2021 Free Software Foundation, Inc.

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

// TODO: Figure out the right code organization for this.
// This file has code that deals with the windows process ABI and is shared
// between the windows and wine solib code.

static bool winabi_target_get_peb(CORE_ADDR tlb, bool win64, enum bfd_endian byte_order, CORE_ADDR *peb_addr)
{
  int ptr_bytes;
  int peb_offset;  /* Offset of process_environment_block in TIB.  */
  if (!win64)
  {
    ptr_bytes = 4;
    peb_offset = 48;
  }
  else
  {
    ptr_bytes = 8;
    peb_offset = 96;
  }
  gdb_byte buf[8];
  if (!target_read_memory (tlb + peb_offset, buf, ptr_bytes))
  {
    *peb_addr = extract_unsigned_integer (buf, ptr_bytes, byte_order);
    return true;
  }
  return false;
}

static int offset_PEB_LDR_DATA = 3; // sizeof(target pointer)
static bool winabi_ldr_in_load_order_list(CORE_ADDR peb_addr, bool win64, enum bfd_endian byte_order, CORE_ADDR *list_head)
{
    gdb_byte buf[8];
    size_t ptr_bytes = win64 ? 8 : 4;
    if (!target_read_memory(peb_addr + offset_PEB_LDR_DATA * ptr_bytes,
                            buf, ptr_bytes))
    {
        CORE_ADDR peb_ldr_data_addr = extract_unsigned_integer (buf, ptr_bytes, byte_order);
        if (!target_read_memory(peb_ldr_data_addr + 8 + ptr_bytes,
                                buf, ptr_bytes))
        {
            *list_head = extract_unsigned_integer (buf, ptr_bytes, byte_order);
            return true;
        }
    }
    return false;
}
