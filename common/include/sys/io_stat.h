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
 * File I/O stat defines, types, and symbols.
 */

#ifndef SYS_IO_STAT_H
#define SYS_IO_STAT_H

/** @addtogroup fileio
    @{
*/

/**	These are the partition filesystem types stored in stat.mode when
	reading "hdd0:" as a directory.
*/

#define	IO_FSTYPE_EXT2		0x0083
#define	IO_FSTYPE_EXT2_SWAP	0x0082
#define	IO_FSTYPE_PFS		0x0100
#define	IO_FSTYPE_EMPTY		0x0000

/**	These are used for filesystems with advanced file
	attributes or permissions.

	Add to this list any known devices that use these attributes.
	Known device list: pfs
*/

/** File types */
#define IO_S_IFMT		070000
#define IO_S_IFLNK		040000
#define IO_S_IFREG		020000
#define IO_S_IFDIR		010000

/** File ID bits */
#define IO_S_ISUID		04000
#define IO_S_ISGID		02000
#define IO_S_ISVTX		01000

/** User access */
#define IO_S_IRWXU		0700
#define IO_S_IRUSR		0400
#define IO_S_IWUSR		0200
#define IO_S_IXUSR		0100

/** Group access */
#define IO_S_IRWXG		0070
#define IO_S_IRGRP		0040
#define IO_S_IWGRP		0020
#define IO_S_IXGRP		0010

/** Other access */
#define IO_S_IRWXO		0007
#define IO_S_IROTH		0004
#define IO_S_IWOTH		0002
#define IO_S_IXOTH		0001

/** All access */
#define IO_S_IRWXA	(IO_S_IRWXU|IO_S_IRWXG|IO_S_IRWXO)
#define IO_S_IRALL	(IO_S_IRUSR|IO_S_IRGRP|IO_S_IROTH)
#define IO_S_IWALL	(IO_S_IWUSR|IO_S_IWGRP|IO_S_IWOTH)
#define IO_S_IXALL	(IO_S_IXUSR|IO_S_IXGRP|IO_S_IXOTH)

#define IO_S_IMASK	(IO_S_ISUID|IO_S_ISGID|IO_S_VTX|IO_S_IRWXA)

#define IO_S_ISLNK(m)	(((m) & IO_S_IFMT) == IO_S_IFLNK)
#define IO_S_ISREG(m)	(((m) & IO_S_IFMT) == IO_S_IFREG)
#define IO_S_ISDIR(m)	(((m) & IO_S_IFMT) == IO_S_IFDIR)

/**	These are used for filesystems without advanced file
	attributes or permissions.
	FAT, memory cards, etc.

	Add to this list any known devices that use these attributes.
	Known device list: mc
	                   mass (usbhdfsd)
	                   sd (iEEE1394_disk)
	                   smb (smbman)
	                   devfs (devfs)
	                   host (depends on implementation)
*/

#define IO_MC_IFMT		0x0030
#define IO_MC_IFDIR		0x0020
#define IO_MC_IFREG		0x0010

#define IO_MC_RWXC		0x000F
#define IO_MC_R			0x0001
#define IO_MC_W			0x0002
#define IO_MC_X			0x0004
#define IO_MC_C			0x0008

#define IO_MC_ISREG(m)	(((m) & IO_MC_IFMT) == IO_MC_IFREG)
#define IO_MC_ISDIR(m)	(((m) & IO_MC_IFMT) == IO_MC_IFDIR)

/** The field to be changed in Chstat. */
#define IO_CST_MODE		0x0001
#define IO_CST_ATTR		0x0002
#define IO_CST_SIZE		0x0004
#define IO_CST_CT		0x0008
#define IO_CST_AT		0x0010
#define IO_CST_MT		0x0020
#define IO_CST_PRVT		0x0040

#define IO_DIRENT_SIZE 320

typedef struct {
	unsigned int	mode;
	unsigned int	attr;
	unsigned int	size;
	unsigned char	ctime[8];
	unsigned char	atime[8];
	unsigned char	mtime[8];
	unsigned int	hisize;
	/** Number of subs (main) / subpart number (sub) */
	unsigned int	private_0;
	unsigned int	private_1;
	unsigned int	private_2;
	unsigned int	private_3;
	unsigned int	private_4;
	/** Sector start.  */
	unsigned int	private_5;
} io_stat_t;

typedef struct {
	io_stat_t	stat;
	char		name[256];
	unsigned int	private_0; /* Restricted after being used. */
} io_dirent_t;


/** These are used for compatibility with memorycards and filesystem
    modules that don't use the above dirent type. */

#define FIO_DIRENT_SIZE 296

typedef struct {
	unsigned int	mode;
	unsigned int	attr;
	unsigned int	size;
	unsigned char	ctime[8];
	unsigned char	atime[8];
	unsigned char	mtime[8];
	unsigned int	hisize;
} fio_stat_t;

/* I believe there may be another restricted private member at unused[5]. */
typedef struct {
	fio_stat_t	stat;
	char		name[256];
	unsigned int	private_0; /* Restricted after being used. */
	unsigned int	unused[6];
} fio_dirent_t;

/** End of addtogroup fileio
 *  @}
 */

#endif /* SYS_IO_STAT_H */

