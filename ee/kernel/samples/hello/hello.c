/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Hello sample
*/

#include <tamtypes.h>
#include <sifcmd.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileXio.h>

#include <stdio.h>
#include <sio.h>

/* Return status of IOP setup. */
int iop_ret = 0;

/* This function runs before main to setup the IOP prior to running
   any global C or C++ constructors. */
void iop_start(void)
{
  SifInitRpc(0);

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

int main()
{
  if (iop_ret < 0)
    return;
/*
  init_scr();
  scr_printf("Hello, world!\n"); // hello world in the screen
*/

  printf("Hello, world!\n");

  sio_init(115200, 0, 0, 0, 0);
  sio_printf("Hello from EE SIO!\n");

  /* Return to the bootloader or PS2 browser. */
  return 0;
}
