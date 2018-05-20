/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Copyright (c) 2004 adresd <adresd_ps2dev@yahoo.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Advanced I/O library.
 */

#include "types.h"
#include "defs.h"
#include "loadcore.h"
#include "iomanX.h"
#include "sysclib.h"
#include "stdarg.h"
#include "intrman.h"

#define MODNAME "iomanx"
IRX_ID("IOX/File_Manager", 1, 1);

#include "errno.h"

#define MAX_DEVICES 32
#define MAX_FILES   32

static iop_device_t *dev_list[MAX_DEVICES];
iop_file_t file_table[MAX_FILES];

#define isnum(c) ((c) >= '0' && (c) <= '9')

extern struct irx_export_table _exp_iomanx;

extern int hook_ioman();
extern int unhook_ioman();

iop_device_t **GetDeviceList(void)
{
  return(dev_list);
}

int _start(int argc, char **argv)
{
  if(RegisterLibraryEntries(&_exp_iomanx) != 0)
  {
    return MODULE_NO_RESIDENT_END;
  }

  memset(dev_list, 0, sizeof(dev_list));
  memset(file_table, 0, sizeof(file_table));

  if(hook_ioman() != 0)
  {
    return MODULE_NO_RESIDENT_END;
  }

  return MODULE_RESIDENT_END;
}

int shutdown()
{
  unhook_ioman();
  return MODULE_NO_RESIDENT_END;
}

int AddDrv(iop_device_t *device)
{
  int i, res = -1;
  int oldIntr;

  CpuSuspendIntr(&oldIntr);

  for (i = 0; i < MAX_DEVICES; i++)
  {
    if (dev_list[i] == NULL)
    break;
  }

  if (i >= MAX_DEVICES)
  {
    CpuResumeIntr(oldIntr);
    return res;
  }

  dev_list[i] = device;
  CpuResumeIntr(oldIntr);

  FlushIcache();

  if ((res = device->ops->init(device)) < 0)
  {
    dev_list[i] = NULL;
    return(-1);
  }

  return(0);
}

int DelDrv(const char *name)
{
  int i;

  for (i = 0; i < MAX_DEVICES; i++) {
    if (dev_list[i] != NULL && !strcmp(name, dev_list[i]->name)) {
      dev_list[i]->ops->deinit(dev_list[i]);
      dev_list[i] = NULL;
      return 0;
    }
  }

  return -1;
}


static char * find_iop_device(const char *dev, int *unit, iop_device_t **device)
{
  char canon[16];
  char *filename, *tail, *d = (char *)dev;
  int i, len, num = 0;

  if (*d == ' ') {
    while (*d == ' ')
      d++;
      d--;
  }

  if ((tail = index(d, ':')) == NULL)
    return ((char *)-1);

  len = (int)(tail - d);
  strncpy(canon, d, len);
  canon[len] = '\0';

  /* This is the name passed to the device op.  */
  filename = d + len + 1;

  /* Search backward for the unit number.  */
  if (isnum(canon[len - 1])) {
    while (isnum(canon[len - 1])) len--;

    num = strtol(canon + len, 0, 10);
  }

  if (unit)
    *unit = num;

  /* Find the actual device.  */
  canon[len] = '\0';
  for (i = 0; i < MAX_DEVICES; i++) {
    if (dev_list[i] != NULL && !strcmp(canon, dev_list[i]->name)) {
      if (device)
	*device = dev_list[i];

      return filename;
    }
  }

  return ((char *)-1);
}

iop_file_t *get_file(int fd)
{
  if (fd >= MAX_FILES)
    return NULL;

  if (file_table[fd].device != NULL)
    return &file_table[fd];

  return NULL;
}

iop_file_t *get_new_file(void)
{
  int i;
  iop_file_t *fd = NULL;
  int oldIntr;

  CpuSuspendIntr(&oldIntr);

  for (i = 0; i < MAX_FILES; i++)
  {
    if (!file_table[i].device)
    {
      fd = &file_table[i];

      // fill in "device" temporarily to mark the fd as allocated.
      fd->device = (iop_device_t *) 0xFFFFFFFF;
      break;
    }
  }

  CpuResumeIntr(oldIntr);

  return fd;
}

int open(const char *name, int flags, ...)
{
  iop_file_t *f = get_new_file();
  char *filename;
  va_list alist;
  int res = -ENOSYS, mode;

  va_start(alist, flags);
  mode = va_arg(alist, int);
  va_end(alist);

  /* Uses a file descriptor, if it can't open a file descriptor,
     returns -EMFILE. */
  if (!f)
  {
    return -EMFILE;
  }

  /* Finds the file's device, if it can't find the device (-1),
     returns -ENODEV. */
  if ((filename = find_iop_device(name, &f->unit, &f->device)) == ((char *)-1))
  {
    f->device = NULL;
    return -ENODEV;
  }

  f->mode = flags;

  /* A device's open() gets called, if it returns less than 0, clears
     file, and returns the return value. */
  if ((res = f->device->ops->open(f, filename, flags, mode)) < 0)
  {
    f->mode = 0;
    f->device = NULL;
    return res;
  }

  res = (int)(f - file_table);

  return res;
}

int close(int fd)
{
  iop_file_t *f;
  int res;

  /* Finds the file in the file table, if it can't find it,
     returns -EBADF. */
  if ((f = get_file(fd)) == NULL)
    return -EBADF;

  /* A device's close() gets called and its return value is returned.
     The file descriptor is cleared. */

  /* I discovered newlib expects close() to close both files and directories.
     To support this, we add an internal flag to files that were opened with
     dopen. This might also be part of the internal functionality of IOMAN. */
  if (f->mode & 0x0008)
  {
    res = f->device->ops->dclose(f);
  }
  else
  {
    res = f->device->ops->close(f);
  }

  f->mode = 0;
  f->device = NULL;

  return res;
}

int read(int fd, void *ptr, int size)
{
  iop_file_t *f = get_file(fd);

  /* Finds the file in the file table, if it can't find it,
     returns -EBADF. */
  if (f == NULL)
    return -EBADF;

  /* A device's read() gets called and its return value is returned. */ 
  return f->device->ops->read(f, ptr, size);
}

int write(int fd, void *ptr, int size)
{
  iop_file_t *f = get_file(fd);

  /* Finds the file in the file table, if it can't find it,
     returns -EBADF. */
  if (f == NULL)
    return -EBADF;

  /* A device's read() gets called and its return value is returned. */ 
  return f->device->ops->write(f, ptr, size);
}

int lseek(int fd, int offset, int whence)
{
  iop_file_t *f = get_file(fd);

  /* Finds the file in the file table, if it can't find it,
     returns -EBADF. */
  if (f == NULL)
    return -EBADF;

  /* Prints an error here if the whence arg isn't correct. */
  if (whence < IO_SEEK_SET || whence > IO_SEEK_END)
    return -EINVAL;

  /* A device's lseek() gets called, the return value is returned. */
  return f->device->ops->lseek(f, offset, whence);
}

int ioctl(int fd, int cmd, void *arg)
{
  iop_file_t *f = get_file(fd);

  /* Finds the file in the file table, if it can't find it,
     returns -EBADF. */
  if (f == NULL)
    return -EBADF;

  /* A device's ioctl() gets called and its return value returned. */
  return f->device->ops->ioctl(f, cmd, arg);
}

int remove(const char *name)
{
  iop_file_t f;
  char *filename;

  /* Uses a file descriptor, if it can't open a file descriptor,
     returns -EMFILE. */
  
  /* Finds the file's device, if it can't find the device (-1),
     returns -ENODEV. */
  if ((filename = find_iop_device(name, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /* A device's remove() gets called and the return value returned.
     The file descriptor is cleared. */
  return f.device->ops->remove(&f, filename);
}

/* Because mkdir, rmdir, chdir, and sync have similiar arguments (each starts
   with a path followed by an optional integer), we use a common routine to
   handle all of them.  */
static int path_common(const char *name, int arg, int code)
{
  iop_file_t f;
  iop_device_ops_t *dops;
  char *filename;

  if ((filename = find_iop_device(name, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /** This test invalidates any modules not using fileXio. */
  if (code & 0x100)
    if ((f.device->type & 0xf0000000) != FILEXIO_DT_FSEXT)
      return -48;

  dops = (iop_device_ops_t *)f.device->ops;
  switch (code) {
    case 4:	/* mkdir() */
      return dops->mkdir(&f, filename, arg);
    case 5:	/* rmdir() */
      return dops->rmdir(&f, filename);
    case 0x103:	/* chdir() */
      return dops->chdir(&f, filename);
    case 0x106:	/* sync() */
      return dops->sync(&f, filename, arg);
  }

  return -EINVAL;
}

int mkdir(const char *name, int mode)
{
  /* Uses a file descriptor, if it can't open a file descriptor,
     returns -EMFILE. */

  /* Finds the file's device, if it can't find the device (-1),
     returns -ENODEV. */

  /* A device's mkdir() gets called and its return value returned.
     The file descriptor is cleared. */
  return path_common(name, mode, 4);
}

int rmdir(const char *name)
{
  /* Uses a file descriptor, if it can't open a file descriptor,
     returns -EMFILE. */

  /* Finds the file's device, if it can't find the device (-1),
     returns -ENODEV. */

  /* A device's rmdir() gets called and its return value returned.
     The file descriptor is cleared. */
  return path_common(name, 0, 5);
}

int dopen(const char *name)
{
  iop_file_t *f = get_new_file();
  char *filename;
  int res;

  /* Uses a file descriptor, if it can't open a file descriptor,
     returns -EMFILE. */
  if (!f)
    return -EMFILE;

  /* Finds the file's device, if it can't find the device (-1),
     returns -ENODEV. */
  if ((filename = find_iop_device(name, &f->unit, &f->device)) == ((char *)-1))
  {
    f->device = NULL;
    return -ENODEV;
  }

  /* Indicates a directory. */
  f->mode = 0x0008;

  if ((res = f->device->ops->dopen(f, filename)) >= 0)
    res = (int)(f - file_table);
  else
  {
    f->mode = 0;
    f->device = NULL;
  }

  return res;
}

int dread(int fd, io_dirent_t *buf)
{
  iop_file_t *f = get_file(fd);

  /* Finds the file in the file table, if it can't find it,
     returns -EBADF. */
  if (f == NULL ||  !(f->mode & 0x0008))
    return -EBADF;

  /* A device's dread() gets called and the return value returned. */
  return f->device->ops->dread(f, buf);
}

int getstat(const char *name, io_stat_t *stat)
{
  iop_file_t f;
  char *filename;

  /* Uses a file descriptor, if it can't open a file descriptor,
     returns -EMFILE. */

  /* Finds the file's device, if it can't find the device (-1),
     returns -ENODEV. */

  if ((filename = find_iop_device(name, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /* A device's getstat() gets called and the return value returned.
     The file descriptor is cleared. */
  return f.device->ops->getstat(&f, filename, stat);
}

int chstat(const char *name, io_stat_t *stat, unsigned int mask)
{
  iop_file_t f;
  char *filename;

  /* Uses a file descriptor, if it can't open a file descriptor,
     returns -EMFILE. */

  /* Finds the file's device, if it can't find the device (-1),
     returns -ENODEV. */
  if ((filename = find_iop_device(name, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /* A device's chstat() gets called and the return value returned.
     The file descriptor is cleared. */
  return f.device->ops->chstat(&f, filename, stat, mask);
}

int format(const char *dev, const char *blockdev, void *arg, int arglen)
{
  iop_file_t f;
  char *filename;

  /* Uses a file descriptor, if it can't open a file descriptor,
     returns -EMFILE. */

  /* Finds the file's device, if it can't find the device (-1),
     returns -ENODEV. */
  if ((filename = find_iop_device(dev, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /* A device's format() gets called and the return value returned.
     The file descriptor is cleared. */
  return f.device->ops->format(&f, filename, blockdev, arg, arglen);
}

static int link_common(const char *old, const char *new, int code)
{
  iop_file_t f;
  iop_device_t *new_device;
  char *filename, *new_filename = (char *)new;
  int new_unit;

  if ((filename = find_iop_device(old, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /* Make sure the user isn't attempting to link across devices.  */
  if (index(new, ':') != NULL) {
    new_filename = find_iop_device(new, &new_unit, &new_device);
    if ((new_filename == ((char *)-1)) || (new_unit != f.unit) ||
	(new_device != f.device))
      return -ENXIO;
  }

  /** This test invalidates any modules not using fileXio. */
  if ((f.device->type & 0xf0000000) != FILEXIO_DT_FSEXT)
    return -48;

  /* rename() */
  if (code == 7)
    return f.device->ops->rename(&f, filename, new_filename);

  return f.device->ops->symlink(&f, filename, new_filename);
}

int rename(const char *old, const char *new)
{
  return link_common(old, new, 7);
}

int chdir(const char *name)
{
  return path_common(name, 0, 0x103);
}

int sync(const char *dev, int flag)
{
  return path_common(dev, flag, 0x106);
}

int mount(const char *fsname, const char *devname, int flag, void *arg, int arglen)
{
  iop_file_t f;
  char *filename;

  if ((filename = find_iop_device(fsname, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /** This test invalidates any modules not using fileXio. */
  if ((f.device->type & 0xf0000000) != FILEXIO_DT_FSEXT)
    return -48;

  return f.device->ops->mount(&f, filename, devname, flag, arg, arglen);
}

int umount(const char *fsname)
{
  iop_file_t f;
  char *filename;

  if ((filename = find_iop_device(fsname, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /** This test invalidates any modules not using fileXio. */
  if ((f.device->type & 0xf0000000) != FILEXIO_DT_FSEXT)
    return -48;

  return f.device->ops->umount(&f, filename);
}

long long lseek64(int fd, long long offset, int whence)
{
  iop_file_t *f = get_file(fd);

  if (f == NULL)
    return -EBADF;

  if (whence < IO_SEEK_SET || whence > IO_SEEK_END)
    return -EINVAL;

  /** This test invalidates any modules not using fileXio. */
  if ((f->device->type & 0xf0000000) != FILEXIO_DT_FSEXT)
    return -48;

  return f->device->ops->lseek64(f, offset, whence);
}

int devctl(const char *name, int cmd, void *arg, unsigned int arglen,
	    void *buf, unsigned int buflen)
{
  iop_file_t f;
  char *filename;

  if ((filename = find_iop_device(name, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /** This test invalidates any modules not using fileXio. */
  if ((f.device->type & 0xf0000000) != FILEXIO_DT_FSEXT)
    return -48;

  return f.device->ops->devctl(&f, filename, cmd, arg, arglen, buf, buflen);
}

int symlink(const char *old, const char *new)
{
  return link_common(old, new, 8);
}

int readlink(const char *name, char *buf, unsigned int buflen)
{
  iop_file_t f;
  char *filename;

  if ((filename = find_iop_device(name, &f.unit, &f.device)) == ((char *)-1))
    return -ENODEV;

  /** This test invalidates any modules not using fileXio. */
  if ((f.device->type & 0xf0000000) != FILEXIO_DT_FSEXT)
    return -48;

  return f.device->ops->readlink(&f, filename, buf, buflen);
}


int ioctl2(int fd, int command, void *arg, unsigned int arglen, void *buf,
	    unsigned int buflen)
{
  iop_file_t *f;

  if ((f = get_file(fd)) == NULL)
    return -EBADF;

  return f->device->ops->ioctl2(f, command, arg, arglen, buf, buflen);
}
