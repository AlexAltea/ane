// SPDX-License-Identifier: MIT
/* Copyright 2025 Alexandro Sanchez Bach <alexandro@phi.nz> */

#ifndef TD_V7_H_
#define TD_V7_H_

#include <libane/integer.h>

#include "bitfield.h"

namespace ane {

struct TDHeader_V7 {
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
    union {
        Bitfield<U32, 0, 16> dtid; // only if tde==1
    };
};

struct TDCoeffDMAConfig {
    Bitfield<U32,  0, 1> en;
    Bitfield<U32,  4, 2> cr_h;
    Bitfield<U32,  6, 4> cache_hint;
    Bitfield<U32, 28, 1> prefetch_participate_en;
};

struct KernelDMASrc {
    TDCoeffDMAConfig coeff_dma_config[16];
    uint32_t coeff_addr[16];
    uint32_t coeff_size[16];
};

struct TD_V7 {
    TDHeader_V7 header;
    U32 unk30;
    U32 unk34;
    KernelDMASrc kernel_dma_src;
};

static_assert(sizeof(TDHeader_V7) == 0x2C, "unexpected size");
static_assert(sizeof(KernelDMASrc_V5) == 0xC0, "unexpected size");
static_assert(sizeof(CommonHeader_V5) == 0x40, "unexpected size");
static_assert(sizeof(TileDMASrc_V5) == 0x60, "unexpected size");
static_assert(sizeof(L2Config_V5) == 0x40, "unexpected size");
static_assert(sizeof(NEConfig_V5) == 0x14, "unexpected size");
static_assert(sizeof(TileDMADst_V5) == 0x1C, "unexpected size");


static_assert(offsetof(TD_V5, header) == 0x0, "unexpected offset");
static_assert(offsetof(TD_V5, kernel_dma_src) == 0x34, "unexpected offset");
static_assert(offsetof(TD_V5, common) == 0x128, "unexpected offset");
static_assert(offsetof(TD_V5, tile_dma_src) == 0x16C, "unexpected offset");
static_assert(offsetof(TD_V5, l2_config) == 0x1e0, "unexpected offset");
static_assert(offsetof(TD_V5, ne_config) == 0x240, "unexpected offset");
static_assert(offsetof(TD_V5, tile_dma_dst) == 0x258, "unexpected offset");

} // namespace ane

#endif // TD_V7_H_
