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

/** @addtogroup fio_stat File I/O file modes and types
    @{
*/

/** @name I/O Access Flags
 *  The flags used when accessing a file.
 * @{
 */
#define IO_RDONLY	0x0001
#define IO_WRONLY	0x0002
#define IO_RDWR		0x0003
#define IO_NONBLOCK	0x0010
#define IO_APPEND	0x0100
#define IO_CREAT	0x0200
#define IO_TRUNC	0x0400
#define	IO_EXCL		0x0800
/** @warning Internal use only */
#define IO_DIROPEN	0x0008
/** @} */

/** @name I/O Seek Positions
 *  The positions used when seeking a file.
 * @{
 */
#define IO_SEEK_SET	0
#define IO_SEEK_CUR	1
#define IO_SEEK_END	2
/** @} */

/** End of addtogroup fio_stat
 *  @}
 */

#endif /* SYS_IO_FCNTL_H */
