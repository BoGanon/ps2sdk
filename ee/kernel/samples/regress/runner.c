/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Testsuite runner
*/

#include <stdio.h>
#include "testsuite.h"

#ifdef _EE
#include <kernel.h>
#include <sifrpc.h>
#include <sbv_patches.h>
#include <loadfile.h>
#include <fileXio_rpc.h>
#endif

extern int libc_add_tests(test_suite *p);

#ifdef _EE
int iop_ret = 0;

void iop_start(void)
{
  SifInitRpc(0);

  sbv_patch_fileio();

  SifLoadStartModule("rom0:FILEIO",0,NULL,&iop_ret);
  if (iop_ret < 0)
    return;
  SifLoadStartModule("host:iomanX.irx",0,NULL,&iop_ret);
  if (iop_ret < 0)
    return;
  SifLoadStartModule("host:fileXio.irx",0,NULL,&iop_ret);
  if (iop_ret < 0)
    return;

  fileXioInit();
}
#endif

int main(int argc, char *argv[])
{
  test_suite suite;

#ifdef _EE
  /* Check the IOP setup status. */
  if (iop_ret < 0)
	return 1;
#endif

  /* initialize test suite */
  init_testsuite(&suite);

  /* add all tests to this suite */
  libc_add_tests(&suite);

  /* run all tests */
  run_testsuite(&suite);

  return 0;
}
