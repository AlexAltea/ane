// SPDX-License-Identifier: MIT
/* Copyright 2025 Alexandro Sanchez Bach <alexandro@phi.nz> */

#ifndef TD_V11_H_
#define TD_V11_H_

#include <cstddef>

#include <libane/integer.h>

#include "td_v5.h"
#include "td_v7.h"

namespace ane {

struct TDHeader_V11 {
    U32 unk00;
    U32 unk04;
    U32 unk08;
    U32 unk0C;
    U32 unk10;
    U32 unk14;
    union {
        Bitfield<U32, 0, 24> unk_maybe_log_events;
    };
    union {
        Bitfield<U32, 0, 24> unk_maybe_exceptions;
    };
    union {
        Bitfield<U32, 0, 24> unk_maybe_debug_log_events;
    };
    union {
        Bitfield<U32, 0, 24> unk_maybe_debug_exceptions;
    };
};

enum CacheHint_V11 {
    cache_hint_alloc,
    cache_hint_noalloc = 0x2,
    cache_hint_drop,
    cache_hint_depri,
};

union TDCoeffDMAConfig_V11 {
    Bitfield<U32,  0, 1> en;
    Bitfield<U32,  4, 4> cache_hint; // 0x2 = cache_hint_noalloc
    Bitfield<U32,  8, 8> data_set_id;
};

struct KernelDMASrc_V11 {
    // coeff_dma_config[16]
    // coeff_bfr_size[16] (has to be 0x40-aligned and >=0x40)
    // header_dma_config[4]
    // header_bfr_size[4]
};

using DMASrcFormat_V11 = DMASrcFormat_V5;
using DMADstFormat_V11 = DMADstFormat_V5;

struct TileDMASrc_V11 {
    uint32_t unk00;
    uint32_t RowStride;
    uint32_t PlaneStride;
    uint32_t DepthStride;
    uint32_t GroupStride;
    uint32_t unk14;
    DMASrcFormat_V11 Fmt;
};

struct CommonHeader_V11 {
    union {
        Bitfield<U32,  0, 15> InDim_Win;
        Bitfield<U32, 16, 15> InDim_Hin;
    };
    union {
        Bitfield<U32, 0, 17> Cin_Cin;
    };
    union {
        Bitfield<U32, 0, 17> Cout_Cout;
    };
    union {
        Bitfield<U32,  0, 15> OutDim_Wout;
        Bitfield<U32, 16, 15> OutDim_Hout;
    };
    U32 unk010;
    U32 unk014;
};

struct L2Config_V11 {
    union {
        Bitfield<U32,  0, 2> SourceCfg_SourceType;
        Bitfield<U32,  2, 2> SourceCfg_Dependent;
        Bitfield<U32,  4, 1> SourceCfg_AliasConvSrc;
        Bitfield<U32,  5, 1> SourceCfg_AliasConvRslt;
        Bitfield<U32,  6, 2> SourceCfg_DMAFmt;
        Bitfield<U32,  8, 4> SourceCfg_DMAInterleave;
        Bitfield<U32, 12, 4> SourceCfg_DMACmpVec;
        Bitfield<U32, 16, 3> SourceCfg_DMAOffsetCh;
        Bitfield<U32, 20, 1> SourceCfg_AliasPlanarSrc;
        Bitfield<U32, 22, 1> SourceCfg_AliasPlanarRslt;
    };
    union {
        Bitfield<U32, 4, 17> SourceChannelStride_Stride;
    };
    union {
        Bitfield<U32, 4, 17> SourceRowStride_Stride;
    };
    U32 unk_maybe_stride1;
    U32 unk_maybe_stride2;
    union {
        Bitfield<U32,  0, 2> ResultCfg_ResultType;
        Bitfield<U32,  3, 1> ResultCfg_L2BfrMode;
        Bitfield<U32,  4, 1> ResultCfg_AliasConvSrc;
        Bitfield<U32,  5, 1> ResultCfg_AliasConvRslt;
        Bitfield<U32,  6, 2> ResultCfg_DMAFmt;
        Bitfield<U32,  8, 4> ResultCfg_DMAInterleave;
        Bitfield<U32, 12, 4> ResultCfg_DMACmpVec;
        Bitfield<U32, 16, 3> ResultCfg_DMAOffsetCh;
        Bitfield<U32, 20, 1> ResultCfg_AliasPlanarSrc;
        Bitfield<U32, 22, 1> ResultCfg_AliasPlanarRslt;
    };
    union {
        Bitfield<U32, 4, 17> ResultBase_Addr;
    };
};

struct NEConfig_V11 {
    union {
        Bitfield<U32,  0, 2> KernelCfg_KernelFmt;
        Bitfield<U32,  2, 1> KernelCfg_PalettizedEn;
        Bitfield<U32,  4, 4> KernelCfg_PalettizedBits;
        Bitfield<U32,  8, 1> KernelCfg_SparseFmt;
        Bitfield<U32, 10, 1> KernelCfg_GroupKernelReuse;
    };
    union {
        Bitfield<U32,  0, 3> MACCfg_OpMode;
        Bitfield<U32,  3, 1> MACCfg_KernelMode;
        Bitfield<U32,  4, 1> MACCfg_BiasMode;
        Bitfield<U32,  6, 1> MACCfg_MatrixBiasEn;
        Bitfield<U32,  8, 5> MACCfg_BinaryPoint;
        Bitfield<U32, 14, 1> MACCfg_PostScaleMode;
        Bitfield<U32, 16, 2> MACCfg_NonlinearMode;
    };
    union {
        Bitfield<U32, 0, 16> PostScale_PostScale;
        Bitfield<U32, 16, 5> PostScale_PostRightShift;
    };
};

struct TileDMADst_V11 {
    uint32_t unk00;
    uint32_t RowStride;
    uint32_t PlaneStride;
    uint32_t DepthStride;
    uint32_t GroupStride;
    DMADstFormat_V11 Fmt;
};

struct TD_V11 {
    TDHeader_V11 header;         // 0x000
    U32 unk028[9];               // 0x028
    CommonHeader_V11 common;   // 0x04C
    U32 unk064[4];               // 0x064
    TileDMASrc_V11 tile_dma_src; // 0x074
    U32 unk090;
    L2Config_V11 l2_config;    // 0x094
    U32 unk0B0;
    NEConfig_V11 ne_config;    // 0x0B4
    U32 unk0C0;
    TileDMADst_V11 tile_dma_dst; // 0x0C4
    U32 unk0DC[9];
};

static_assert(sizeof(TDHeader_V11) == 0x28, "unexpected size");
static_assert(sizeof(TileDMASrc_V11) == 0x1C, "unexpected size");
static_assert(sizeof(CommonHeader_V11) == 0x18, "unexpected size");
static_assert(sizeof(L2Config_V11) == 0x1C, "unexpected size");
static_assert(sizeof(NEConfig_V11) == 0x0C, "unexpected size");
static_assert(sizeof(TileDMADst_V11) == 0x18, "unexpected size");

static_assert(offsetof(TD_V11, header) == 0x00, "unexpected offset");
static_assert(offsetof(TD_V11, common) == 0x4C, "unexpected offset");
static_assert(offsetof(TD_V11, tile_dma_src) == 0x74, "unexpected offset");
static_assert(offsetof(TD_V11, l2_config) == 0x94, "unexpected offset");
static_assert(offsetof(TD_V11, ne_config) == 0xB4, "unexpected offset");
static_assert(offsetof(TD_V11, tile_dma_dst) == 0xC4, "unexpected offset");
static_assert(sizeof(TD_V11) == 0x100, "unexpected size");

} // namespace ane

#endif // TD_V11_H_
