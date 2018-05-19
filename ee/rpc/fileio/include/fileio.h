/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE FILEIO RPC client
 */

/** 
 * @addtogroup libfileio FILEIO: EE FILEIO RPC Client
 * @{
 *
 * @details This is the EE RPC client protocol for FILEIO.
 *
 * @bug FILEIO in the bios is unstable.
 *
 * @bug @ref fioDread "fioDread()" and @ref fioGetstat "fioGetstat()"
 *           do not suspend interrupts to perform DMA transfers on the IOP.
 *
 * @bug @ref fioRemove "fioRemove()" falls through to fioMkdir() upon
 *           completion.
 *
 * @bug A @ref sbvpatches "patch" is available to fix these issues.
 *
 * @note This rpc client protocol does not support newer versions of FILEIO.
*/

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <sys/io_fcntl.h>
#include <sys/io_stat.h>

#define FIO_PATH_MAX	256

/** @name fioSetBlockMode
 *  Values for setting blocking mode. 
 * @{
 */
#define FIO_WAIT	0
#define FIO_NOWAIT	1
/**@}*/

/** @name fioSync
 *  Values for the return value of fioSync(). 
 * @{
 */
#define FIO_COMPLETE	1
#define FIO_INCOMPLETE	0
/**@}*/

#ifdef __cplusplus
extern "C" {
#endif

int fioInit(void);
void fioExit(void);
int fioOpen(const char *fname, int mode);
int fioClose( int fd);
int fioRead( int fd, void *buff, int buff_size);
int fioWrite( int fd, const void *buff, int buff_size);
int fioLseek( int fd, int offset, int whence);
int fioMkdir(const char* dirname);
int fioPutc(int fd,int c);
int fioGetc(int fd);
int fioGets(int fd, char* buff, int n);
void fioSetBlockMode(int blocking);
int fioSync(int mode, int *retVal);
int fioIoctl(int fd, int request, void *data);
int fioDopen(const char *name);
int fioDclose(int fd);
int fioDread(int fd, io_dirent_t *buf);
int fioDreadMC(int fd, fio_dirent_t *buf);
int fioGetstat(const char *name, io_stat_t *buf);
int fioChstat(const char *name, io_stat_t *buf, unsigned int cbit);
int fioRemove(const char *name);
int fioFormat(const char *name);
int fioRmdir(const char* dirname);

#ifdef __cplusplus
}
#endif

#endif /* __FILEIO_H__ */

/** End of addtogroup libfileio
 *  @}
 */
