// SPDX-License-Identifier: MIT

#include "dump_td_v11.h"

#include "dump.h"
#include "td_v11.h"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <vector>

void dump_td_v11(const struct hwx_file *hwx, const struct hwx_section *section)
{
	if (!hwx || !section) {
		return;
	}

	const size_t entry_size = sizeof(ane::TD_V11);
	if (entry_size == 0) {
		return;
	}
	if (section->size < entry_size) {
		std::println("\n__TEXT/__text is smaller than a TD_V11; skipping dump.");
		return;
	}

	const uint64_t entry_count = section->size / entry_size;
	const uint64_t trailing_bytes = section->size % entry_size;

	std::println("\nTD (__TEXT/__text)");
	std::println("  total entries : {} (showing first)", entry_count);
	if (trailing_bytes) {
		std::println("  trailing bytes : {}", trailing_bytes);
	}

	std::vector<std::uint8_t> buffer(entry_size);
	if (hwx_section_read(hwx, section, 0, buffer.data(), buffer.size()) != 0) {
		std::println(stderr, "Failed to read __TEXT/__text: {}", std::strerror(errno));
		return;
	}

	ane::TD_V11 td{};
	std::memcpy(&td, buffer.data(), entry_size);

	const auto &header = td.header;
	std::println("\n  Header");
	PRINT(header, unk_maybe_log_events, "0x{:X}");
	PRINT(header, unk_maybe_exceptions, "0x{:X}");
	PRINT(header, unk_maybe_debug_log_events, "0x{:X}");
	PRINT(header, unk_maybe_debug_exceptions, "0x{:X}");

	const auto &tile_dma_src = td.tile_dma_src;
	std::println("\n  Tile DMA Src");
	PRINT(tile_dma_src, unk00, "0x{:X}");
	PRINT(tile_dma_src, RowStride, "{:d}");
	PRINT(tile_dma_src, PlaneStride, "{:d}");
	PRINT(tile_dma_src, DepthStride, "{:d}");
	PRINT(tile_dma_src, GroupStride, "{:d}");
	PRINT(tile_dma_src, unk14, "0x{:X}");
	PRINT(tile_dma_src, Fmt.FmtMode, "{:d}");
	PRINT(tile_dma_src, Fmt.Truncate, "{:d}");
	PRINT(tile_dma_src, Fmt.Shift, "{:d}");
	PRINT(tile_dma_src, Fmt.MemFmt, "{:d}");
	PRINT(tile_dma_src, Fmt.OffsetCh, "{:d}");
	PRINT(tile_dma_src, Fmt.Interleave, "{:d}");
	PRINT(tile_dma_src, Fmt.CmpVec, "{:d}");

	const auto &common = td.common;
	std::println("\n  Common Header");
	PRINT(common, InDim_Win, "{:d}");
	PRINT(common, InDim_Hin, "{:d}");
	PRINT(common, Cin_Cin, "{:d}");
	PRINT(common, Cout_Cout, "{:d}");
	PRINT(common, OutDim_Wout, "{:d}");
	PRINT(common, OutDim_Hout, "{:d}");
	PRINT(common, unk010, "{:d}");
	PRINT(common, unk014, "{:d}");

	const auto &l2_config = td.l2_config;
	std::println("\n  L2 Config");
	PRINT(l2_config, SourceCfg_SourceType, "{:d}");
	PRINT(l2_config, SourceCfg_Dependent, "{:d}");
	PRINT(l2_config, SourceCfg_AliasConvSrc, "{:d}");
	PRINT(l2_config, SourceCfg_AliasConvRslt, "{:d}");
	PRINT(l2_config, SourceCfg_DMAFmt, "{:d}");
	PRINT(l2_config, SourceCfg_DMAInterleave, "{:d}");
	PRINT(l2_config, SourceCfg_DMACmpVec, "{:d}");
	PRINT(l2_config, SourceCfg_DMAOffsetCh, "{:d}");
	PRINT(l2_config, SourceCfg_AliasPlanarSrc, "{:d}");
	PRINT(l2_config, SourceCfg_AliasPlanarRslt, "{:d}");
	PRINT(l2_config, SourceChannelStride_Stride, "0x{:X}");
	PRINT(l2_config, SourceRowStride_Stride, "0x{:X}");
	PRINT(l2_config, unk_maybe_stride1, "0x{:X}");
	PRINT(l2_config, unk_maybe_stride2, "0x{:X}");
	PRINT(l2_config, ResultCfg_ResultType, "{:d}");
	PRINT(l2_config, ResultCfg_L2BfrMode, "{:d}");
	PRINT(l2_config, ResultCfg_AliasConvSrc, "{:d}");
	PRINT(l2_config, ResultCfg_AliasConvRslt, "{:d}");
	PRINT(l2_config, ResultCfg_DMAFmt, "{:d}");
	PRINT(l2_config, ResultCfg_DMAInterleave, "{:d}");
	PRINT(l2_config, ResultCfg_DMACmpVec, "{:d}");
	PRINT(l2_config, ResultCfg_DMAOffsetCh, "{:d}");
	PRINT(l2_config, ResultCfg_AliasPlanarSrc, "{:d}");
	PRINT(l2_config, ResultCfg_AliasPlanarRslt, "{:d}");
	PRINT(l2_config, ResultBase_Addr, "0x{:X}");

	const auto &ne_config = td.ne_config;
	std::println("\n  NE Config");
	PRINT(ne_config, KernelCfg_KernelFmt, "{:d}");
	PRINT(ne_config, KernelCfg_PalettizedEn, "{:d}");
	PRINT(ne_config, KernelCfg_PalettizedBits, "{:d}");
	PRINT(ne_config, KernelCfg_SparseFmt, "{:d}");
	PRINT(ne_config, KernelCfg_GroupKernelReuse, "{:d}");
	PRINT(ne_config, MACCfg_OpMode, "{:d}");
	PRINT(ne_config, MACCfg_KernelMode, "{:d}");
	PRINT(ne_config, MACCfg_BiasMode, "{:d}");
	PRINT(ne_config, MACCfg_MatrixBiasEn, "{:d}");
	PRINT(ne_config, MACCfg_BinaryPoint, "{:d}");
	PRINT(ne_config, MACCfg_PostScaleMode, "{:d}");
	PRINT(ne_config, MACCfg_NonlinearMode, "{:d}");
	PRINT(ne_config, PostScale_PostScale, "{:d}");
	PRINT(ne_config, PostScale_PostRightShift, "{:d}");

	const auto &tile_dma_dst = td.tile_dma_dst;
	std::println("\n  Tile DMA Dest");
	PRINT(tile_dma_dst, unk00, "0x{:X}");
	PRINT(tile_dma_dst, RowStride, "{:d}");
	PRINT(tile_dma_dst, PlaneStride, "{:d}");
	PRINT(tile_dma_dst, DepthStride, "{:d}");
	PRINT(tile_dma_dst, GroupStride, "{:d}");
	PRINT(tile_dma_dst, Fmt.FmtMode, "{:d}");
	PRINT(tile_dma_dst, Fmt.Truncate, "{:d}");
	PRINT(tile_dma_dst, Fmt.Shift, "{:d}");
	PRINT(tile_dma_dst, Fmt.MemFmt, "{:d}");
	PRINT(tile_dma_dst, Fmt.OffsetCh, "{:d}");
	PRINT(tile_dma_dst, Fmt.ZeroPadLast, "{:d}");
	PRINT(tile_dma_dst, Fmt.ZeroPadFirst, "{:d}");
	PRINT(tile_dma_dst, Fmt.CmpVecFill, "{:d}");
	PRINT(tile_dma_dst, Fmt.Interleave, "{:d}");
	PRINT(tile_dma_dst, Fmt.CmpVec, "{:d}");
}
