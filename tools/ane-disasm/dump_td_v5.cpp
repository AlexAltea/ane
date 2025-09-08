// SPDX-License-Identifier: MIT

#include "dump_td_v5.h"

#include "dump.h"
#include "td_v5.h"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <vector>

static void dump_kernel_dma(const ane::TD_V5 &td)
{
	const auto &dma = td.kernel_dma_src;
	std::println("\n  Kernel DMA Sources");
	for (size_t i = 0; i < 16; ++i) {
		const auto &config = dma.coeff_dma_config[i];
		const bool enabled = static_cast<uint32_t>(config.en) != 0;
		const uint32_t base = dma.coeff_addr[i];
		const uint32_t size = dma.coeff_size[i];

		if (!enabled) {
			std::println("    coeff[{0}]  = Disabled", i);
			continue;
		}

		std::println("    coeff[{0}]  = Enabled (base: 0x{1:X}, size: 0x{2:X})", i, base, size);
		std::println("      cr_h                  = {}", static_cast<uint32_t>(config.cr_h));
		std::println("      cache_hint            = {}", static_cast<uint32_t>(config.cache_hint));
		std::println("      prefetch_participate  = {}", static_cast<uint32_t>(config.prefetch_participate_en));
	}
}

void dump_td_v5(const struct hwx_file *hwx, const struct hwx_section *section)
{
	if (!hwx || !section) {
		return;
	}

	const size_t entry_size = sizeof(ane::TD_V5);
	if (entry_size == 0) {
		return;
	}
	if (section->size < entry_size) {
		std::println("\n__TEXT/__text is smaller than a TD_V5; skipping dump.");
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

	ane::TD_V5 td{};
	std::memcpy(&td, buffer.data(), entry_size);
	const auto &header = td.header;

	std::println("\n  Header");
	PRINT(header, tid, "0x{:X}");
	PRINT(header, nid, "0x{:X}");
	PRINT(header, lnid, "{:d}");
	PRINT(header, eon, "{:d}");
	PRINT(header, exe_cycles, "{:d}");
	PRINT(header, next_size, "{:d}");
	PRINT(header, log_events, "0x{:X}");
	PRINT(header, exceptions, "0x{:X}");
	PRINT(header, debug_log_events, "0x{:X}");
	PRINT(header, debug_exceptions, "0x{:X}");
	PRINT(header, disallow_abort, "{:d}");
	PRINT(header, td_skip, "{:d}");
	PRINT(header, kpc, "{:d}");
	PRINT(header, spl, "{:d}");
	PRINT(header, tsr, "{:d}");
	PRINT(header, spc, "{:d}");
	PRINT(header, dpc, "{:d}");
	PRINT(header, tse, "{:d}");
	PRINT(header, next_priority, "{:d}");
	PRINT(header, tde, "{:d}");
	PRINT(header, src_loc, "{:d}");
	PRINT(header, dst_loc, "{:d}");
	PRINT(header, tq_dis, "{:d}");
	PRINT(header, next_pointer, "0x{:X}");
	print_td_enable_field("r0", header.rbase0, header.rbe0);
	print_td_enable_field("r1", header.rbase1, header.rbe1);
	print_td_enable_field("w", header.wbase, header.wbe);
	print_td_enable_field("t", header.tbase, header.tbe);
	PRINT(header, ene, "{:d}");
	print_td_enable_field("k0", header.kbase0, header.kbe0);
	print_td_enable_field("k1", header.kbase1, header.kbe1);
	print_td_enable_field("k2", header.kbase2, header.kbe2);
	print_td_enable_field("k3", header.kbase3, header.kbe3);

	dump_kernel_dma(td);

	const auto &common = td.common;
	std::println("\n  Common Header");
	PRINT(common, InDim_Win, "{:d}");
	PRINT(common, InDim_Hin, "{:d}");
	PRINT(common, unk004, "0x{:X}");
	PRINT(common, ChCfg_InFmt, "{:d}");
	PRINT(common, ChCfg_OutFmt, "{:d}");
	PRINT(common, Cin_Cin, "{:d}");
	PRINT(common, Cout_Cout, "{:d}");
	PRINT(common, OutDim_Wout, "{:d}");
	PRINT(common, OutDim_Hout, "{:d}");
	PRINT(common, unk018, "0x{:X}");
	PRINT(common, ConvCfg_Kw, "{:d}");
	PRINT(common, ConvCfg_Kh, "{:d}");
	PRINT(common, ConvCfg_OCGSize, "{:d}");
	PRINT(common, ConvCfg_Sx, "{:d}");
	PRINT(common, ConvCfg_Sy, "{:d}");
	PRINT(common, ConvCfg_Px, "{:d}");
	PRINT(common, ConvCfg_Py, "{:d}");
	PRINT(common, ConvCfg_Ox, "{:d}");
	PRINT(common, ConvCfg_Oy, "{:d}");
	PRINT(common, unk020, "0x{:X}");
	PRINT(common, GroupConvCfg_NumGroups, "{:d}");
	PRINT(common, GroupConvCfg_UnicastEn, "{:d}");
	PRINT(common, GroupConvCfg_ElemMultMode, "{:d}");
	PRINT(common, GroupConvCfg_UnicastCin, "{:d}");
	PRINT(common, TileCfg_TileHeight, "{:d}");
	PRINT(common, unk02C, "0x{:X}");
	PRINT(common, unk030, "0x{:X}");
	PRINT(common, Cfg_SmallSourceMode, "{:d}");
	PRINT(common, Cfg_ShPref, "{:d}");
	PRINT(common, Cfg_ShMin, "{:d}");
	PRINT(common, Cfg_ShMax, "{:d}");
	PRINT(common, Cfg_ActiveNE, "{:d}");
	PRINT(common, Cfg_ContextSwitchIn, "{:d}");
	PRINT(common, Cfg_ContextSwitchOut, "{:d}");
	PRINT(common, Cfg_AccDoubleBufEn, "{:d}");
	PRINT(common, TaskInfo_TaskID, "{:d}");
	PRINT(common, TaskInfo_TaskQ, "{:d}");
	PRINT(common, TaskInfo_NID, "{:d}");
	PRINT(common, DPE_Category, "{:d}");

	const auto &tile_dma_src = td.tile_dma_src;
	std::println("\n  Tile DMA Source");
	PRINT(tile_dma_src, DMAConfig.en, "{:d}");
	PRINT(tile_dma_src, DMAConfig.cr_h, "{:d}");
	PRINT(tile_dma_src, DMAConfig.cache_hint, "{:d}");
	PRINT(tile_dma_src, DMAConfig.cache_hint_reuse, "{:d}");
	PRINT(tile_dma_src, DMAConfig.cache_hint_noreuse, "{:d}");
	PRINT(tile_dma_src, DMAConfig.dependency_mode, "{:d}");
	PRINT(tile_dma_src, unk04, "0x{:X}");
	PRINT(tile_dma_src, BaseAddr, "0x{:X}");
	PRINT(tile_dma_src, RowStride, "{:d}");
	PRINT(tile_dma_src, PlaneStride, "{:d}");
	PRINT(tile_dma_src, DepthStride, "{:d}");
	PRINT(tile_dma_src, GroupStride, "{:d}");
	PRINT(tile_dma_src, Fmt.FmtMode, "{:d}");
	PRINT(tile_dma_src, Fmt.Truncate, "{:d}");
	PRINT(tile_dma_src, Fmt.Shift, "{:d}");
	PRINT(tile_dma_src, Fmt.MemFmt, "{:d}");
	PRINT(tile_dma_src, Fmt.OffsetCh, "{:d}");
	PRINT(tile_dma_src, Fmt.Interleave, "{:d}");
	PRINT(tile_dma_src, Fmt.CmpVec, "{:d}");

	const auto &l2_config = td.l2_config;
	std::println("\n  L2 Config");
	PRINT(l2_config, L2Cfg_InputReLU, "{:d}");
	PRINT(l2_config, L2Cfg_PaddingMode, "{:d}");
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
	PRINT(l2_config, SourceBase_Addr, "0x{:X}");
	PRINT(l2_config, SourceChannelStride_Stride, "0x{:X}");
	PRINT(l2_config, SourceRowStride_Stride, "0x{:X}");
	PRINT(l2_config, unk_maybe_stride1, "0x{:X}");
	PRINT(l2_config, unk_maybe_stride2, "0x{:X}");
	PRINT(l2_config, unk01C, "0x{:X}");
	PRINT(l2_config, unk020, "0x{:X}");
	PRINT(l2_config, unk024, "0x{:X}");
	PRINT(l2_config, unk028, "0x{:X}");
	PRINT(l2_config, unk02C, "0x{:X}");
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
	PRINT(l2_config, ConvResultChannelStride_Stride, "0x{:X}");
	PRINT(l2_config, ConvResultRowStride_Stride, "0x{:X}");

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
	PRINT(ne_config, MatrixVectorBias_MatrixVectorBias, "{:d}");
	PRINT(ne_config, AccBias_AccBias, "{:d}");
	PRINT(ne_config, AccBias_AccBiasShift, "{:d}");
	PRINT(ne_config, PostScale_PostScale, "{:d}");
	PRINT(ne_config, PostScale_PostRightShift, "{:d}");

	const auto &tile_dma_dst = td.tile_dma_dst;
	std::println("\n  Tile DMA Dest");
	PRINT(tile_dma_dst, DMAConfig.en, "{:d}");
	PRINT(tile_dma_dst, DMAConfig.cr_h, "{:d}");
	PRINT(tile_dma_dst, DMAConfig.cache_hint, "{:d}");
	PRINT(tile_dma_dst, DMAConfig.l2_bfr_mode, "{:d}");
	PRINT(tile_dma_dst, DMAConfig.bypass_eow, "{:d}");
	PRINT(tile_dma_dst, BaseAddr, "0x{:X}");
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
