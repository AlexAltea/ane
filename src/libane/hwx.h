// SPDX-License-Identifier: MIT
/* Copyright 2025 Alexandro Sanchez Bach <alexandro@phi.nz> */

#ifndef HWX_H_
#define HWX_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HWX_SEGMENT_NAME_MAX 16
#define HWX_SECTION_NAME_MAX 16
#define HWX_CPU_SUBTYPE_MASK 0xFFu

#define HWX_MACHO_MAGIC_64 0xBEEFFACEu
#define HWX_LOAD_COMMAND_SEGMENT_64 0x19u
#define HWX_LOAD_COMMAND_THREAD 0x04u

struct MachHeader64 {
	uint32_t magic;
	uint32_t cputype;
	uint32_t cpusubtype;
	uint32_t filetype;
	uint32_t ncmds;
	uint32_t sizeofcmds;
	uint32_t flags;
	uint32_t reserved;
};

enum hwx_cpu_subtype {
	HWX_CPU_SUBTYPE_M9 = 0x0u,
	HWX_CPU_SUBTYPE_H11 = 0x1u,
	HWX_CPU_SUBTYPE_T0 = 0x2u,
	HWX_CPU_SUBTYPE_H12 = 0x3u,
	HWX_CPU_SUBTYPE_H13 = 0x4u,
	HWX_CPU_SUBTYPE_H14 = 0x5u,
	HWX_CPU_SUBTYPE_H15 = 0x6u,
	HWX_CPU_SUBTYPE_UNKNOWN = 0xFFFFFFFFu
};

struct hwx_section {
	char segment_name[HWX_SEGMENT_NAME_MAX + 1];
	char section_name[HWX_SECTION_NAME_MAX + 1];
	uint64_t addr;
	uint64_t size;
	uint32_t offset;
	uint32_t align;
	uint32_t reloff;
	uint32_t nreloc;
	uint32_t flags;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
};

struct hwx_segment {
	char name[HWX_SEGMENT_NAME_MAX + 1];
	uint64_t vmaddr;
	uint64_t vmsize;
	uint64_t fileoff;
	uint64_t filesize;
	uint32_t maxprot;
	uint32_t initprot;
	uint32_t flags;
	uint32_t section_count;
	struct hwx_section *sections;
};

struct hwx_thread_state {
	uint32_t flavor;
	uint32_t count;
	uint32_t byte_size;
	uint8_t *data;
};

enum hwx_thread_flavor {
	HWX_ANE_TD_STATE = 1u,
	HWX_ANE_BIND_STATE = 3u,
	HWX_ANE_SEG_STATE = 4u,
};

struct hwx_ane_td_state {
	uint64_t base_addr[256];
	uint64_t td_addr;
	uint32_t td_words; // td_size == td_words * 4 + 4
	uint32_t td_count;
	uint32_t ane;
	uint32_t ene;
};

struct hwx_ane_bind_state {
	uint32_t unk;
};

struct hwx_ane_seg_state {
	uint64_t base_addr[256];
	uint64_t seg_addr;
	uint64_t sect_idx;
	uint32_t seg_header_size;
	uint32_t seg_words; // seg_size == seg_words * 4
	uint32_t seg_id;
	uint32_t first_td_id;
	uint32_t td_count;
	uint32_t next_segment_count;
	uint32_t next_segment_id[2];
	uint32_t ane;
	uint32_t ene;
};

struct hwx_file;

struct hwx_file *hwx_open(const char *path);
void hwx_close(struct hwx_file *file);

const struct MachHeader64 *hwx_header(const struct hwx_file *file);
enum hwx_cpu_subtype hwx_cpu_subtype(const struct hwx_file *file);
const char *hwx_cpu_subtype_name(enum hwx_cpu_subtype subtype);
const char *hwx_cpu_subtype_name_raw(uint32_t subtype);
uint32_t hwx_td_version(const struct hwx_file *file);

uint32_t hwx_segment_count(const struct hwx_file *file);
const struct hwx_segment *hwx_segments(const struct hwx_file *file);
const struct hwx_segment *hwx_segment_by_name(const struct hwx_file *file, const char *name);
const struct hwx_section *hwx_section_by_name(const struct hwx_file *file, const char *segment_name, const char *section_name);
const struct hwx_section *hwx_get_tsk_section(const struct hwx_file *file);
const struct hwx_section *hwx_get_krn_section(const struct hwx_file *file);

uint32_t hwx_thread_state_count(const struct hwx_file *file);
const struct hwx_thread_state *hwx_thread_states(const struct hwx_file *file);
const struct hwx_thread_state *hwx_thread_state_next(const struct hwx_file *file, uint32_t flavor, const struct hwx_thread_state *current);

int hwx_segment_read(const struct hwx_file *file, const struct hwx_segment *segment, uint64_t offset, void *buffer, size_t size);
int hwx_section_read(const struct hwx_file *file, const struct hwx_section *section, uint64_t offset, void *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif // HWX_H_
