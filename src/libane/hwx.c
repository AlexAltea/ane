// SPDX-License-Identifier: MIT

#include "hwx.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct hwx_file {
	int fd;
	uint64_t file_size;
	struct MachHeader64 header;
	struct hwx_segment *segments;
	uint32_t segment_count;
	uint32_t td_version;
	struct hwx_thread_state *thread_states;
	uint32_t thread_state_count;
};

struct LoadCommand {
	uint32_t cmd;
	uint32_t cmdsize;
};

struct SegmentCommand64 {
	uint32_t cmd;
	uint32_t cmdsize;
	char segname[16];
	uint64_t vmaddr;
	uint64_t vmsize;
	uint64_t fileoff;
	uint64_t filesize;
	uint32_t maxprot;
	uint32_t initprot;
	uint32_t nsects;
	uint32_t flags;
};

struct Section64 {
	char sectname[16];
	char segname[16];
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

static int hwx_within_file(uint64_t file_size, uint64_t offset, uint64_t length)
{
	if (offset > file_size) {
		return 0;
	}
	if (length > (file_size - offset)) {
		return 0;
	}
	return 1;
}

static void hwx_trim_name(const char *src, size_t max_len, char *dst)
{
	size_t len = 0;
	while (len < max_len && src[len] != '\0') {
		++len;
	}
	memcpy(dst, src, len);
	dst[len] = '\0';
}

static int hwx_pread_exact(int fd, void *buffer, size_t size, uint64_t offset)
{
	size_t total = 0;
	char *dst = (char *)buffer;

	while (total < size) {
		off_t off = (off_t)(offset + total);
		ssize_t read_bytes = pread(fd, dst + total, size - total, off);
		if (read_bytes < 0) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		}
		if (read_bytes == 0) {
			errno = EINVAL;
			return -1;
		}
		total += (size_t)read_bytes;
	}

	return 0;
}

static uint32_t hwx_td_version_for_cpu(enum hwx_cpu_subtype subtype)
{
	switch (subtype) {
	case HWX_CPU_SUBTYPE_T0:
		return 4;
	case HWX_CPU_SUBTYPE_M9:
	case HWX_CPU_SUBTYPE_H11:
		return 5;
	case HWX_CPU_SUBTYPE_H12:
		return 6;
	case HWX_CPU_SUBTYPE_H13:
		return 7;
	case HWX_CPU_SUBTYPE_H15:
		return 8;
	case HWX_CPU_SUBTYPE_H14:
		return 11;
	default:
		return 0;
	}
}

static void hwx_free_segments(struct hwx_segment *segments, uint32_t count)
{
	if (!segments) {
		return;
	}

	for (uint32_t i = 0; i < count; ++i) {
		free(segments[i].sections);
	}
	free(segments);
}

static void hwx_free_thread_states(struct hwx_thread_state *states, uint32_t count)
{
	if (!states) {
		return;
	}

	for (uint32_t i = 0; i < count; ++i) {
		free(states[i].data);
	}
	free(states);
}

struct hwx_file *hwx_open(const char *path)
{
	if (!path) {
		errno = EINVAL;
		return NULL;
	}

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		return NULL;
	}

	struct stat st;
	if (fstat(fd, &st) != 0) {
		close(fd);
		return NULL;
	}

	if (!S_ISREG(st.st_mode)) {
		close(fd);
		errno = EINVAL;
		return NULL;
	}

	uint64_t file_size = (uint64_t)st.st_size;
	struct MachHeader64 header;
	if (hwx_pread_exact(fd, &header, sizeof(header), 0) != 0) {
		close(fd);
		return NULL;
	}

	if (header.magic != HWX_MACHO_MAGIC_64) {
		close(fd);
		errno = EINVAL;
		return NULL;
	}

	if (!hwx_within_file(file_size, sizeof(struct MachHeader64), header.sizeofcmds)) {
		close(fd);
		errno = EINVAL;
		return NULL;
	}

	struct hwx_file *file = calloc(1, sizeof(*file));
	if (!file) {
		close(fd);
		return NULL;
	}

	file->fd = fd;
	file->file_size = file_size;
	file->header = header;

	uint32_t remaining_cmds = header.ncmds;
	uint64_t offset = sizeof(struct MachHeader64);
	uint64_t remaining_bytes = header.sizeofcmds;
	struct hwx_segment *segments = NULL;
	uint32_t segment_count = 0;
	struct hwx_thread_state *thread_states = NULL;
	uint32_t thread_state_count = 0;

	while (remaining_cmds--) {
		if (remaining_bytes < sizeof(struct LoadCommand)) {
			errno = EINVAL;
			goto error;
		}

		struct LoadCommand lc;
		if (hwx_pread_exact(fd, &lc, sizeof(lc), offset) != 0) {
			goto error;
		}

		if (lc.cmdsize < sizeof(struct LoadCommand) || lc.cmdsize > remaining_bytes) {
			errno = EINVAL;
			goto error;
		}

		if (lc.cmd == HWX_LOAD_COMMAND_SEGMENT_64) {
			uint8_t *buffer = (uint8_t *)malloc(lc.cmdsize);
			if (!buffer) {
				goto error;
			}

			if (hwx_pread_exact(fd, buffer, lc.cmdsize, offset) != 0) {
				free(buffer);
				goto error;
			}

			const struct SegmentCommand64 *seg_cmd = (const struct SegmentCommand64 *)buffer;
			size_t expected = sizeof(struct SegmentCommand64) +
					(size_t)seg_cmd->nsects * sizeof(struct Section64);
			if (lc.cmdsize < expected) {
				free(buffer);
				errno = EINVAL;
				goto error;
			}

			if (!hwx_within_file(file_size, seg_cmd->fileoff, seg_cmd->filesize)) {
				free(buffer);
				errno = EINVAL;
				goto error;
			}

			struct hwx_segment segment;
			memset(&segment, 0, sizeof(segment));
			hwx_trim_name(seg_cmd->segname, sizeof(seg_cmd->segname), segment.name);
			segment.vmaddr = seg_cmd->vmaddr;
			segment.vmsize = seg_cmd->vmsize;
			segment.fileoff = seg_cmd->fileoff;
			segment.filesize = seg_cmd->filesize;
			segment.maxprot = seg_cmd->maxprot;
			segment.initprot = seg_cmd->initprot;
			segment.flags = seg_cmd->flags;
			segment.section_count = seg_cmd->nsects;
			segment.sections = NULL;

			if (segment.section_count > 0) {
				const struct Section64 *src_sections =
					(const struct Section64 *)(seg_cmd + 1);
				struct hwx_section *sections = calloc(segment.section_count,
								    sizeof(struct hwx_section));
				if (!sections) {
					free(buffer);
					goto error;
				}

				for (uint32_t i = 0; i < segment.section_count; ++i) {
					struct hwx_section *dst = &sections[i];
					hwx_trim_name(src_sections[i].segname,
						     sizeof(src_sections[i].segname),
						     dst->segment_name);
					hwx_trim_name(src_sections[i].sectname,
						     sizeof(src_sections[i].sectname),
						     dst->section_name);
					dst->addr = src_sections[i].addr;
					dst->size = src_sections[i].size;
					dst->offset = src_sections[i].offset;
					dst->align = src_sections[i].align;
					dst->reloff = src_sections[i].reloff;
					dst->nreloc = src_sections[i].nreloc;
					dst->flags = src_sections[i].flags;
					dst->reserved1 = src_sections[i].reserved1;
					dst->reserved2 = src_sections[i].reserved2;
					dst->reserved3 = src_sections[i].reserved3;
					if (dst->size &&
					    !hwx_within_file(file_size, dst->offset, dst->size)) {
						free(buffer);
						free(sections);
						errno = EINVAL;
						goto error;
					}
				}

				segment.sections = sections;
			}

			struct hwx_segment *new_segments = realloc(
				segments, (segment_count + 1u) * sizeof(struct hwx_segment));
			if (!new_segments) {
				free(buffer);
				free(segment.sections);
				goto error;
			}

			segments = new_segments;
			segments[segment_count] = segment;
			segment_count += 1;
			free(buffer);
		} else if (lc.cmd == HWX_LOAD_COMMAND_THREAD) {
			if (lc.cmdsize < sizeof(struct LoadCommand) + 2u * sizeof(uint32_t)) {
				errno = EINVAL;
				goto error;
			}

			uint8_t *buffer = (uint8_t *)malloc(lc.cmdsize);
			if (!buffer) {
				goto error;
			}

			if (hwx_pread_exact(fd, buffer, lc.cmdsize, offset) != 0) {
				free(buffer);
				goto error;
			}

			size_t remaining = lc.cmdsize;
			const uint8_t *cursor = buffer;

			if (remaining < sizeof(struct LoadCommand)) {
				free(buffer);
				errno = EINVAL;
				goto error;
			}
			cursor += sizeof(struct LoadCommand);
			remaining -= sizeof(struct LoadCommand);

			while (remaining >= 2u * sizeof(uint32_t)) {
				uint32_t flavor;
				uint32_t count;
				memcpy(&flavor, cursor, sizeof(flavor));
				memcpy(&count, cursor + sizeof(flavor), sizeof(count));
				cursor += 2u * sizeof(uint32_t);
				remaining -= 2u * sizeof(uint32_t);

				size_t byte_count = (size_t)count * sizeof(uint32_t);
				if (count != 0 && byte_count / sizeof(uint32_t) != count) {
					free(buffer);
					errno = EINVAL;
					goto error;
				}
				if (byte_count > remaining) {
					byte_count = remaining;
				}
				if (byte_count > UINT32_MAX) {
					byte_count = UINT32_MAX;
				}

				struct hwx_thread_state state;
				memset(&state, 0, sizeof(state));
				state.flavor = flavor;
				state.count = count;
				state.byte_size = (uint32_t)byte_count;
				if (byte_count > 0) {
					state.data = (uint8_t *)malloc(byte_count);
					if (!state.data) {
						free(buffer);
						goto error;
					}
					memcpy(state.data, cursor, byte_count);
				}

				struct hwx_thread_state *new_states = realloc(
					thread_states,
					(thread_state_count + 1u) * sizeof(struct hwx_thread_state));
				if (!new_states) {
					free(state.data);
					free(buffer);
					goto error;
				}

				thread_states = new_states;
				thread_states[thread_state_count] = state;
				thread_state_count += 1;

				cursor += byte_count;
				remaining -= byte_count;
			}

			free(buffer);
		}

		offset += lc.cmdsize;
		remaining_bytes -= lc.cmdsize;
	}

	if (remaining_bytes != 0) {
		errno = EINVAL;
		goto error;
	}

	file->segments = segments;
	file->segment_count = segment_count;
	file->thread_states = thread_states;
	file->thread_state_count = thread_state_count;
	file->td_version = hwx_td_version_for_cpu(hwx_cpu_subtype(file));

	if (!hwx_get_tsk_section(file) || !hwx_get_krn_section(file)) {
		errno = ENOENT;
		goto error;
	}

	return file;

error:
	hwx_free_thread_states(thread_states, thread_state_count);
	hwx_free_segments(segments, segment_count);
	close(fd);
	free(file);
	return NULL;
}

void hwx_close(struct hwx_file *file)
{
	if (!file) {
		return;
	}

	hwx_free_segments(file->segments, file->segment_count);
	hwx_free_thread_states(file->thread_states, file->thread_state_count);
	close(file->fd);
	free(file);
}

const struct MachHeader64 *hwx_header(const struct hwx_file *file)
{
	if (!file) {
		return NULL;
	}
	return &file->header;
}

enum hwx_cpu_subtype hwx_cpu_subtype(const struct hwx_file *file)
{
	if (!file) {
		return HWX_CPU_SUBTYPE_UNKNOWN;
	}
	switch (file->header.cpusubtype & HWX_CPU_SUBTYPE_MASK) {
	case HWX_CPU_SUBTYPE_M9:
		return HWX_CPU_SUBTYPE_M9;
	case HWX_CPU_SUBTYPE_H11:
		return HWX_CPU_SUBTYPE_H11;
	case HWX_CPU_SUBTYPE_T0:
		return HWX_CPU_SUBTYPE_T0;
	case HWX_CPU_SUBTYPE_H12:
		return HWX_CPU_SUBTYPE_H12;
	case HWX_CPU_SUBTYPE_H13:
		return HWX_CPU_SUBTYPE_H13;
	case HWX_CPU_SUBTYPE_H14:
		return HWX_CPU_SUBTYPE_H14;
	case HWX_CPU_SUBTYPE_H15:
		return HWX_CPU_SUBTYPE_H15;
	default:
		return HWX_CPU_SUBTYPE_UNKNOWN;
	}
}

const char *hwx_cpu_subtype_name(enum hwx_cpu_subtype subtype)
{
	switch (subtype) {
	case HWX_CPU_SUBTYPE_M9:
		return "m9";
	case HWX_CPU_SUBTYPE_H11:
		return "h11";
	case HWX_CPU_SUBTYPE_T0:
		return "t0";
	case HWX_CPU_SUBTYPE_H12:
		return "h12";
	case HWX_CPU_SUBTYPE_H13:
		return "h13";
	case HWX_CPU_SUBTYPE_H14:
		return "h14";
	case HWX_CPU_SUBTYPE_H15:
		return "h15";
	default:
		return "unknown";
	}
}

const char *hwx_cpu_subtype_name_raw(uint32_t subtype)
{
	return hwx_cpu_subtype_name(
		(enum hwx_cpu_subtype)(subtype & HWX_CPU_SUBTYPE_MASK));
}

uint32_t hwx_td_version(const struct hwx_file *file)
{
	if (!file) {
		return 0;
	}
	return file->td_version;
}

uint32_t hwx_segment_count(const struct hwx_file *file)
{
	if (!file) {
		return 0;
	}
	return file->segment_count;
}

const struct hwx_segment *hwx_segments(const struct hwx_file *file)
{
	if (!file) {
		return NULL;
	}
	return file->segments;
}

const struct hwx_segment *hwx_segment_by_name(const struct hwx_file *file, const char *name)
{
	if (!file || !name) {
		errno = EINVAL;
		return NULL;
	}

	const struct hwx_segment *segments = file->segments;
	for (uint32_t i = 0; i < file->segment_count; ++i) {
		if (strcmp(segments[i].name, name) == 0) {
			return &segments[i];
		}
	}

	return NULL;
}

const struct hwx_section *hwx_section_by_name(const struct hwx_file *file,
				      const char *segment_name,
				      const char *section_name)
{
	if (!file || !section_name) {
		errno = EINVAL;
		return NULL;
	}

	const struct hwx_segment *segments = file->segments;
	for (uint32_t i = 0; i < file->segment_count; ++i) {
		const struct hwx_segment *segment = &segments[i];
		if (segment_name && segment_name[0] != '\0' &&
		    strcmp(segment->name, segment_name) != 0) {
			continue;
		}

		for (uint32_t j = 0; j < segment->section_count; ++j) {
			const struct hwx_section *section = &segment->sections[j];
			if (strcmp(section->section_name, section_name) == 0) {
				return section;
			}
		}
	}

	return NULL;
}

const struct hwx_section *hwx_get_tsk_section(const struct hwx_file *file)
{
	return hwx_section_by_name(file, "__TEXT", "__text");
}

const struct hwx_section *hwx_get_krn_section(const struct hwx_file *file)
{
	return hwx_section_by_name(file, "__TEXT", "__const");
}

uint32_t hwx_thread_state_count(const struct hwx_file *file)
{
	if (!file) {
		return 0;
	}
	return file->thread_state_count;
}

const struct hwx_thread_state *hwx_thread_states(const struct hwx_file *file)
{
	if (!file) {
		return NULL;
	}
	return file->thread_states;
}

const struct hwx_thread_state *hwx_thread_state_next(const struct hwx_file *file,
					       uint32_t flavor,
					       const struct hwx_thread_state *current)
{
	if (!file) {
		errno = EINVAL;
		return NULL;
	}

	const struct hwx_thread_state *states = file->thread_states;
	const uint32_t count = file->thread_state_count;
	uint32_t start_index = 0;

	if (current) {
		if (current < states || current >= states + count) {
			errno = EINVAL;
			return NULL;
		}
		start_index = (uint32_t)(current - states) + 1u;
	}

	for (uint32_t i = start_index; i < count; ++i) {
		if (states[i].flavor == flavor) {
			return &states[i];
		}
	}

	return NULL;
}

static int hwx_validate_read(const struct hwx_file *file, uint64_t offset, uint64_t size)
{
	if (!file) {
		errno = EINVAL;
		return -1;
	}

	if (!hwx_within_file(file->file_size, offset, size)) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int hwx_segment_read(const struct hwx_file *file,
		       const struct hwx_segment *segment,
		       uint64_t offset,
		       void *buffer,
		       size_t size)
{
	if (!segment || (!buffer && size > 0)) {
		errno = EINVAL;
		return -1;
	}

	if (size == 0) {
		return 0;
	}

	if (offset > segment->filesize ||
	    size > (segment->filesize - offset)) {
		errno = EINVAL;
		return -1;
	}

	uint64_t file_offset = segment->fileoff + offset;
	if (hwx_validate_read(file, file_offset, (uint64_t)size) != 0) {
		return -1;
	}

	return hwx_pread_exact(file->fd, buffer, size, file_offset);
}

int hwx_section_read(const struct hwx_file *file,
		       const struct hwx_section *section,
		       uint64_t offset,
		       void *buffer,
		       size_t size)
{
	if (!section || (!buffer && size > 0)) {
		errno = EINVAL;
		return -1;
	}

	if (size == 0) {
		return 0;
	}

	if (offset > section->size ||
	    size > (section->size - offset)) {
		errno = EINVAL;
		return -1;
	}

	uint64_t file_offset = (uint64_t)section->offset + offset;
	if (hwx_validate_read(file, file_offset, (uint64_t)size) != 0) {
		return -1;
	}

	return hwx_pread_exact(file->fd, buffer, size, file_offset);
}
