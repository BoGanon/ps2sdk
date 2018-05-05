/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * File I/O access control flags
 */

#ifndef SYS_IO_FCNTL_H
#define SYS_IO_FCNTL_H

/** @addtogroup fileio
    @{
*/

#define IO_RDONLY	0x0001
#define IO_WRONLY	0x0002
#define IO_RDWR		0x0003
#define IO_NONBLOCK	0x0010
#define IO_APPEND	0x0100
#define IO_CREAT	0x0200
#define IO_TRUNC	0x0400
#define	IO_EXCL		0x0800

#define IO_SEEK_SET	0
#define IO_SEEK_CUR	1
#define IO_SEEK_END	2

/** @warning Internal pfs use only */
#define IO_DIROPEN	000010

/** End of addtogroup fileio
 *  @}
 */

#endif /* SYS_IO_FCNTL_H */
