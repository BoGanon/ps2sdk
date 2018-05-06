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
 * EE FILE IO handling
 */

#include <tamtypes.h>
#include <ps2lib_err.h>
#include <kernel.h>
#include <sifrpc.h>
#include <fileio.h>
#include "kernel/string.h"

#define D(fmt, args...) printf("(%s:%s:%i):" # fmt, __FILE__, __FUNCTION__, __LINE__, ## args)

enum _fio_functions {
	FIO_F_OPEN = 0,
	FIO_F_CLOSE,
	FIO_F_READ,
	FIO_F_WRITE,
	FIO_F_LSEEK,
	FIO_F_IOCTL,
	FIO_F_REMOVE,
	FIO_F_MKDIR,
	FIO_F_RMDIR,
	FIO_F_DOPEN,
	FIO_F_DCLOSE,
	FIO_F_DREAD,
	FIO_F_GETSTAT,
	FIO_F_CHSTAT,
	FIO_F_FORMAT,
	FIO_F_ADDDRV,
	FIO_F_DELDRV
};

/** Shared between _fio_read_intr and fio_read.  The updated modules shipped
   with licensed games changed the size of the buffers from 16 to 64.  */
struct _fio_read_data {
	u32	size1;
	u32	size2;
	void	*dest1;
	void	*dest2;
	u8	buf1[16];
	u8	buf2[16];
};

extern int _iop_reboot_count;
extern SifRpcClientData_t _fio_cd;
extern int _fio_init;
extern int _fio_block_mode;
extern int _fio_completion_sema;
extern int _fio_recv_data[512];
extern int _fio_intr_data[32];

void _fio_read_intr(struct _fio_read_data *);
void _fio_intr();

#if defined(F___fio_internals) || defined(DOXYGEN)
SifRpcClientData_t _fio_cd;
int _fio_recv_data[512] __attribute__((aligned(64)));
int _fio_intr_data[32] __attribute__((aligned(64)));
int _fio_init = 0;
int _fio_block_mode;
int _fio_completion_sema = -1;
#endif

#if defined(F_fio_init) || defined(DOXYGEN)
int fioInit(void)
{
	int res;
	ee_sema_t sema;
	static int _rb_count = 0;

	if(_rb_count != _iop_reboot_count)
	{
		_rb_count = _iop_reboot_count;

		fioExit();
	}

        if (_fio_init)
		return 0;

	SifInitRpc(0);

	while (((res = SifBindRpc(&_fio_cd, 0x80000001, 0)) >= 0) &&
			(_fio_cd.server == NULL))
		nopdelay();

	if (res < 0)
		return res;

	sema.init_count = 1;
	sema.max_count = 1;
	sema.option = 0;
	_fio_completion_sema = CreateSema(&sema);
	if (_fio_completion_sema < 0)
		return -E_LIB_SEMA_CREATE;

	_fio_init = 1;
	_fio_block_mode = FIO_WAIT;

	return 0;
}
#endif

#if defined(F__fio_intr) || defined(DOXYGEN)
void _fio_intr(void)
{
	iSignalSema(_fio_completion_sema);
}
#endif

#if defined(F_fio_sync) || defined(DOXYGEN)
int fioSync(int mode, int *retVal)
{
	int res;

	if ((res = fioInit()) < 0)
		return res;

	if(_fio_block_mode != FIO_NOWAIT) return -E_LIB_UNSUPPORTED;

	switch(mode)
	{
		case FIO_WAIT:

			WaitSema(_fio_completion_sema);
			SignalSema(_fio_completion_sema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&_fio_recv_data[0]);

			return FIO_COMPLETE;

		case FIO_NOWAIT:

			if(PollSema(_fio_completion_sema) < 0)
				return FIO_INCOMPLETE;

			SignalSema(_fio_completion_sema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&_fio_recv_data[0]);

			return FIO_COMPLETE;

		default:
			return -E_LIB_UNSUPPORTED;
	}
}
#endif

#if defined(F_fio_setblockmode) || defined(DOXYGEN)
void fioSetBlockMode(int blocking)
{
	_fio_block_mode = blocking;
}

#endif

#if defined(F_fio_exit) || defined(DOXYGEN)
void fioExit(void)
{
	if(_fio_init)
	{
		_fio_init = 0;
		memset(&_fio_cd, 0, sizeof _fio_cd);
		if (_fio_completion_sema >= 0)
		{
			DeleteSema(_fio_completion_sema);
        	}
	}
}
#endif

#if defined(F_fio_open) || defined(DOXYGEN)
struct _fio_open_arg {
	int	mode;
	char 	name[FIO_PATH_MAX];
} ALIGNED(16);

int fioOpen(const char *name, int mode)
{
	struct _fio_open_arg arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.mode = mode;
	strncpy(arg.name, name, FIO_PATH_MAX - 1);
	arg.name[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_OPEN, _fio_block_mode, &arg, sizeof arg,
					_fio_recv_data, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=(_fio_block_mode == FIO_NOWAIT)?0:_fio_recv_data[0];
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}


	return result;
}
#endif

#if defined(F_fio_close) || defined(DOXYGEN)
int fioClose(int fd)
{
	union { int fd; int result; } arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.fd = fd;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_CLOSE, 0, &arg, 4, &arg, 4,
					(void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F__fio_read_intr) || defined(DOXYGEN)
void _fio_read_intr(struct _fio_read_data *data)
{
	struct _fio_read_data *uncached = UNCACHED_SEG(data);

	if (uncached->size1 && uncached->dest1) {
		memcpy(uncached->dest1, uncached->buf1, uncached->size1);
	}

	if (uncached->size2 && uncached->dest2) {
		memcpy(uncached->dest2, uncached->buf2, uncached->size2);
	}

	iSignalSema(_fio_completion_sema);
}
#endif

#if defined(F_fio_read) || defined(DOXYGEN)
struct _fio_read_arg {
	int	fd;
	void	*ptr;
	int	size;
	struct _fio_read_data *read_data;
} ALIGNED(16);

int fioRead(int fd, void *ptr, int size)
{
	struct _fio_read_arg arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.fd      = fd;
	arg.ptr       = ptr;
	arg.size      = size;
	arg.read_data = (struct _fio_read_data *)_fio_intr_data;

	if (!IS_UNCACHED_SEG(ptr))
		SifWriteBackDCache(ptr, size);

	if ((res = SifCallRpc(&_fio_cd, FIO_F_READ, _fio_block_mode, &arg, sizeof arg,
					_fio_recv_data, 4, (void *)_fio_read_intr, _fio_intr_data)) >= 0)
	{
		result=(_fio_block_mode == FIO_NOWAIT)?0:_fio_recv_data[0];
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_write) || defined(DOXYGEN)
struct _fio_write_arg {
	int	fd;
	const void	*ptr;
	u32	size;
	u32	mis;
	u8	aligned[16];
} ALIGNED(16);

int fioWrite(int fd, const void *ptr, int size)
{
	struct _fio_write_arg arg;
	int mis, res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.fd = fd;
	arg.ptr  = ptr;
	arg.size = size;

	/* Copy the unaligned (16-byte) portion into the argument */
	mis = 0;
	if ((u32)ptr & 0xf) {
		mis = 16 - ((u32)ptr & 0xf);
		if (mis > size)
			mis = size;
	}
	arg.mis = mis;

	if (mis)
		memcpy(arg.aligned, ptr, mis);

	if (!IS_UNCACHED_SEG(ptr))
		SifWriteBackDCache((void*)ptr, size);

	if ((res = SifCallRpc(&_fio_cd, FIO_F_WRITE, _fio_block_mode, &arg, sizeof arg,
					_fio_recv_data, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=(_fio_block_mode == FIO_NOWAIT)?0:_fio_recv_data[0];
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_lseek) || defined(DOXYGEN)
struct _fio_lseek_arg {
	union {
		int	fd;
		int	result;
	} p;
	int	offset;
	int	whence;
} ALIGNED(16);

int fioLseek(int fd, int offset, int whence)
{
	struct _fio_lseek_arg arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.fd   = fd;
	arg.offset = offset;
	arg.whence = whence;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_LSEEK, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.p.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_ioctl) || defined(DOXYGEN)
struct _fio_ioctl_arg {
	union {
		int fd;
		int result;
	} p;
	int request;
	u8 data[1024];		// Will this be ok ?
} ALIGNED(16);

int fioIoctl(int fd, int request, void *data)
{
	struct _fio_ioctl_arg arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.fd = fd;
	arg.request = request;

	if(data != NULL)
		memcpy(arg.data, data, 1024);

	if ((res = SifCallRpc(&_fio_cd, FIO_F_IOCTL, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.p.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_remove) || defined(DOXYGEN)
int fioRemove(const char *name)
{
	union {
		char path[FIO_PATH_MAX];
		int	result;
	} arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.path, name, FIO_PATH_MAX - 1);
	arg.path[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_REMOVE, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_mkdir) || defined(DOXYGEN)
int fioMkdir(const char* path)
{
	union {
		char path[FIO_PATH_MAX];
		int	result;
	} arg;
	int res, result;

 	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.path, path, FIO_PATH_MAX - 1);
	arg.path[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_MKDIR, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_rmdir) || defined(DOXYGEN)
int fioRmdir(const char* dirname)
{
	union {
		char path[FIO_PATH_MAX];
		int	result;
	} arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.path, dirname, FIO_PATH_MAX - 1);
	arg.path[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_RMDIR, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_putc) || defined(DOXYGEN)
int fioPutc(int fd,int c)
{
	return fioWrite(fd,&c,1);
}
#endif

#if defined(F_fio_getc) || defined(DOXYGEN)
int fioGetc(int fd)
{
	int c;

	fioRead(fd,&c,1);
	return c;
}
#endif

#if defined(F_fio_gets) || defined(DOXYGEN)
int fioGets(int fd, char* buff, int n)
{
	// Rather than doing a slow byte-at-a-time read
	// read upto the max langth, then re-position file afterwards
	int read, i;

	read = fioRead(fd, buff, n);

	for (i=0; i<(read-1); i++)
	{
		switch (buff[i])
		{
		case '\n':
			fioLseek(fd, (i + 1) - read, IO_SEEK_CUR);
			buff[i]=0;	// terminate after newline
			return i;

		case 0:
			fioLseek(fd, i-read, IO_SEEK_CUR);
			return i;
		}
	}

	// if we reached here, then we havent found the end of a string
	return i;
}
#endif

#if defined(F_fio_dopen) || defined(DOXYGEN)
int fioDopen(const char *name)
{
	union {
		char name[FIO_PATH_MAX];
		int	result;
	} arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.name, name, FIO_PATH_MAX - 1);
	arg.name[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_DOPEN, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_dclose) || defined(DOXYGEN)
int fioDclose(int fd)
{
	union {
		int fd;
		int result;
	} arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.fd = fd;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_DCLOSE, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_dread) || defined(DOXYGEN)
struct _fio_dread_arg {
	union {
		int fd;
		int result;
	} p;
	io_dirent_t *buf;
} ALIGNED(16);

int fioDread(int fd, io_dirent_t *buf)
{
	struct _fio_dread_arg arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.fd = fd;
	arg.buf = buf;

	if (!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, sizeof(io_dirent_t));

	if ((res = SifCallRpc(&_fio_cd, FIO_F_DREAD, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.p.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_getstat) || defined(DOXYGEN)
struct _fio_getstat_arg {
	union {
		io_stat_t *buf;
		int result;
	} p;
	char name[FIO_PATH_MAX];
} ALIGNED(16);

int fioGetstat(const char *name, io_stat_t *buf)
{
	struct _fio_getstat_arg arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.buf = buf;
	strncpy(arg.name, name, FIO_PATH_MAX - 1);
	arg.name[FIO_PATH_MAX - 1] = 0;

	if (!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, sizeof(io_stat_t));

	if ((res = SifCallRpc(&_fio_cd, FIO_F_GETSTAT, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.p.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_chstat) || defined(DOXYGEN)
struct _fio_chstat_arg {
	union {
		int cbit;
		int result;
	} p;
	io_stat_t stat;
	char name[FIO_PATH_MAX];
};

int fioChstat(const char *name, io_stat_t *buf, u32 cbit)
{
	struct _fio_chstat_arg arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.cbit = cbit;
	memcpy(&arg.stat, buf, sizeof(io_stat_t));
	strncpy(arg.name, name, FIO_PATH_MAX - 1);
	arg.name[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_CHSTAT, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.p.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

#if defined(F_fio_format) || defined(DOXYGEN)
int fioFormat(const char *name)
{
	union {
		char path[FIO_PATH_MAX];
		int	result;
	} arg;
	int res, result;

	if ((res = fioInit()) < 0)
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.path, name, FIO_PATH_MAX - 1);
	arg.path[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_FORMAT, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) >= 0)
	{
		result=arg.result;
	}
	else
	{	//Signal semaphore to avoid a deadlock if SifCallRpc fails.
		SignalSema(_fio_completion_sema);
		result=res;
	}

	return result;
}
#endif

/* The unistd glue functions.  */
#if defined(F_fio_unistd) || defined(DOXYGEN)
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <kernel/dirent.h>

int mkdir(const char *path, mode_t mode)
{
  return fioMkdir(path);
}

int rename(const char *old, const char *new)
{
  return -1;
}

int link(const char *old, const char *new)
{
  return -1;
}

off_t lseek(int fd, off_t offset, int whence)
{
  if (offset > INT_MAX)
    return -1;

  return fioLseek(fd,offset,whence);
}

/* fioGetStat is unstable on an unpatched kernel */
int stat(const char *path, struct stat *st)
{
  long long high;
  int mc = 0;
  unsigned int years = 0;
  io_stat_t f_st;
  struct tm loctime;

  if (fioGetstat(path,&f_st) < 0)
    return -1;

  if ((path[0] == 'm') && (path[1] == 'c'))
    mc = 1;
  if ((path[0] == 'm') && (path[1] == 'a')
      && (path[2] == 's') && (path[3] == 's'))
    mc = 1;
  if ((path[0] == 's') && (path[1] == 'd'))
    mc = 1;
  if ((path[0] == 's') && (path[1] == 'm') && (path[2] == 'b'))
    mc = 1;

  /* Type */
  if (mc)
  {
    if (IO_MC_ISDIR(f_st.mode))
      st->st_mode = S_IFDIR;
    if (IO_MC_ISREG(f_st.mode))
      st->st_mode = S_IFREG;
    if (f_st.mode & IO_MC_R)
      st->st_mode = st->st_mode | S_IRUSR | S_IRGRP | S_IROTH;
    if (f_st.mode & IO_MC_W)
      st->st_mode = st->st_mode | S_IWUSR | S_IWGRP | S_IWOTH;
    if (f_st.mode & IO_MC_X)
      st->st_mode = st->st_mode | S_IXUSR | S_IXGRP | S_IXOTH;
  }
  else
  {
    if (IO_S_ISLNK(f_st.mode))
      st->st_mode = S_IFLNK;
    if (IO_S_ISREG(f_st.mode))
      st->st_mode = S_IFREG;
    if (IO_S_ISDIR(f_st.mode))
      st->st_mode = S_IFDIR;

    /* Access */
    if (f_st.mode & IO_S_IROTH)
      st->st_mode = st->st_mode | S_IROTH;
    if (f_st.mode & IO_S_IWOTH)
      st->st_mode = st->st_mode | S_IWOTH;
    if (f_st.mode & IO_S_IXOTH)
      st->st_mode = st->st_mode | S_IXOTH;

    if (f_st.mode & IO_S_IRGRP)
      st->st_mode = st->st_mode | S_IRGRP;
    if (f_st.mode & IO_S_IWGRP)
      st->st_mode = st->st_mode | S_IWGRP;
    if (f_st.mode & IO_S_IXGRP)
      st->st_mode = st->st_mode | S_IXGRP;

    if (f_st.mode & IO_S_IRUSR)
      st->st_mode = st->st_mode | S_IRUSR;
    if (f_st.mode & IO_S_IWUSR)
      st->st_mode = st->st_mode | S_IWUSR;
    if (f_st.mode & IO_S_IXUSR)
      st->st_mode = st->st_mode | S_IXUSR;
  }

  /* Size */
  st->st_size = f_st.size;

  if (f_st.hisize) {
    high = f_st.hisize;
    st->st_size = st->st_size + (high << 32);
  }

  /* Time, ignores timezone. */
  years = f_st.ctime[7];
  years = (years<<8) + f_st.ctime[6];
  loctime.tm_year =  years;
  loctime.tm_mon  =  f_st.ctime[5]-1;
  loctime.tm_mday =  f_st.ctime[4]-1;
  loctime.tm_hour =  f_st.ctime[3];
  loctime.tm_min  =  f_st.ctime[2];
  loctime.tm_sec  =  f_st.ctime[1];
  st->st_ctime    = mktime(&loctime);

  years = 0;
  years = f_st.atime[7];
  years = (years<<8) + f_st.atime[6];
  loctime.tm_year =  years;
  loctime.tm_mon  =  f_st.atime[5]-1;
  loctime.tm_mday =  f_st.atime[4]-1;
  loctime.tm_hour =  f_st.atime[3];
  loctime.tm_min  =  f_st.atime[2];
  loctime.tm_sec  =  f_st.atime[1];
  st->st_atime    = mktime(&loctime);

  years = 0;
  years = f_st.mtime[7];
  years = (years<<8) + f_st.mtime[6];
  loctime.tm_year =  years;
  loctime.tm_mon  =  f_st.mtime[5]-1;
  loctime.tm_mday =  f_st.mtime[4]-1;
  loctime.tm_hour =  f_st.mtime[3];
  loctime.tm_min  =  f_st.mtime[2];
  loctime.tm_sec  =  f_st.mtime[1];
  st->st_mtime    = mktime(&loctime);

  if ((st->st_ctime == -1) || (st->st_atime == -1) || (st->st_mtime == -1))
    return -1;

  return 0;
}

int fstat(int fd, struct stat *sbuf)
{
  return -1;
}

int isatty(int fd)
{
  return 1;
}

int open(const char *name, int flags, ...)
{
  return fioOpen(name,flags);
}

DIR *opendir (const char *path)
{
  static DIR dir;

  if ((dir.d_fd = fioDopen(path)) < 0)
    return NULL;

  strncpy(dir.d_dir, path, 256);

  return &dir;
}
struct dirent entry;
struct dirent *readdir(DIR *d)
{
  io_dirent_t __fio_entry;

  if (d == NULL)
    return NULL;

  if (d->d_fd < 0)
    return NULL;

  if (fioDread(d->d_fd, &__fio_entry) < 1)
      return NULL;

  strncpy(entry.d_name, __fio_entry.name, 256);

  return &entry;
}

int closedir(DIR *d)
{
  if (d == NULL)
    return -1;

  if (d->d_fd < 0)
    return -1;

  if (fioDclose(d->d_fd) < 0)
    return -1;
  else {
    d->d_fd = -1;
    return 0;
  }
}

void rewinddir(DIR *d)
{
  if (d == NULL)
    return;

  if (d->d_fd < 0)
    return;

  /* Reinitialize by closing and opening. */
  if (fioDclose(d->d_fd) < 0) {
    d->d_fd = -1;
    return;
  }

  d->d_fd = fioDopen(d->d_dir);

  return;
}

int close(int fd)
{
  return fioClose(fd);
}

int read(int fd, void *buf, size_t count)
{
  if (count > INT_MAX)
    return -1;

  return fioRead(fd, buf, count);
}

int remove(const char *path)
{
  return fioRemove(path);
}

int rmdir(const char *path)
{
  return fioRmdir(path);
}

int unlink(const char *path)
{
  return fioRemove(path);
}

int write(int fd, const void *buf, size_t count)
{
  if (count > INT_MAX)
    return -1;

  return fioWrite(fd, buf, (int)count);
}
#endif /* F_fio_unistd */
