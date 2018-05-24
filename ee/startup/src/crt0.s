# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2001-2007 ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Standard startup file.

   # Check weak symbols
   # jump and link if defined
   # Usage:
   #            ckwk $8,sym,1f
   #        1:
   .macro ckwk reg,sym,lbl
   la   \reg, \sym
   beqz   \reg, \lbl
   nop
   jalr   \reg
   nop
   .endm

   # IOP init
   .weak  iop_start
   .type  iop_start, @function

   # Provided by libc
   .type   atexit, @function
   .type   exit, @function

   # New C++ ctor and dtor arrays
   # Compatible with _init and _fini
   # gcc-3.2.3 doesn't support constructor priorities.
   .type   __libc_init_array, @function
   .type   __libc_fini_array, @function

   .set   noat
   .set	  noreorder

   .text
   .align   3

   .globl  _start
   .ent    _start
_start:

zerobss:
   # clear bss area
   la   $2, _fbss
   la   $3, _end

1:
   sltu   $1, $2, $3
   beq   $1, $0, 2f
   nop
   sq   $0, ($2)
   addiu   $2, $2, 16
   j   1b
   nop
2:

setup_thread:
   # setup current thread for creating threads
   la   $4, _gp
   la   $5, _stack
   la   $6, _stack_size
   la   $7, _args
   la   $8, _root
   move   $gp, $4
   addiu   $3, $0, 60
   syscall         # SetupThread(_gp, _stack, _stack_size, _args, _root)

setup_mem:
   # setup stack and make room for storing arguments
   move   $sp, $2
   addiu   $sp, $sp,-96

   # initialize heap
   la   $4, _end
   la   $5, _heap_size
   addiu   $3, $0, 61
   syscall         # SetupHeap(_end, _heap_size)

   # writeback data cache
   jal FlushCache  # FlushCache(0)
   move   $4, $0

setup_sys:
   # Initialize the kernel first (Apply necessary patches).
   jal _InitSys
   nop
   
   # Enable interrupts
   ei

   # Initialize the IOP (user-defined)
   # If compiling newlib with FileXio support and using C++ this MUST be defined
   # by the user to load iomanX.irx and fileXio.irx to avoid causing a deadlock
   # when the fileXio RPC client is used by the standard library for stdout
   # from C++ constructors before main.
   ckwk   $8,iop_start,1f
1:

setup_libc:
   # add destructors using atexit()
   la   $4, __libc_fini_array
   jal  atexit
   nop
   
   # call constructors
   jal   __libc_init_array
   nop

run_main:
   la   $16, _args
   lw   $4, ($16)
   jal   main       # main(argc, argv)
   addiu $5, $16, 4

   # fall through to libc exit
   jal   exit       # exit(retval) (noreturn)
   move   $4, $2

   .end   _start

   .align   3
   
   # _exit(retval) (noreturn) called by libc
   .globl   _exit
   .ent   _exit
   .text

_exit:
   j   Exit           # Exit(retval) (noreturn)
   nop

   .end   _exit

   .ent   _root

_root:
   addiu   $3, $0, 35
   syscall          # ExitThread() (noreturn)
   .end   _root

   .bss
   .align   6
_args:
   .space   4+16*4+256   # argc, 16 arguments, 256 bytes payload
