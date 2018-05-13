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
 * fileXio RPC client
 */

#include <fileXio_rpc.h>

#define __need__va_list
#include <stdarg.h>

/* Needed for unistd glue functions */
#include <sys/stat.h>
#include <time.h>

#include <limits.h>
#include <errno.h>
#ifdef errno
#undef errno
#endif
extern int errno;

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>

#include <kernel/string.h>
#include <kernel/dirent.h>

#ifndef ENOPKG
#define ENOPKG 65
#endif

extern int _iop_reboot_count;
static SifRpcClientData_t cd0;
static unsigned int sbuff[0x1300] __attribute__((aligned (64)));
static int _intr_data[0xC00] __attribute__((aligned(64)));
static int fileXioInited = 0;
static int fileXioBlockMode;
static int fileXioCompletionSema = -1;

static void _fxio_intr(void)
{
	iSignalSema(fileXioCompletionSema);
}

static int _lock_sema_id = -1;
static inline int _lock(void)
{
	return(WaitSema(_lock_sema_id));
}

static inline int _unlock(void)
{
	return(SignalSema(_lock_sema_id));
}

int fileXioInit(void)
{
	int res;
	ee_sema_t sp;
	static int _rb_count = 0;

	if(_rb_count != _iop_reboot_count)
	{
		_rb_count = _iop_reboot_count;

		fileXioExit();
	}

	if(fileXioInited)
	{
		return 0;
	}

	sp.init_count = 1;
	sp.max_count = 1;
	sp.option = 0;
	_lock_sema_id = CreateSema(&sp);

	while(((res = SifBindRpc(&cd0, FILEXIO_IRX, 0)) >= 0) && (cd0.server == NULL))
		nopdelay();

	if(res < 0)
		return res;

	sp.init_count = 1;
	sp.max_count = 1;
	sp.option = 0;
	fileXioCompletionSema = CreateSema(&sp);
	if (fileXioCompletionSema < 0)
		return -1;

	fileXioInited = 1;
	fileXioBlockMode = FXIO_WAIT;

	return 0;
}

void fileXioExit(void)
{
	if(fileXioInited)
	{
		if(_lock_sema_id >= 0) DeleteSema(_lock_sema_id);
		if(fileXioCompletionSema >= 0) DeleteSema(fileXioCompletionSema);

		memset(&cd0, 0, sizeof(cd0));

		fileXioInited = 0;
	}
}

void fileXioStop(void)
{
	if(fileXioInit() < 0)
		return;

	SifCallRpc(&cd0, FILEXIO_STOP, 0, sbuff, 0, sbuff, 0, 0, 0);

	return;
}

int fileXioGetDeviceList(struct fileXioDevice deviceEntry[], unsigned int req_entries)
{
	int rv;
	struct fxio_devlist_packet *packet=(struct fxio_devlist_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->deviceEntry = deviceEntry;
	packet->reqEntries = req_entries;

	// This will get the directory contents, and fill dirEntry via DMA
	if((rv = SifCallRpc(&cd0, FILEXIO_GETDEVICELIST, fileXioBlockMode, sbuff, sizeof(struct fxio_devlist_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioGetdir(const char* pathname, struct fileXioDirEntry dirEntry[], unsigned int req_entries)
{
	int rv;
	struct fxio_getdir_packet *packet=(struct fxio_getdir_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	// copy the requested pathname to the rpc buffer
	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	SifWriteBackDCache(dirEntry, (sizeof(struct fileXioDirEntry) * req_entries));

	packet->dirEntry = dirEntry;
	packet->reqEntries = req_entries;

	// This will get the directory contents, and fill dirEntry via DMA
	if((rv = SifCallRpc(&cd0, FILEXIO_GETDIR, fileXioBlockMode, sbuff, sizeof(struct fxio_getdir_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioMount(const char* mountpoint, const char* mountstring, int flag)
{
	int rv;
	struct fxio_mount_packet *packet=(struct fxio_mount_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->blockdevice, mountstring, sizeof(packet->blockdevice));
	strncpy(packet->mountpoint, mountpoint, sizeof(packet->mountpoint));
	packet->flags = flag;

	if((rv = SifCallRpc(&cd0, FILEXIO_MOUNT, fileXioBlockMode, sbuff, sizeof(struct fxio_mount_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioUmount(const char* mountpoint)
{
	int rv;
	struct fxio_unmount_packet *packet=(struct fxio_unmount_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->mountpoint, mountpoint, sizeof(packet->mountpoint));

	if((rv = SifCallRpc(&cd0, FILEXIO_UMOUNT, fileXioBlockMode, sbuff, sizeof(struct fxio_unmount_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioCopyfile(const char* source, const char* dest, int mode)
{
	int rv;
	struct fxio_copyfile_packet *packet=(struct fxio_copyfile_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));
	packet->mode = mode;

	if((rv = SifCallRpc(&cd0, FILEXIO_COPYFILE, fileXioBlockMode, sbuff, sizeof(struct fxio_copyfile_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioMkdir(const char* pathname, int mode)
{
	int rv;
	struct fxio_mkdir_packet *packet=(struct fxio_mkdir_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));
	packet->mode = mode;

	if((rv = SifCallRpc(&cd0, FILEXIO_MKDIR, fileXioBlockMode, sbuff, sizeof(struct fxio_mkdir_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioRmdir(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&cd0, FILEXIO_RMDIR, fileXioBlockMode, sbuff, sizeof(struct fxio_pathsel_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioRemove(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&cd0, FILEXIO_REMOVE, fileXioBlockMode, sbuff, sizeof(struct fxio_pathsel_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioRename(const char* source, const char* dest)
{
	int rv;
	struct fxio_rename_packet *packet=(struct fxio_rename_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));

	if((rv = SifCallRpc(&cd0, FILEXIO_RENAME, fileXioBlockMode, sbuff, sizeof(struct fxio_rename_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioSymlink(const char* source, const char* dest)
{
	int rv;
	struct fxio_rename_packet *packet=(struct fxio_rename_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));

	if((rv = SifCallRpc(&cd0, FILEXIO_SYMLINK, fileXioBlockMode, sbuff, sizeof(struct fxio_rename_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioReadlink(const char* source, char* buf, int buflen)
{
	int rv;
	struct fxio_readlink_packet *packet=(struct fxio_readlink_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	if( !IS_UNCACHED_SEG(buf))
  	  SifWriteBackDCache(buf, buflen);

	strncpy(packet->source, source, sizeof(packet->source));
	packet->buffer = buf;
	packet->buflen = buflen;

	if((rv = SifCallRpc(&cd0, FILEXIO_READLINK, fileXioBlockMode, sbuff, sizeof(struct fxio_readlink_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioChdir(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&cd0, FILEXIO_CHDIR, fileXioBlockMode, sbuff, sizeof(struct fxio_pathsel_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioOpen(const char* source, int flags, ...)
{
	int rv, mode;
	struct fxio_open_packet *packet=(struct fxio_open_packet*)sbuff;
	va_list alist;

	va_start(alist, flags);
	mode = va_arg(alist, int);	//Retrieve the mode argument, regardless of whether it is expected or not.
	va_end(alist);

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, source, sizeof(packet->pathname));
	packet->flags = flags;
	packet->mode = mode;
	if((rv = SifCallRpc(&cd0, FILEXIO_OPEN, fileXioBlockMode, sbuff, sizeof(struct fxio_open_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioClose(int fd)
{
	int rv;
	struct fxio_close_packet *packet=(struct fxio_close_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;

	if((rv = SifCallRpc(&cd0, FILEXIO_CLOSE, fileXioBlockMode, sbuff, sizeof(struct fxio_close_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

static void recv_intr(void *data_raw)
{
	fxio_rests_pkt *rests = UNCACHED_SEG(data_raw);

	if(rests->ssize) memcpy(rests->sbuf, rests->sbuffer, rests->ssize);
	if(rests->esize) memcpy(rests->ebuf, rests->ebuffer, rests->esize);

	iSignalSema(fileXioCompletionSema);
}

int fileXioRead(int fd, void *buf, int size)
{
	int rv;
	struct fxio_read_packet *packet=(struct fxio_read_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	packet->buffer = buf;
	packet->size = size;
	packet->intrData = _intr_data;

	if (!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, size);

	if((rv = SifCallRpc(&cd0, FILEXIO_READ, fileXioBlockMode, sbuff, sizeof(struct fxio_read_packet), sbuff, 4, &recv_intr, _intr_data)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioWrite(int fd, const void *buf, int size)
{
	unsigned int miss;
	int rv;
	struct fxio_write_packet *packet=(struct fxio_write_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	if((unsigned int)buf & 0x3F)
	{
		miss = 64 - ((unsigned int)buf & 0x3F);
		if(miss > size) miss = size;
	} else {
		miss = 0;
	}

	packet->fd = fd;
	packet->buffer = buf;
	packet->size = size;
	packet->unalignedDataLen = miss;

	memcpy(packet->unalignedData, buf, miss);

	if(!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache((void*)buf, size);

	if((rv = SifCallRpc(&cd0, FILEXIO_WRITE, fileXioBlockMode, sbuff, sizeof(struct fxio_write_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioLseek(int fd, int offset, int whence)
{
	int rv;
	struct fxio_lseek_packet *packet=(struct fxio_lseek_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	packet->offset = (u32)offset;
	packet->whence = whence;

	if((rv = SifCallRpc(&cd0, FILEXIO_LSEEK, fileXioBlockMode, sbuff, sizeof(struct fxio_lseek_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

//
// NOTE: 64-bit
//
s64 fileXioLseek64(int fd, s64 offset, int whence)
{
	s64 rv;
	struct fxio_lseek64_packet *packet=(struct fxio_lseek64_packet*)sbuff;
	struct fxio_lseek64_return_pkt *ret_packet=(struct fxio_lseek64_return_pkt*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	packet->offset_lo = (u32)(offset & 0xffffffff);
	packet->offset_hi = (u32)((offset >> 32) & 0xffffffff);
	packet->whence = whence;

	if((rv = SifCallRpc(&cd0, FILEXIO_LSEEK64, fileXioBlockMode, sbuff, sizeof(struct fxio_lseek64_packet), sbuff, 8, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else {
			s64 rvHI = ret_packet->pos_hi;
			rvHI = rvHI << 32;
			rv = rvHI | ret_packet->pos_lo;
		}
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();

	return(rv);
}

int fileXioChStat(const char *name, io_stat_t *stat, int mask)
{
	int rv;
	struct fxio_chstat_packet *packet=(struct fxio_chstat_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	packet->stat = stat;
	packet->mask = mask;

	if(!IS_UNCACHED_SEG(stat))
		SifWriteBackDCache(stat, sizeof(io_stat_t));

	if((rv = SifCallRpc(&cd0, FILEXIO_CHSTAT, fileXioBlockMode, sbuff, sizeof(struct fxio_chstat_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioGetStat(const char *name, io_stat_t *stat)
{
	int rv;
	struct fxio_getstat_packet *packet=(struct fxio_getstat_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	packet->stat = stat;

	if(!IS_UNCACHED_SEG(stat))
		SifWriteBackDCache(stat, sizeof(io_stat_t));

	if((rv = SifCallRpc(&cd0, FILEXIO_GETSTAT, fileXioBlockMode, sbuff, sizeof(struct fxio_getstat_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioFormat(const char *dev, const char *blockdev, const void *args, int arglen)
{
	int rv;
	struct fxio_format_packet *packet=(struct fxio_format_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->device, dev, sizeof(packet->device));
	if(blockdev)
		strncpy(packet->blockDevice, blockdev, sizeof(packet->blockDevice));

	if(arglen > sizeof(packet->args)) arglen = sizeof(packet->args);
	memcpy(packet->args, args, arglen);
	packet->arglen = arglen;

	if((rv = SifCallRpc(&cd0, FILEXIO_FORMAT, fileXioBlockMode,  sbuff, sizeof(struct fxio_format_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioSync(const char *devname, int flag)
{
	int rv;
	struct fxio_sync_packet *packet=(struct fxio_sync_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->device, devname, sizeof(packet->device));
	packet->flags = flag;

	if((rv = SifCallRpc(&cd0, FILEXIO_SYNC, fileXioBlockMode, sbuff, sizeof(struct fxio_sync_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioDopen(const char *name)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	if((rv = SifCallRpc(&cd0, FILEXIO_DOPEN, fileXioBlockMode, sbuff, sizeof(struct fxio_pathsel_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioDclose(int fd)
{
	int rv;
	struct fxio_close_packet *packet=(struct fxio_close_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	if((rv = SifCallRpc(&cd0, FILEXIO_DCLOSE, fileXioBlockMode, sbuff, sizeof(struct fxio_close_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioDread(int fd, io_dirent_t *dirent)
{
	int rv;
	struct fxio_dread_packet *packet=(struct fxio_dread_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	packet->dirent = dirent;

	if (!IS_UNCACHED_SEG(dirent))
		SifWriteBackDCache(dirent, sizeof(io_dirent_t));

	if((rv = SifCallRpc(&cd0, FILEXIO_DREAD, fileXioBlockMode, sbuff, sizeof(struct fxio_dread_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

static void fxio_ctl_intr(void *data_raw)
{
	struct fxio_ctl_return_pkt *pkt = UNCACHED_SEG(data_raw);

	memcpy(pkt->dest, pkt->buf, pkt->len);

	iSignalSema(fileXioCompletionSema);
}

int fileXioDevctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	struct fxio_devctl_packet *packet = (struct fxio_devctl_packet *)sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	if(arglen > CTL_BUF_SIZE) arglen = CTL_BUF_SIZE;
	if(buflen > CTL_BUF_SIZE) buflen = CTL_BUF_SIZE;
	strncpy(packet->name, name, CTL_BUF_SIZE);
	packet->name[CTL_BUF_SIZE-1] = '\0';
	memcpy(packet->arg, arg, arglen);

	packet->cmd = cmd;
	packet->arglen = arglen;
	packet->buf = buf;
	packet->buflen = buflen;
	packet->intr_data = _intr_data;

	SifWriteBackDCache(buf, buflen);

	if(buflen)
		rv = SifCallRpc(&cd0, FILEXIO_DEVCTL, fileXioBlockMode, packet, sizeof(struct fxio_devctl_packet), sbuff, 4, &fxio_ctl_intr, _intr_data);
	else
		rv = SifCallRpc(&cd0, FILEXIO_DEVCTL, fileXioBlockMode, packet, sizeof(struct fxio_devctl_packet), sbuff, 4, (void *)&_fxio_intr, NULL);

	if(rv >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioIoctl(int fd, int cmd, void *arg){
	struct fxio_ioctl_packet *packet = (struct fxio_ioctl_packet *)sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	memcpy(packet->arg, arg, IOCTL_BUF_SIZE);

	packet->fd = fd;
	packet->cmd = cmd;

	if((rv = SifCallRpc(&cd0, FILEXIO_IOCTL, fileXioBlockMode, packet, sizeof(struct fxio_ioctl_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioIoctl2(int fd, int command, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	struct fxio_ioctl2_packet *packet = (struct fxio_ioctl2_packet *)sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	if(arglen > CTL_BUF_SIZE) arglen = CTL_BUF_SIZE;
	if(buflen > CTL_BUF_SIZE) buflen = CTL_BUF_SIZE;
	memcpy(packet->arg, arg, arglen);

	packet->fd = fd;
	packet->cmd = command;
	packet->arglen = arglen;
	packet->buf = buf;
	packet->buflen = buflen;
	packet->intr_data = _intr_data;

	SifWriteBackDCache(buf, buflen);

	if(buflen)
		rv = SifCallRpc(&cd0, FILEXIO_IOCTL2, fileXioBlockMode, packet, sizeof(struct fxio_ioctl2_packet), sbuff, 4, &fxio_ctl_intr, _intr_data);
	else
		rv = SifCallRpc(&cd0, FILEXIO_IOCTL2, fileXioBlockMode, packet, sizeof(struct fxio_ioctl2_packet), sbuff, 4, (void *)&_fxio_intr, NULL);

	if(rv >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioWaitAsync(int mode, int *retVal)
{
	if(fileXioInit() < 0)
		return -ENOPKG;

	if(fileXioBlockMode != FXIO_NOWAIT) return 0;

	switch(mode)
	{
		case FXIO_WAIT:

			WaitSema(fileXioCompletionSema);
			SignalSema(fileXioCompletionSema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&sbuff[0]);

			return FXIO_COMPLETE;

		case FXIO_NOWAIT:

			if(PollSema(fileXioCompletionSema) < 0)
				return FXIO_INCOMPLETE;

			SignalSema(fileXioCompletionSema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&sbuff[0]);

			return FXIO_COMPLETE;

		default:
			return -1;
	}
}

void fileXioSetBlockMode(int blocking)
{
	fileXioBlockMode = blocking;
}

int fileXioSetRWBufferSize(int size){
	struct fxio_rwbuff *packet = (struct fxio_rwbuff *)sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->size = size;

	if((rv = SifCallRpc(&cd0, FILEXIO_SETRWBUFFSIZE, 0, packet, sizeof(struct fxio_rwbuff), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		rv = sbuff[0];
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

/* The unistd glue functions*/
int mkdir(const char *path, mode_t mode)
{
  return fileXioMkdir(path,mode);
}

int rename(const char *old, const char *new)
{
  return fileXioRename(old, new);
}

int link(const char *path, const char *link)
{
  return fileXioSymlink(path, link);
}

off_t lseek(int fd, off_t offset, int whence)
{
  /* If I remember right, fileXio uses a 64-bit offset value
     as a trick to read up to 4 gb sized files using the max value of an
     unsigned int. Otherwise, it would be limited to 2 gb. */
  if (offset > UINT_MAX)
    return -1; //-EOVERFLOW;

  return fileXioLseek(fd,offset,whence);
}

int stat(const char *path, struct stat *st)
{
  long long high;
  int mc = 0;
  unsigned int years = 0;
  io_stat_t f_st;
  struct tm loctime;

  if (fileXioGetStat(path,&f_st) < 0)
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

  /* I think hisize stores the upper 32-bits of a 64-bit size value */
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
  int mode;
  va_list list;

  va_start(list,flags);
  mode = va_arg(list,int);
  va_end(list);

  return fileXioOpen(name, flags, mode);
}

DIR *opendir (const char *path)
{
  static DIR dir;

  if ((dir.d_fd = fileXioDopen(path)) < 0)
    return NULL;

  strncpy(dir.d_dir, path, 256);

  return &dir;
}

static struct dirent entry;
struct dirent *readdir(DIR *d)
{
  io_dirent_t __io_entry;


  if (d == NULL)
    return NULL;

  if (d->d_fd < 0)
    return NULL;

  if (fileXioDread(d->d_fd, &__io_entry) < 1)
      return NULL;

  strncpy(entry.d_name, __io_entry.name, 256);

  return &entry;
}

int closedir(DIR *d)
{
  if (d == NULL)
    return -1;

  if (d->d_fd < 0)
    return -1;

  if (fileXioDclose(d->d_fd) < 0)
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
  if (fileXioDclose(d->d_fd) < 0) {
    d->d_fd = -1;
    return;
  }

  if ((d->d_fd = fileXioDopen(d->d_dir)) < 0)
    d->d_fd = -1;

  return;
}

int close(int fd)
{
  return fileXioClose(fd);
}

int read(int fd, void *buf, size_t count)
{
  if (count > INT_MAX)
    return -1;

  return fileXioRead(fd, buf, (int)count);
}

int remove(const char *path)
{
  return fileXioRemove(path);
}

int rmdir(const char *path)
{
  return fileXioRmdir(path);
}

int unlink(const char *path)
{
  return fileXioRemove(path);
}

int write(int fd, const void *buf, size_t count)
{
  if (count > INT_MAX)
    return -1;

  return fileXioWrite(fd, buf, (int)count);
}
