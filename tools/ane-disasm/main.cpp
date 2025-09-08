// SPDX-License-Identifier: MIT

#include "dump_td_v5.h"
#include "dump_td_v11.h"
#include "hwx.h"

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <print>
#include <string>

static const char *thread_flavor_name(uint32_t flavor)
{
	switch (flavor) {
	case HWX_ANE_TD_STATE:
		return "HWX_ANE_TD_STATE";
	case HWX_ANE_BIND_STATE:
		return "HWX_ANE_BIND_STATE";
	case HWX_ANE_SEG_STATE:
		return "HWX_ANE_SEG_STATE";
	default:
		return "unknown";
	}
}

static void dump_nonzero_addresses(const char *label, const uint64_t *values, size_t count)
{
	bool any = false;
	for (size_t i = 0; i < count; ++i) {
		const uint64_t value = values[i];
		if (value == 0) {
			continue;
		}
		if (!any) {
			std::println("    {}:", label);
			any = true;
		}
		std::println("      [{:3}] 0x{:016X}", i, value);
	}
	if (!any) {
		std::println("    {}: <none>", label);
	}
}

static void dump_state_bytes(const uint8_t *data, uint32_t size, size_t max_bytes = 64)
{
	if (!data || size == 0) {
		std::println("    data : <none>");
		return;
	}

	const size_t dump = std::min<size_t>(size, max_bytes);
	std::println("    data : showing {} of {} bytes", dump, size);
	for (size_t offset = 0; offset < dump; offset += 16) {
		const size_t line = std::min<size_t>(16, dump - offset);
		std::print("      {:04X} : ", offset);
		for (size_t j = 0; j < line; ++j) {
			std::print("{:02X} ", data[offset + j]);
		}
		std::println("");
	}
	if (dump < size) {
		std::println("      ...");
	}
}

static void dump_thread_state_detail(const struct hwx_thread_state &state)
{
	switch (state.flavor) {
	case HWX_ANE_TD_STATE: {
		if (!state.data || state.byte_size < sizeof(struct hwx_ane_td_state)) {
			std::println("    td_state payload truncated ({} bytes)", state.byte_size);
			dump_state_bytes(state.data, state.byte_size);
			return;
		}
		const auto *td = reinterpret_cast<const struct hwx_ane_td_state *>(state.data);
		dump_nonzero_addresses("td base_addr", td->base_addr, 256);
		std::println("    td_addr  : 0x{:016X}", td->td_addr);
		std::println("    td_words : {}", td->td_words);
		std::println("    td_size  : {}", static_cast<uint64_t>(td->td_words) * 4u + 4u);
		std::println("    td_count : {}", td->td_count);
		std::println("    ane      : {}", td->ane);
		std::println("    ene      : {}", td->ene);
		break;
	}
	case HWX_ANE_BIND_STATE: {
		if (!state.data || state.byte_size < sizeof(struct hwx_ane_bind_state)) {
			std::println("    bind_state payload truncated ({} bytes)", state.byte_size);
			dump_state_bytes(state.data, state.byte_size);
			return;
		}
		const auto *bind = reinterpret_cast<const struct hwx_ane_bind_state *>(state.data);
		std::println("    unk      : 0x{:08X}", bind->unk);
		break;
	}
	case HWX_ANE_SEG_STATE: {
		if (!state.data || state.byte_size < sizeof(struct hwx_ane_seg_state)) {
			std::println("    seg_state payload truncated ({} bytes)", state.byte_size);
			dump_state_bytes(state.data, state.byte_size);
			return;
		}
		const auto *seg = reinterpret_cast<const struct hwx_ane_seg_state *>(state.data);
		dump_nonzero_addresses("seg base_addr", seg->base_addr, 256);
		std::println("    seg_addr : 0x{:016X}", seg->seg_addr);
		std::println("    sect_idx : {}", seg->sect_idx);
		std::println("    seg_header_size    : {}", seg->seg_header_size);
		std::println("    seg_words          : {}", seg->seg_words);
		std::println("    seg_size           : {}", static_cast<uint64_t>(seg->seg_words) * 4u);
		std::println("    seg_id             : {}", seg->seg_id);
		std::println("    first_td_id        : {}", seg->first_td_id);
		std::println("    td_count           : {}", seg->td_count);
		std::println("    next_segment_count : {}", seg->next_segment_count);
		std::println("    next_segment_id    : [{}, {}]", seg->next_segment_id[0], seg->next_segment_id[1]);
		std::println("    ane                : {}", seg->ane);
		std::println("    ene                : {}", seg->ene);
		break;
	}
	default:
		dump_state_bytes(state.data, state.byte_size);
		break;
	}
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		std::println(stderr, "Usage: ane-disasm <path/to/model.hwx>");
		return EXIT_FAILURE;
	}

	const std::filesystem::path path = argv[1];
	struct hwx_file *hwx = hwx_open(path.string().c_str());
	if (!hwx) {
		std::println(stderr, "Failed to load {}: {}", path.string(), std::strerror(errno));
		return EXIT_FAILURE;
	}

	const struct MachHeader64 *header = hwx_header(hwx);
	if (!header) {
		hwx_close(hwx);
		std::println(stderr, "Failed to read HWX header from {}", path.string());
		return EXIT_FAILURE;
	}

	std::println("HWX Header");
	std::println("  magic      : 0x{:X}", header->magic);
	std::println("  cputype    : 0x{:X}", header->cputype);
	std::println("  cpusubtype : 0x{:X} ({})", header->cpusubtype, hwx_cpu_subtype_name_raw(header->cpusubtype));
	std::println("  filetype   : 0x{:X}", header->filetype);
	std::println("  ncmds      : {}", header->ncmds);
	std::println("  sizeofcmds : {}", header->sizeofcmds);
	std::println("  flags      : 0x{:X}", header->flags);
	std::println("  reserved   : 0x{:X}", header->reserved);
	std::println("  td_version : {}", hwx_td_version(hwx));

	const uint32_t segment_count = hwx_segment_count(hwx);
	const struct hwx_segment *segments = hwx_segments(hwx);
	std::println("\nSegments ({})", segment_count);
	const struct hwx_section *td_section = nullptr;
	for (uint32_t i = 0; i < segment_count; ++i) {
		const struct hwx_segment *segment = &segments[i];
		std::println("  Segment {}", segment->name);
		std::println("    vmaddr   : 0x{:X}", segment->vmaddr);
		std::println("    vmsize   : 0x{:X}", segment->vmsize);
		std::println("    fileoff  : 0x{:X}", segment->fileoff);
		std::println("    filesize : 0x{:X}", segment->filesize);
		std::println("    maxprot  : 0x{:X}", segment->maxprot);
		std::println("    initprot : 0x{:X}", segment->initprot);
		std::println("    flags    : 0x{:X}", segment->flags);

		std::println("    Sections ({})", segment->section_count);
		for (uint32_t j = 0; j < segment->section_count; ++j) {
			const struct hwx_section *section = &segment->sections[j];
			std::println("      Section {}", section->section_name);
			std::println("        segment : {}", section->segment_name);
			std::println("        addr    : 0x{:X}", section->addr);
			std::println("        size    : 0x{:X}", section->size);
			std::println("        offset  : 0x{:X}", section->offset);
			std::println("        align   : {}", section->align);
			std::println("        flags   : 0x{:X}", section->flags);

			if (!td_section && std::strcmp(segment->name, "__TEXT") == 0 &&
			    std::strcmp(section->section_name, "__text") == 0) {
				td_section = section;
			}
		}
	}

	const uint32_t thread_state_count = hwx_thread_state_count(hwx);
	std::println("\nThread States ({})", thread_state_count);
	const struct hwx_thread_state *states = hwx_thread_states(hwx);
	if (thread_state_count == 0 || !states) {
		std::println("  <none>");
	} else {
		for (uint32_t i = 0; i < thread_state_count; ++i) {
			const struct hwx_thread_state &state = states[i];
			std::println("  State {}", i);
			std::println("    flavor   : 0x{:X} ({})", state.flavor, thread_flavor_name(state.flavor));
			std::println("    count    : {}", state.count);
			std::println("    byte_size: {}", state.byte_size);
			dump_thread_state_detail(state);
		}
	}

	if (td_section) {
		switch (hwx_td_version(hwx)) {
		case 5:
			dump_td_v5(hwx, td_section);
			break;
		case 7:
			dump_td_v5(hwx, td_section); // TODO
			break;
		case 11:
			dump_td_v11(hwx, td_section);
			break;
		default:
			std::println(stderr, "\nUnsupported TD version: {}", hwx_td_version(hwx));
			break;
		}
	} else {
		std::println("\nNo __TEXT/__text section present.");
	}

	hwx_close(hwx);
	return EXIT_SUCCESS;
}
