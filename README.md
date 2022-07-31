# README for gdb-solib-wine

This is a development fork of GDB to enhance GDB with an understanding
of WINE's dynamic linker. It is primarily intended for situations where
winedbg cannot be used (e.g. in rr traces).

# Usage

## Setup

In order for GDB to be able to find your shared libraries, you may
need to work around some case sensitivity issues. The easiest way
I've found to do this is to add some symlinks:

```
cd ~/.wine/dosdevices
ln -s z: Z:
ln -s c: C:
```

## In gdb

In the current version of this patchset, wine solib functionality
is guarded by the `WineLinux` osabi mode (whether that is the
correct design is something to be figured out).

Activate it by using:
```
set solib-search-path ~/.wine/dosdevices/
set osabi WineLinux
file wine64
sharedlibrary
```

Wine's PE files should now load automatically.

# Wine notes

In order to get complete backtraces, you may need more extensive
unwind information than are available in wine master. See
https://github.com/JuliaComputing/wine-staging for the JC patchset
where I'm working on adding this unwind info.

# Notes for use under rr

You will likely want to build a relatively new version of rr to
make sure you pick up https://github.com/rr-debugger/rr/commit/1a774d0b5946a19b291756092be699d5295c0dc1.
Otherwise rr will fail to load wine's PE libraries.

# Example session:

```
$ ~/rr-build/bin/rr replay -s 2000 -g 239500
GNU gdb (GDB) 12.0.50.20211215-git
Copyright (C) 2021 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-pc-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
10000: No such file or directory.
"/home/keno/.local/share/rr/wine64-21/mmap_clone_7008_wine64-preloader" is not a core dump: file format not recognized
Remote debugging using 127.0.0.1:2000
Reading symbols from /home/keno/.local/share/rr/wine64-21/mmap_clone_7008_wine64-preloader...
0x000000007000000e in syscall_untraced_record_only ()
(rr) file ~/wine-debug/loader/wine64
A program is being debugged already.
Are you sure you want to change the file? (y or n) y
Load new symbol table from "~/wine-debug/wine/wine64-build/loader/wine64"? (y or n) y
Reading symbols from ~/wine-debug/wine/wine64-build/loader/wine64...
Reading symbols from /home/keno/rr-build/bin/../lib/rr/librrpreload.so...
Reading symbols from /lib/x86_64-linux-gnu/libpthread.so.0...
(No debugging symbols found in /lib/x86_64-linux-gnu/libpthread.so.0)
Reading symbols from /lib/x86_64-linux-gnu/libdl.so.2...
(No debugging symbols found in /lib/x86_64-linux-gnu/libdl.so.2)
Reading symbols from /lib/x86_64-linux-gnu/libc.so.6...
(No debugging symbols found in /lib/x86_64-linux-gnu/libc.so.6)
Reading symbols from /lib64/ld-linux-x86-64.so.2...
(No debugging symbols found in /lib64/ld-linux-x86-64.so.2)
Reading symbols from /home/keno/wine-debug/wine/wine64-build/dlls/ntdll/ntdll.so...
Reading symbols from /usr/lib/x86_64-linux-gnu/libunwind.so.8...
(No debugging symbols found in /usr/lib/x86_64-linux-gnu/libunwind.so.8)
Reading symbols from /lib/x86_64-linux-gnu/liblzma.so.5...
(No debugging symbols found in /lib/x86_64-linux-gnu/liblzma.so.5)
Reading symbols from /home/keno/wine-debug/wine/wine64-build/libs/wine/libwine.so.1...
Reading symbols from /home/keno/wine-debug/wine/wine64-build/dlls/win32u/win32u.so...
Reading symbols from /lib/x86_64-linux-gnu/libm.so.6...
(No debugging symbols found in /lib/x86_64-linux-gnu/libm.so.6)
Reading symbols from /usr/lib/x86_64-linux-gnu/libfreetype.so.6...
(No debugging symbols found in /usr/lib/x86_64-linux-gnu/libfreetype.so.6)
Reading symbols from /usr/lib/x86_64-linux-gnu/libpng16.so.16...
(No debugging symbols found in /usr/lib/x86_64-linux-gnu/libpng16.so.16)
Reading symbols from /lib/x86_64-linux-gnu/libz.so.1...
(No debugging symbols found in /lib/x86_64-linux-gnu/libz.so.1)
Reading symbols from /usr/lib/x86_64-linux-gnu/libfontconfig.so.1...
(No debugging symbols found in /usr/lib/x86_64-linux-gnu/libfontconfig.so.1)
Reading symbols from /lib/x86_64-linux-gnu/libexpat.so.1...
(No debugging symbols found in /lib/x86_64-linux-gnu/libexpat.so.1)
Reading symbols from /lib/x86_64-linux-gnu/libuuid.so.1...
(No debugging symbols found in /lib/x86_64-linux-gnu/libuuid.so.1)
(rr) set osabi WineLinux
(rr) sharedlibrary
Reading symbols from /home/keno/.wine/drive_c/windows/system32/conhost.exe...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/ntdll.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/kernel32.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/kernelbase.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/advapi32.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/msvcrt.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/sechost.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/ucrtbase.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/gdi32.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/win32u.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/user32.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/setupapi.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/rpcrt4.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/version.dll...
Reading symbols from /home/keno/.wine/drive_c/windows/system32/imm32.dll...
(rr) bt
#0  0x000000007000000e in syscall_untraced_record_only ()
#1  0x00007f6ff822d725 in _raw_syscall () at /home/keno/rr/src/preload/raw_syscall.S:120
#2  0x00007f6ff822953a in untraced_syscall_base (syscallno=syscallno@entry=0, a0=a0@entry=4,
    a1=a1@entry=140118877687918, a2=a2@entry=64, a3=a3@entry=0, a4=a4@entry=0, a5=0,
    syscall_instruction=<optimized out>) at /home/keno/rr/src/preload/syscallbuf.c:376
#3  0x00007f6ff822cf2e in sys_read (call=0x681fffa0) at /home/keno/rr/src/preload/syscallbuf.c:2396
#4  syscall_hook_internal (call=0x681fffa0) at /home/keno/rr/src/preload/syscallbuf.c:3347
#5  syscall_hook (call=0x681fffa0) at /home/keno/rr/src/preload/syscallbuf.c:3454
#6  0x00007f6ff8229340 in _syscall_hook_trampoline () at /home/keno/rr/src/preload/syscall_hook.S:313
#7  0x00007f6ff822939f in __morestack () at /home/keno/rr/src/preload/syscall_hook.S:458
#8  0x00007f6ff82293bb in _syscall_hook_trampoline_48_3d_00_f0_ff_ff ()
    at /home/keno/rr/src/preload/syscall_hook.S:477
#9  0x00007f6ff81f3338 in read () from /lib/x86_64-linux-gnu/libpthread.so.0
#10 0x00007f6ff7e73656 in read_reply_data (buffer=buffer@entry=0x6afc80, size=size@entry=64)
    at ../include/winnt.h:2161
#11 0x00007f6ff7e73f82 in wait_reply (req=0x6afc80) at ../dlls/ntdll/unix/server.c:277
#12 server_call_unlocked (req_ptr=req_ptr@entry=0x6afc80) at ../dlls/ntdll/unix/server.c:293
#13 0x00007f6ff7e74170 in wine_server_call (req_ptr=0x6afc80) at ../dlls/ntdll/unix/server.c:308
#14 0x00007f6ff7e76cc5 in __wine_syscall_dispatcher ()
   from /home/keno/wine-debug/wine/wine64-build/dlls/ntdll/ntdll.so
#15 0x000000017000e2d4 in wine_server_call () from /home/keno/.wine/drive_c/windows/system32/ntdll.dll
#16 0x0000000000406ef6 in process_console_ioctls (console=0x416000 <console>)
    at ../programs/conhost/conhost.c:2614
#17 main_loop (console=0x416000 <console>, signal=<optimized out>) at ../programs/conhost/conhost.c:2710
#18 wmain (argc=argc@entry=8, argv=argv@entry=0x29b40) at ../programs/conhost/conhost.c:2825
#19 0x0000000000410565 in wmainCRTStartup () at ../dlls/msvcrt/crt_wmain.c:58
#20 0x000000007b62d479 in BaseThreadInitThunk (unknown=<optimized out>, entry=<optimized out>, arg=<optimized out>) at ../dlls/kernel32/thread.c:61
#21 0x000000017005eb93 in RtlUserThreadStart (entry=0x410500 <wmainCRTStartup>, arg=0x3f0000)
    at ../dlls/ntdll/thread.c:241
#22 0x0000000000000000 in ?? ()
```

# TODO:

- [ ] Add WoW64 support?
- [ ] Breakpoint on library reload?
- [ ] Some sort of better path translation?
- [ ] Wine specific debug commands?
- [ ] Teach GDB to do case insensitive lookups for windows libraries
- [ ] Look at Rtl tables for dynamic unwind info
- [ ] Teach GDB about WCHAR?
