#ifndef _WIMLIB_HEADER_H
#define _WIMLIB_HEADER_H

#include "wimlib/resource.h"
#include "wimlib/types.h"
#include "wimlib/endianness.h"

/* Length of "Globally Unique ID" field in WIM header.  */
#define WIM_GID_LEN    16

/* Length of the WIM header on disk.  */
#define WIM_HEADER_DISK_SIZE 208

/* Version of the WIM file.  There is an older version (used for prerelease
 * versions of Windows Vista), but wimlib doesn't support it.  The differences
 * between the versions are undocumented.  */
#define WIM_VERSION 0x10d00

/* Version number used for a different WIM format, which as of Windows 8 can be
 * created by passing 0x20000000 in dwFlagsAndAttributes to WIMGAPI's
 * WIMCreateFile() and specifying either NONE, XPRESS, or LZMS compression.
 * This format is, however, currently undocumented by Microsoft and is seemingly
 * incompatible with their own ImageX and Dism programs.  wimlib does not yet
 * support this format.  */
#define WIM_MYSTERY_VERSION 0xe00

/* WIM magic characters, translated to a single 64-bit little endian number.  */
#define WIM_MAGIC \
		cpu_to_le64(((u64)'M' << 0) |		\
			    ((u64)'S' << 8) |		\
			    ((u64)'W' << 16) |		\
			    ((u64)'I' << 24) |		\
			    ((u64)'M' << 32) |		\
			    ((u64)'\0' << 40) |		\
			    ((u64)'\0' << 48) |		\
			    ((u64)'\0' << 54))

/* wimlib pipable WIM magic characters, translated to a single 64-bit little
 * endian number.  */
#define PWM_MAGIC \
		cpu_to_le64(((u64)'W' << 0) |		\
			    ((u64)'L' << 8) |		\
			    ((u64)'P' << 16) |		\
			    ((u64)'W' << 24) |		\
			    ((u64)'M' << 32) |		\
			    ((u64)'\0' << 40) |		\
			    ((u64)'\0' << 48) |		\
			    ((u64)'\0' << 54))

/* On-disk format of the WIM header. */
struct wim_header_disk {

	/* Magic characters "MSWIM\0\0\0" */
	le64 magic;

	/* Size of the WIM header, in bytes; WIM_HEADER_DISK_SIZE expected
	 * (currently the only supported value). */
	u32 hdr_size;

	/* Version of the WIM file; WIM_VERSION expected (currently the only
	 * supported value). */
	u32 wim_version;

	/* Flags for the WIM file (WIM_HDR_FLAG_*) */
	u32 wim_flags;

	/* Chunk size for compressed resources in the WIM, or 0 if the WIM is
	 * uncompressed.  */
	u32 chunk_size;

	/* Globally unique identifier for the WIM file.  Basically a bunch of
	 * random bytes. */
	u8 guid[WIM_GID_LEN];

	/* Number of this WIM part in the split WIM file, indexed from 1, or 1
	 * if the WIM is not split. */
	u16 part_number;

	/* Total number of parts of the split WIM file, or 1 if the WIM is not
	 * split. */
	u16 total_parts;

	/* Number of images in the WIM. */
	u32 image_count;

	/* Location and size of the WIM's lookup table. */
	struct resource_entry_disk lookup_table_res_entry;

	/* Location and size of the WIM's XML data. */
	struct resource_entry_disk xml_data_res_entry;

	/* Location and size of metadata resource for the bootable image of the
	 * WIM, or all zeroes if no image is bootable. */
	struct resource_entry_disk boot_metadata_res_entry;

	/* 1-based index of the bootable image of the WIM, or 0 if no image is
	 * bootable. */
	u32 boot_idx;

	/* Location and size of the WIM's integrity table, or all zeroes if the
	 * WIM has no integrity table.
	 *
	 * Note the integrity_table_res_entry here is 4-byte aligned even though
	 * it would ordinarily be 8-byte aligned--- hence, the _packed_attribute
	 * on the `struct wim_header_disk' is essential. */
	struct resource_entry_disk integrity_table_res_entry;

	/* Unused bytes. */
	u8 unused[60];
} _packed_attribute;


/* Header at the very beginning of the WIM file.  This is the in-memory
 * representation and does not include all fields; see `struct wim_header_disk'
 * for the on-disk structure.  */
struct wim_header {

	/* Magic characters: either WIM_MAGIC or PWM_MAGIC.  */
	le64 magic;

	/* Bitwise OR of one or more of the WIM_HDR_FLAG_* defined below. */
	u32 flags;

	/* Compressed resource chunk size  */
	u32 chunk_size;

	/* A unique identifier for the WIM file. */
	u8 guid[WIM_GID_LEN];

	/* Part number of the WIM file in a spanned set. */
	u16 part_number;

	/* Total number of parts in a spanned set. */
	u16 total_parts;

	/* Number of images in the WIM file. */
	u32 image_count;

	/* Location, size, and flags of the lookup table of the WIM. */
	struct resource_entry lookup_table_res_entry;

	/* Location, size, and flags for the XML data of the WIM. */
	struct resource_entry xml_res_entry;

	/* Location, size, and flags for the boot metadata.  This means the
	 * metadata resource for the image specified by boot_idx below.  Should
	 * be zeroed out if boot_idx is 0. */
	struct resource_entry boot_metadata_res_entry;

	/* The index of the bootable image in the WIM file. If 0, there are no
	 * bootable images available. */
	u32 boot_idx;

	/* The location of the optional integrity table used to verify the
	 * integrity WIM.  Zeroed out if there is no integrity table.*/
	struct resource_entry integrity;
};

/* Flags for the `flags' field of the struct wim_header: */

/* Reserved for future use */
#define WIM_HDR_FLAG_RESERVED           0x00000001

/* Files and metadata in the WIM are compressed. */
#define WIM_HDR_FLAG_COMPRESSION        0x00000002

/* WIM is read-only, so modifications should not be allowed even if the WIM is
 * writable at the filesystem level. */
#define WIM_HDR_FLAG_READONLY           0x00000004

/* Resource data specified by images in this WIM may be contained in a different
 * WIM.  Or in other words, this WIM is part of a split WIM.  */
#define WIM_HDR_FLAG_SPANNED            0x00000008

/* The WIM contains resources only; no filesystem metadata.  wimlib ignores this
 * flag, as it looks for resources in all the WIMs anyway. */
#define WIM_HDR_FLAG_RESOURCE_ONLY      0x00000010

/* The WIM contains metadata only.  wimlib ignores this flag.  Note that all the
 * metadata resources for a split WIM should be in the first part. */
#define WIM_HDR_FLAG_METADATA_ONLY      0x00000020

/* The WIM is currently being written or appended to.  */
#define WIM_HDR_FLAG_WRITE_IN_PROGRESS  0x00000040

/* Reparse point fixup flag.  See docs for --rpfix and --norpfix in imagex, or
 * WIMLIB_ADD_FLAG_{RPFIX,NORPFIX} in wimlib.h.  Note that
 * WIM_HDR_FLAG_RP_FIX is a header flag and just sets the default behavior for
 * the WIM; it can still be overridder on a per-image basis.  But there is no
 * flag to set the default behavior for a specific image. */
#define WIM_HDR_FLAG_RP_FIX             0x00000080

/* Unused, reserved flag for another compression type */
#define WIM_HDR_FLAG_COMPRESS_RESERVED  0x00010000

/* Resources within the WIM are compressed using "XPRESS" compression, which is
 * a LZ77-based compression algorithm.  */
#define WIM_HDR_FLAG_COMPRESS_XPRESS    0x00020000

/* Resources within the WIM are compressed using "LZX" compression.  This is also
 * a LZ77-based algorithm. */
#define WIM_HDR_FLAG_COMPRESS_LZX       0x00040000

/* Starting in Windows 8, WIMGAPI can create WIMs using LZMS compression, and
 * this flag is set on such WIMs.  However, an additional undocumented flag
 * needs to be provided to WIMCreateFile() to create such WIMs, and the version
 * number in the header of the resulting WIMs is different (3584).  None of this
 * is actually documented, and wimlib does not yet support this compression
 * format.  */
#define WIM_HDR_FLAG_COMPRESS_LZMS      0x00080000

#endif /* _WIMLIB_HEADER_H */
