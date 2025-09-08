// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H__
#define __ANE_H__

#if defined(__cplusplus)
extern "C" {
#endif

/*
// Asahi Neural Engine (ANE) Userspace Library
//
// USAGE:

	Simply drop "ane.h" with libane.a (generated from ane.c).

	#include "ane.h" // only requires <stdint.h>

	int main(void)
	{
		struct ane_nn *nn = ane_init("model.anec"); // init network from anec or hwx format
		if (!nn) {
			printf("failed to load model\n");
			return -1;
		}

		void *input0 = malloc(ane_src_size(nn, 0)); // get size of 0th index input
		ane_send(nn, input0, 0); // send 0th index input to device
		ane_send(nn, input1, 1); // send 1st index input to device

		if (ane_exec(nn) < 0) { // call exec after sending all inputs
			printf("execution failed\n");
		}

		ane_read(nn, output0, 0); // receive 0th index output from device

		ane_free(nn); // inverse of ane_init()
		return 0;
	}

	Compiles with gcc or g++:

	gcc -I/usr/include/libane main.c /usr/lib/libane.a  # or -lane

*/

#include <stdint.h>

#define TILE_COUNT 0x61 // 0x20

struct hwx_file;

struct ane_model {
	uint64_t size;
	uint32_t td_size;
	uint32_t td_count;
	uint64_t tsk_size;
	uint64_t krn_size;
	uint32_t src_count;
	uint32_t dst_count;
	uint32_t tiles[TILE_COUNT];
	uint64_t nchw[TILE_COUNT][6];
	struct hwx_file *hwx;
};

struct ane_bo {
	void *map; /* mmap-ed CPU virtual address */
	uint64_t size; /* size of mmap region */
	uint32_t handle; /* drm gem handle */
	uint64_t offset; /* drm gem fake offset for mmap */
};

struct ane_nn {
	int fd; /* file descriptor to accel node (index dev_id) */
	struct ane_model model; /* ane model metadata */
	struct ane_bo chans[TILE_COUNT]; /* mmap-ed tile channels */
	struct ane_bo btsp_chan; /* mmap-ed bootstrap channel */
};

/* #define LIBANE_CONFIG_NO_ERR */
/* #define LIBANE_CONFIG_NO_INDEX_CHECK */
/* #define LIBANE_CONFIG_NO_STATIC_ASSERT */

#define LIBANE_ASSERT_TILE_INDEX(idx) \
	do {                              \
	} while (0)

struct ane_nn *__ane_init(const char *path, int dev_id);
static inline struct ane_nn *ane_init(const char *path)
{
	return __ane_init(path, 0);
}

void __ane_free(struct ane_nn *nn);
static inline void ane_free(struct ane_nn *nn)
{
	__ane_free(nn);
}

int ane_exec(struct ane_nn *nn);

#define ane_model(nn)	  (&(nn)->model)
#define ane_src_count(nn) (ane_model(nn)->src_count)
#define ane_dst_count(nn) (ane_model(nn)->dst_count)

uint64_t __ane_src_size(struct ane_nn *nn, const uint32_t idx);
uint64_t __ane_dst_size(struct ane_nn *nn, const uint32_t idx);
static inline uint64_t ane_src_size(struct ane_nn *nn, const uint32_t idx)
{
	LIBANE_ASSERT_TILE_INDEX(idx);
	return __ane_src_size(nn, idx);
}

static inline uint64_t ane_dst_size(struct ane_nn *nn, const uint32_t idx)
{
	LIBANE_ASSERT_TILE_INDEX(idx);
	return __ane_dst_size(nn, idx);
}

void __ane_send(struct ane_nn *nn, void *from, const uint32_t idx);
void __ane_read(struct ane_nn *nn, void *to, const uint32_t idx);
static inline void ane_send(struct ane_nn *nn, void *from, const uint32_t idx)
{
	LIBANE_ASSERT_TILE_INDEX(idx);
	__ane_send(nn, from, idx);
}

static inline void ane_read(struct ane_nn *nn, void *to, const uint32_t idx)
{
	LIBANE_ASSERT_TILE_INDEX(idx);
	__ane_read(nn, to, idx);
}

void __ane_tile_send(struct ane_nn *nn, void *from, const uint32_t idx);
void __ane_tile_read(struct ane_nn *nn, void *to, const uint32_t idx);
static inline void ane_tile_send(struct ane_nn *nn, void *from,
			       const uint32_t idx)
{
	LIBANE_ASSERT_TILE_INDEX(idx);
	__ane_tile_send(nn, from, idx);
}

static inline void ane_tile_read(struct ane_nn *nn, void *to, const uint32_t idx)
{
	LIBANE_ASSERT_TILE_INDEX(idx);
	__ane_tile_read(nn, to, idx);
}

void ane_tile(void *data, void *tile, const uint64_t N, const uint64_t C,
	      const uint64_t H, const uint64_t W, const uint64_t P,
	      const uint64_t R);
void ane_untile(void *data, void *tile, const uint64_t N, const uint64_t C,
		const uint64_t H, const uint64_t W, const uint64_t P,
		const uint64_t R);

#if defined(__cplusplus)
}
#endif

#endif /* __ANE_H__ */
