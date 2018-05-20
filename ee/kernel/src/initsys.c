/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * initsys - basic initialization/termination functions for libkernel.
 */

#include "kernel.h"

#if defined(F__InitSys) || defined(DOXYGEN)
void _InitSys(void)
{
#ifndef KERNEL_NO_PATCHES
	InitAlarm();
	InitThread();
	InitExecPS2();
	InitTLBFunctions();
#endif
}
#endif

#if defined(F_TerminateLibrary) || defined(DOXYGEN)
void TerminateLibrary(void)
{
#ifndef KERNEL_NO_PATCHES
	InitTLB();
#endif
}
#endif

#if defined(F_ps2_sbrk) || defined(DOXYGEN)
extern unsigned int end;

/* Not sure if this needs to be here. */
void *ps2_sbrk(ptrdiff_t incr)
{
   static unsigned char *heap_end = (unsigned char*)&end;
   unsigned char *prev_heap_end;

   prev_heap_end = heap_end;
   heap_end += incr;

   if (((ptrdiff_t)heap_end) <= ((ptrdiff_t)EndOfHeap()))
     return heap_end;

   heap_end = prev_heap_end;

   return ((void*)-1);
}
#endif
