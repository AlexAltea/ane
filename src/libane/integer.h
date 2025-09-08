// SPDX-License-Identifier: MIT
/* Copyright 2025 Alexandro Sanchez Bach <alexandro@phi.nz> */

#ifndef ANE_INTEGER_H_
#define ANE_INTEGER_H_

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
namespace ane {
#endif

// Shorthands
typedef int8_t S8;
typedef int8_t S08;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;

typedef uint8_t U8;
typedef uint8_t U08;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

#ifdef __cplusplus
} // namespace ane
#endif

#endif // !ANE_INTEGER_H_
