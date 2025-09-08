// SPDX-License-Identifier: MIT
/* Copyright 2025 Alexandro Sanchez Bach <alexandro@phi.nz> */

#ifndef TD_V5_H_
#define TD_V5_H_

#include <libane/integer.h>

#include "bitfield.h"

namespace ane {

struct TDHeader_V5 {
    union {
        Bitfield<U32, 0, 16> tid;
        Bitfield<U32, 16, 8> nid;
        Bitfield<U32, 24, 1> lnid;
        Bitfield<U32, 25, 1> eon;
    };
    union {
        Bitfield<U32, 0, 16> exe_cycles;
        Bitfield<U32, 16, 9> next_size;
    };
    union {
        Bitfield<U32, 0, 24> log_events;
    };
    union {
        Bitfield<U32, 0, 24> exceptions;
    };
    union {
        Bitfield<U32, 0, 24> debug_log_events;
    };
    union {
        Bitfield<U32, 0, 24> debug_exceptions;
    };
    union {
        Bitfield<U32,  8, 1> disallow_abort;
        Bitfield<U32,  9, 1> td_skip;
        Bitfield<U32, 10, 1> kpc;
        Bitfield<U32, 11, 1> spl;
        Bitfield<U32, 12, 1> tsr;
        Bitfield<U32, 13, 1> spc;
        Bitfield<U32, 14, 1> dpc;
        Bitfield<U32, 15, 1> tse;
        Bitfield<U32, 16, 6> next_priority;
        Bitfield<U32, 24, 1> tde;
        Bitfield<U32, 28, 1> src_loc;
        Bitfield<U32, 29, 1> dst_loc;
        Bitfield<U32, 31, 1> tq_dis;
    };
    union {
        Bitfield<U32, 0, 32> next_pointer;
    };
    union {
        Bitfield<U32,  0, 5> rbase0;
        Bitfield<U32,  5, 1> rbe0;
        Bitfield<U32,  6, 5> rbase1;
        Bitfield<U32, 11, 1> rbe1;
        Bitfield<U32, 12, 5> wbase;
        Bitfield<U32, 17, 1> wbe;
        Bitfield<U32, 18, 5> tbase;
        Bitfield<U32, 23, 1> tbe;
        Bitfield<U32, 24, 3> ene;
    };
    union {
        Bitfield<U32,  0, 5> kbase0;
        Bitfield<U32,  5, 1> kbe0;
        Bitfield<U32,  6, 5> kbase1;
        Bitfield<U32, 11, 1> kbe1;
        Bitfield<U32, 12, 5> kbase2;
        Bitfield<U32, 17, 1> kbe2;
        Bitfield<U32, 18, 5> kbase3;
        Bitfield<U32, 23, 1> kbe3;
    };
    uint32_t header10;
};

union CoeffDMAConfig_V5 {
    Bitfield<U32,  0, 1> en;
    Bitfield<U32,  4, 2> cr_h;
    Bitfield<U32,  6, 4> cache_hint;
    Bitfield<U32, 28, 1> prefetch_participate_en;
};

struct KernelDMASrc_V5 {
    CoeffDMAConfig_V5 coeff_dma_config[16];
    uint32_t coeff_addr[16];
    uint32_t coeff_size[16];
};

struct CommonHeader_V5 {
    union {
        Bitfield<U32,  0, 15> InDim_Win;
        Bitfield<U32, 16, 15> InDim_Hin;
    };
    U32 unk004;
    union {
        Bitfield<U32, 0, 2> ChCfg_InFmt;
        Bitfield<U32, 4, 2> ChCfg_OutFmt;
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
    U32 unk018;
    union {
        Bitfield<U32,  0, 5> ConvCfg_Kw;
        Bitfield<U32,  5, 5> ConvCfg_Kh;
        Bitfield<U32, 10, 3> ConvCfg_OCGSize;
        Bitfield<U32, 13, 2> ConvCfg_Sx;
        Bitfield<U32, 15, 2> ConvCfg_Sy;
        Bitfield<U32, 17, 5> ConvCfg_Px;
        Bitfield<U32, 22, 5> ConvCfg_Py;
        Bitfield<U32, 28, 2> ConvCfg_Ox;
        Bitfield<U32, 30, 2> ConvCfg_Oy;
    };
    U32 unk020;
    union {
        Bitfield<U32,  0, 13> GroupConvCfg_NumGroups;
        Bitfield<U32, 14, 1> GroupConvCfg_UnicastEn;
        Bitfield<U32, 15, 1> GroupConvCfg_ElemMultMode;
        Bitfield<U32, 16, 16> GroupConvCfg_UnicastCin;
    };
    union {
        Bitfield<U32, 0, 15> TileCfg_TileHeight;
    };
    U32 unk02C;
    U32 unk030;
    union {
        Bitfield<U32,  2, 1> Cfg_SmallSourceMode;
        Bitfield<U32,  8, 3> Cfg_ShPref;
        Bitfield<U32, 12, 3> Cfg_ShMin;
        Bitfield<U32, 16, 3> Cfg_ShMax;
        Bitfield<U32, 19, 3> Cfg_ActiveNE;
        Bitfield<U32, 22, 1> Cfg_ContextSwitchIn;
        Bitfield<U32, 24, 1> Cfg_ContextSwitchOut;
        Bitfield<U32, 26, 1> Cfg_AccDoubleBufEn;
    };
    union {
        Bitfield<U32,  0, 16> TaskInfo_TaskID;
        Bitfield<U32, 16, 4> TaskInfo_TaskQ;
        Bitfield<U32, 20, 8> TaskInfo_NID;
    };
    union {
        Bitfield<U32, 0, 4> DPE_Category;
    };
};

union DMASrcConfig_V5 {
    Bitfield<U32,  0, 1> en;
    Bitfield<U32,  4, 2> cr_h;
    Bitfield<U32,  6, 4> cache_hint;
    Bitfield<U32, 10, 4> cache_hint_reuse;
    Bitfield<U32, 14, 4> cache_hint_noreuse;
    Bitfield<U32, 18, 2> dependency_mode;
};

union DMASrcFormat_V5 {
    Bitfield<U32,  0, 2> FmtMode;
    Bitfield<U32,  4, 2> Truncate;
    Bitfield<U32,  8, 1> Shift;
    Bitfield<U32, 12, 2> MemFmt;
    Bitfield<U32, 16, 3> OffsetCh;
    Bitfield<U32, 24, 4> Interleave;
    Bitfield<U32, 28, 4> CmpVec;
};

struct TileDMASrc_V5 {
    DMASrcConfig_V5 DMAConfig;
    uint32_t unk04;
    uint32_t BaseAddr;
    uint32_t RowStride;
    uint32_t PlaneStride;
    uint32_t DepthStride;
    uint32_t GroupStride;
    uint32_t unk1[7];
    DMASrcFormat_V5 Fmt;
    uint32_t unk2[5];
    uint32_t PixelOffset[4];
};

struct L2Config_V5 {
    union {
        Bitfield<U32, 0, 1> L2Cfg_InputReLU;
        Bitfield<U32, 2, 2> L2Cfg_PaddingMode;
    };
    union {
        Bitfield<U32,  0, 2> SourceCfg_SourceType;
        Bitfield<U32,  2, 2> SourceCfg_Dependent;
        Bitfield<U32,  4, 1> SourceCfg_AliasConvSrc;
        Bitfield<U32,  5, 1> SourceCfg_AliasConvRslt;
        Bitfield<U32,  6, 2> SourceCfg_DMAFmt;
        Bitfield<U32,  8, 4> SourceCfg_DMAInterleave;
        Bitfield<U32, 12, 4> SourceCfg_DMACmpVec;
        Bitfield<U32, 16, 3> SourceCfg_DMAOffsetCh;
        Bitfield<U32, 20, 1> SourceCfg_AliasPlanarSrc; // not in v7?
        Bitfield<U32, 22, 1> SourceCfg_AliasPlanarRslt; // not in v7?
    };
    union {
        Bitfield<U32, 4, 17> SourceBase_Addr;
    };
    union {
        Bitfield<U32, 4, 17> SourceChannelStride_Stride;
    };
    union {
        Bitfield<U32, 4, 17> SourceRowStride_Stride;
    };
    U32 unk_maybe_stride1;
    U32 unk_maybe_stride2;
    U32 unk01C;
    U32 unk020;
    U32 unk024;
    U32 unk028;
    U32 unk02C;
    union {
        Bitfield<U32,  0, 2> ResultCfg_ResultType;
        Bitfield<U32,  3, 1> ResultCfg_L2BfrMode;
        Bitfield<U32,  4, 1> ResultCfg_AliasConvSrc;
        Bitfield<U32,  5, 1> ResultCfg_AliasConvRslt;
        Bitfield<U32,  6, 2> ResultCfg_DMAFmt;
        Bitfield<U32,  8, 4> ResultCfg_DMAInterleave;
        Bitfield<U32, 12, 4> ResultCfg_DMACmpVec;
        Bitfield<U32, 16, 3> ResultCfg_DMAOffsetCh;
        Bitfield<U32, 20, 1> ResultCfg_AliasPlanarSrc; // not in v7?
        Bitfield<U32, 22, 1> ResultCfg_AliasPlanarRslt; // not in v7?
    };
    union {
        Bitfield<U32, 4, 17> ResultBase_Addr;
    };
    union {
        Bitfield<U32, 4, 17> ConvResultChannelStride_Stride;
    };
    union {
        Bitfield<U32, 4, 17> ConvResultRowStride_Stride;
    };
};

struct NEConfig_V5 {
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
        Bitfield<U32, 0, 16> MatrixVectorBias_MatrixVectorBias;
    };
    union {
        Bitfield<U32, 0, 16> AccBias_AccBias;
        Bitfield<U32, 16, 5> AccBias_AccBiasShift;
    };
    union {
        Bitfield<U32, 0, 16> PostScale_PostScale;
        Bitfield<U32, 16, 5> PostScale_PostRightShift;
    };
};

union DMADstConfig_V5 {
    Bitfield<U32,  0, 1> en;
    Bitfield<U32,  4, 2> cr_h;
    Bitfield<U32,  6, 4> cache_hint;
    Bitfield<U32, 26, 1> l2_bfr_mode;
    Bitfield<U32, 27, 1> bypass_eow;
};

union DMADstFormat_V5 {
    Bitfield<U32,   0, 2> FmtMode;
    Bitfield<U32,   4, 2> Truncate;
    Bitfield<U32,   8, 1> Shift;
    Bitfield<U32,  12, 2> MemFmt;
    Bitfield<U32,  16, 3> OffsetCh;
    Bitfield<U32,  20, 1> ZeroPadLast;
    Bitfield<U32,  21, 1> ZeroPadFirst;
    Bitfield<U32,  22, 1> CmpVecFill;
    Bitfield<U32,  24, 4> Interleave;
    Bitfield<U32,  28, 4> CmpVec;
};

struct TileDMADst_V5 {
    DMADstConfig_V5 DMAConfig;
    uint32_t BaseAddr;
    uint32_t RowStride;
    uint32_t PlaneStride;
    uint32_t DepthStride;
    uint32_t GroupStride;
    DMADstFormat_V5 Fmt;
};

struct TD_V5 {
    TDHeader_V5 header;
    U32 unk30;
    U32 unk34;
    KernelDMASrc_V5 kernel_dma_src;
    U32 unk1F4[13];
    CommonHeader_V5 common;
    U32 unk168;
    TileDMASrc_V5 tile_dma_src;
    U32 unk1CC[5];
    L2Config_V5 l2_config;
    U32 unk220[8];
    NEConfig_V5 ne_config;
    U32 unk254;
    TileDMADst_V5 tile_dma_dst;
};

static_assert(sizeof(TDHeader_V5) == 0x2C, "unexpected size");
static_assert(sizeof(KernelDMASrc_V5) == 0xC0, "unexpected size");
static_assert(sizeof(CommonHeader_V5) == 0x40, "unexpected size");
static_assert(sizeof(TileDMASrc_V5) == 0x60, "unexpected size");
static_assert(sizeof(L2Config_V5) == 0x40, "unexpected size");
static_assert(sizeof(NEConfig_V5) == 0x14, "unexpected size");
static_assert(sizeof(TileDMADst_V5) == 0x1C, "unexpected size");
static_assert(sizeof(TD_V5) == 0x274, "unexpected size");

static_assert(offsetof(TD_V5, header) == 0x0, "unexpected offset");
static_assert(offsetof(TD_V5, kernel_dma_src) == 0x34, "unexpected offset");
static_assert(offsetof(TD_V5, common) == 0x128, "unexpected offset");
static_assert(offsetof(TD_V5, tile_dma_src) == 0x16C, "unexpected offset");
static_assert(offsetof(TD_V5, l2_config) == 0x1e0, "unexpected offset");
static_assert(offsetof(TD_V5, ne_config) == 0x240, "unexpected offset");
static_assert(offsetof(TD_V5, tile_dma_dst) == 0x258, "unexpected offset");

} // namespace ane

#endif // TD_V5_H_
