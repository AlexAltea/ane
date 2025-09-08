// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#include <asm/types.h>
#include <drm.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <ane_accel.h>
#include "ane.h"
#include "hwx.h"

#ifndef LIBANE_CONFIG_NO_ERR
#include <stdio.h>
#define ane_err(a, ...) fprintf(stderr, "LIBANE: ERR: " a, ##__VA_ARGS__)
#else
#define ane_err(...) \
	do {         \
	} while (0)
#endif

#define TILE_SHIFT	   0xEUL
#define TILE_SIZE	   0x4000UL

#define tile_shift(x)	   (((uint64_t)(x)) << TILE_SHIFT)
#define tile_align(x)	   ((((uint64_t)(x)) + TILE_SIZE - 1) & -TILE_SIZE)
#define tile_size(nn, bdx) (tile_shift(ane_model(nn)->tiles[bdx]))

#define src_bdx(nn, idx)   (4 + ane_dst_count(nn) + idx)
#define dst_bdx(nn, idx)   (4 + idx)

#define MAX_ANE_DEVICES	   2
#define MAX_NODE_LEN	   30
#define MAX_NODE_COUNT	   64

// size 0x25C
struct ane_request_h14 {
	uint32_t unk0;
	uint64_t paddr; // Address must be 64-byte aligned
	// 0xC
	union {
		uint64_t value;
		struct {
			uint32_t value_lo;
			uint32_t value_hi;
		};
	} bar[61]; // 122 hardware BARs
	// 0x1F4
	uint8_t unk1[0x68];
};

static inline void *ane_malloc(const uint64_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		ane_err("failed to malloc size 0x%lx\n", size);
		return NULL;
	}
	return ptr;
}

static inline void *ane_zmalloc(const uint64_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		ane_err("failed to malloc size 0x%lx\n", size);
		return NULL;
	}
	memset(ptr, 0, size);
	return ptr;
}

static inline void *ane_memalign(const uint64_t size)
{
	void *ptr = NULL;
	if (posix_memalign(&ptr, TILE_SIZE, size)) {
		ane_err("failed to memalign size 0x%zx\n", size);
		return NULL;
	}
	return ptr;
}

static inline void *ane_zmemalign(const uint64_t size)
{
	void *ptr = NULL;
	if (posix_memalign(&ptr, TILE_SIZE, size)) {
		ane_err("failed to memalign size 0x%lx\n", size);
		return NULL;
	}
	memset(ptr, 0, size);
	return ptr;
}

static inline void set_nid(void *td, int nid)
{
	uint32_t hdr0 = *(uint32_t *)td;
	hdr0 = (hdr0 & 0xf00ffff) | ((nid & 0xff) << 16);
	memcpy(td, &hdr0, sizeof(uint32_t));
}

static inline void set_btsp_and_command(struct ane_nn *nn)
{
	const struct ane_model *model = ane_model(nn);
	if (!model->hwx) {
		return;
	}

	void *btsp = nn->btsp_chan.map;
	size_t btsp_size = nn->btsp_chan.size;
	if (!btsp || btsp_size == 0) {
		return;
	}

	memset(btsp, 0, btsp_size);

	const struct hwx_section *text = hwx_get_tsk_section(model->hwx);
	if (text) {
		uint64_t copy = model->td_size;
		if (copy == 0 || copy > text->size) {
			copy = text->size;
		}
		if (copy > btsp_size) {
			copy = btsp_size;
		}
		if (copy > 0) {
			if (hwx_section_read(model->hwx, text, 0, btsp, (size_t)copy) != 0) {
				memset(btsp, 0, (size_t)copy);
			}
		}
	}

	set_nid(btsp, ANE_FIFO_NID);
}

static inline int bo_init(struct ane_nn *nn, struct ane_bo *bo)
{
	struct drm_ane_bo_init args = { .size = bo->size };
	int err = ioctl(nn->fd, DRM_IOCTL_ANE_BO_INIT, &args);
	if (err < 0) {
		ane_err("DRM_IOCTL_ANE_BO_INIT failed with 0x%x\n", err);
		return -EINVAL;
	}

	bo->handle = args.handle;
	bo->offset = args.offset;

	return 0;
}

static inline void bo_free(struct ane_nn *nn, struct ane_bo *bo)
{
	if (bo->handle) {
		struct drm_ane_bo_free args = { .handle = bo->handle };
		ioctl(nn->fd, DRM_IOCTL_ANE_BO_FREE, &args);
	}
	bo->handle = 0;
	bo->offset = 0;
}

static inline int bo_mmap(struct ane_nn *nn, struct ane_bo *bo)
{
	bo->map = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, nn->fd,
		       bo->offset);

	if (bo->map == MAP_FAILED) {
		bo->map = NULL;
		ane_err("failed to mmap bo size 0x%lx\n", bo->size);
		return -EINVAL;
	}

	return 0;
}

static inline void bo_munmap(struct ane_nn *nn, struct ane_bo *bo)
{
	(void)nn;
	if (bo->map) {
		munmap(bo->map, bo->size);
	}
	bo->map = NULL;
}

static inline int ane_bo_init(struct ane_nn *nn, struct ane_bo *bo)
{
	int err;

	if (!bo->size)
		return -EINVAL;

	err = bo_init(nn, bo);
	if (err < 0) {
		return err;
	}

	err = bo_mmap(nn, bo);
	if (err < 0) {
		bo_free(nn, bo);
		return err;
	}

	return 0;
}

static inline void ane_bo_free(struct ane_nn *nn, struct ane_bo *bo)
{
	bo_munmap(nn, bo);
	bo_free(nn, bo);
}

static inline void ane_chan_free(struct ane_nn *nn)
{
	ane_bo_free(nn, &nn->btsp_chan);

	for (int bdx = 0; bdx < ANE_MAX_TILE_COUNT; bdx++) {
		ane_bo_free(nn, &nn->chans[bdx]);
	}
}

static inline int ane_chan_init(struct ane_nn *nn)
{
	const struct ane_model *model = ane_model(nn);
	struct ane_bo *bo;
	int err;

	for (int bdx = 0; bdx < ANE_MAX_TILE_COUNT; bdx++) {
		if (model->tiles[bdx]) {
			bo = &nn->chans[bdx];
			bo->size = tile_size(nn, bdx);
			err = ane_bo_init(nn, bo);
			if (err < 0)
				goto error;
		}
	}

	bo = &nn->btsp_chan;
	if (!model->td_size) {
		ane_err("td_size is zero; refusing to init bootstrap channel\n");
		return -EINVAL;
	}
	bo->size = tile_align(model->td_size);
	err = ane_bo_init(nn, bo);
	if (err < 0)
		goto error;

	set_btsp_and_command(nn);

	return 0;

error:
	ane_err("failed to init memory-mapped channels\n");
	ane_chan_free(nn);
	return err;
}

static inline int is_ane_device(int fd)
{
	drm_version_t version = {};
	int err = ioctl(fd, DRM_IOCTL_VERSION, &version);
	if (err < 0) {
		ane_err("failed to get drm version with %d", err);
		return -EINVAL;
	}

	if (!version.name_len) {
		return -EINVAL;
	}

	version.name = (char *)ane_malloc(version.name_len + 1);
	version.date_len = 0;
	version.desc_len = 0;

	err = ioctl(fd, DRM_IOCTL_VERSION, &version);
	if (err < 0) {
		ane_err("failed to get drm version with %d", err);
		free(version.name);
		return -EINVAL;
	}

	/* Results might not be null-terminated strings */
	version.name[version.name_len] = '\0';
	if (strcmp(version.name, "ane") != 0) {
		free(version.name);
		return -EINVAL;
	}

	free(version.name);

	return 0;
}

static inline int open_fd(const char *node)
{
	int fd = open(node, O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		return -ENODEV;
	}

	if (is_ane_device(fd) < 0) {
		close(fd);
		return -EINVAL;
	}

	return fd;
}

static inline int device_open(int dev_id)
{
	int fd;
	char node[MAX_NODE_LEN];
	int found = 0;

	if (dev_id < 0 || dev_id >= MAX_ANE_DEVICES) {
		ane_err("invalid dev_id; 0 <= dev_id <= %d\n",
			MAX_ANE_DEVICES - 1);
		return -EINVAL;
	}

	for (int i = 0; i < MAX_NODE_COUNT; i++) {
		snprintf(node, MAX_NODE_LEN, "/dev/accel/accel%d", i);

		fd = open_fd(node);
		if (fd < 0) {
			continue;
		}

		if (dev_id == found) {
			return fd;
		}

		found++;
		close(fd);
	}

	ane_err("failed to find device with dev_id %d\n", dev_id);
	return -ENODEV;
}

static inline void device_close(int fd)
{
	if (!(fd < 0)) {
		close(fd);
	}
}

static inline int ane_device_open(struct ane_nn *nn, int dev_id)
{
	int fd = device_open(dev_id);
	if (fd < 0) {
		return -EINVAL;
	}

	nn->fd = fd;

	return 0;
}

static inline void ane_device_close(struct ane_nn *nn)
{
	device_close(nn->fd);
	nn->fd = 0;
}

static inline int ane_model_init(struct ane_nn *nn, const char *path)
{
	struct ane_model *model = ane_model(nn);
	memset(model, 0, sizeof(*model));

	struct hwx_file *hwx = hwx_open(path);
	if (!hwx) {
		ane_err("failed to load HWX at %s\n", path);
		return -EINVAL;
	}

	model->hwx = hwx;

	const struct hwx_thread_state *cursor = NULL;
	const struct hwx_thread_state *seg_state = NULL;
	while ((cursor = hwx_thread_state_next(hwx, HWX_ANE_SEG_STATE, cursor)) != NULL) {
		if (cursor->data && cursor->byte_size >= sizeof(struct hwx_ane_seg_state)) {
			seg_state = cursor;
			break;
		}
	}

	if (!seg_state) {
		ane_err("HWX missing SEG_STATE thread state\n");
		hwx_close(hwx);
		model->hwx = NULL;
		return -EINVAL;
	}

	struct hwx_ane_seg_state seg_meta;
	memset(&seg_meta, 0, sizeof(seg_meta));
	memcpy(&seg_meta, seg_state->data, sizeof(seg_meta));
	model->td_size = (uint64_t)seg_meta.seg_words * 4u;
	model->td_count = seg_meta.td_count;
	model->size = model->td_size;
	if (!model->td_size) {
		ane_err("SEG_STATE reported zero-sized segment\n");
		goto err;
	}

	const struct hwx_section *tsk_section = hwx_get_tsk_section(hwx);
	const struct hwx_section *krn_section = hwx_get_krn_section(hwx);
	if (!tsk_section || !krn_section) {
		ane_err("missing tsk or krn section\n");
		goto err;
	}
	model->tsk_size = tsk_section->size;
	model->krn_size = krn_section->size;
	return 0;

err:
	hwx_close(hwx);
	model->hwx = NULL;
	return -EINVAL;
}

static inline void ane_model_free(struct ane_nn *nn)
{
	struct ane_model *model = ane_model(nn);
	if (model->hwx) {
		hwx_close(model->hwx);
		model->hwx = NULL;
	}
}

struct ane_nn *__ane_init(const char *path, int dev_id)
{
	struct ane_nn *nn = ane_zmalloc(sizeof(struct ane_nn));
	if (!nn) {
		return NULL;
	}

	if (ane_model_init(nn, path) < 0) {
		ane_err("failed to load HWX from %s\n", path);
		free(nn);
		return NULL;
	}

	if (ane_device_open(nn, dev_id) < 0) {
		ane_err("failed to open device with dev_id %d\n", dev_id);
		ane_model_free(nn);
		free(nn);
		return NULL;
	}

	if (ane_chan_init(nn) < 0) {
		ane_err("failed to init memory-mapped chans\n");
		ane_device_close(nn);
		ane_model_free(nn);
		free(nn);
		return NULL;
	}

	return nn;
}

void __ane_free(struct ane_nn *nn)
{
	ane_chan_free(nn);
	ane_device_close(nn);
	ane_model_free(nn);
	free(nn);
}

int ane_exec(struct ane_nn *nn)
{
	const struct ane_model *model = ane_model(nn);

	struct drm_ane_submit args;
	memset(&args, 0, sizeof(args));

	args.tsk_size = model->tsk_size;
	args.td_count = model->td_count;
	args.td_size = model->td_size;

	for (int bdx = 0; bdx < ANE_MAX_TILE_COUNT; bdx++) {
		if (true) { // model->tiles[bdx]
			args.handles[bdx] = nn->chans[bdx].handle;
		}
	}
	args.btsp_handle = nn->btsp_chan.handle;

	return 0;//ioctl(nn->fd, DRM_IOCTL_ANE_SUBMIT, &args);
}

#ifndef LIBANE_CONFIG_NO_INDEX_CHECK
#define INDEX_CHECK(cnt, idx, ret)                                                  \
	do {                                                                           \
		if ((idx) >= (cnt)) {                                                   \
			ane_err("tried to index %d but max is %d; bailing.\n", idx, \
				cnt);                                                      \
			return ret;                                                         \
		}                                                                       \
	} while (0)
#else
#define INDEX_CHECK(cnt, idx, ret) \
	do {                            \
	} while (0)
#endif /* LIBANE_CONFIG_NO_INDEX_CHECK */

uint64_t __ane_src_size(struct ane_nn *nn, const uint32_t idx)
{
	INDEX_CHECK(ane_src_count(nn), idx, 0);
	return tile_size(nn, src_bdx(nn, idx));
}

uint64_t __ane_dst_size(struct ane_nn *nn, const uint32_t idx)
{
	INDEX_CHECK(ane_dst_count(nn), idx, 0);
	return tile_size(nn, dst_bdx(nn, idx));
}

void __ane_send(struct ane_nn *nn, void *from, const uint32_t idx)
{
	INDEX_CHECK(ane_src_count(nn), idx, );
	memcpy(nn->chans[src_bdx(nn, idx)].map, from,
	       tile_size(nn, src_bdx(nn, idx)));
}

void __ane_read(struct ane_nn *nn, void *to, const uint32_t idx)
{
	INDEX_CHECK(ane_dst_count(nn), idx, );
	memcpy(to, nn->chans[dst_bdx(nn, idx)].map,
	       tile_size(nn, dst_bdx(nn, idx)));
}

// clang-format off
void ane_tile(void *data, void *tile, const uint64_t N, const uint64_t C,
	      const uint64_t H, const uint64_t W, const uint64_t P,
	      const uint64_t R)
{
	const uint64_t new_H = P / R;
	const uint64_t new_W = R / sizeof(uint16_t);
	const uint64_t stride = W * sizeof(uint16_t);

	const uint64_t C0 = C * H * W;
	const uint64_t C1 = C * new_H * new_W;
	const uint64_t H0 = H * W;
	const uint64_t H1 = new_H * new_W;

	if ((new_H == H) && (new_W == W)) {
		memcpy(tile, data, N * C * P);
		return;
	}

	memset(tile, 0, N * C * P);

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				void *src = ((void *)(data)) + ((n * C0 + c * H0 + h * W) * sizeof(uint16_t));
				void *dst = ((void *)(tile)) + ((n * C1 + c * H1 + h * new_W) * sizeof(uint16_t));
				memcpy(dst, src, stride);
			}
		}
	}
	return;
}

void ane_untile(void *data, void *tile, const uint64_t N, const uint64_t C,
		const uint64_t H, const uint64_t W, const uint64_t P,
		const uint64_t R)
{
	const uint64_t new_H = P / R;
	const uint64_t new_W = R / sizeof(uint16_t);
	const uint64_t stride = W * sizeof(uint16_t);

	const uint64_t C0 = C * H * W;
	const uint64_t C1 = C * new_H * new_W;
	const uint64_t H0 = H * W;
	const uint64_t H1 = new_H * new_W;

	if ((new_H == H) && (new_W == W)) {
		memcpy(data, tile, N * C * H * W * sizeof(uint16_t));
		return;
	}

	memset(data, 0, N * C * H * W * sizeof(uint16_t));

	for (uint64_t n = 0; n < N; n++) {
		for (uint64_t c = 0; c < C; c++) {
			for (uint64_t h = 0; h < H; h++) {
				void *src = ((void *)(tile)) + ((n * C1 + c * H1 + h * new_W) * sizeof(uint16_t));
				void *dst = ((void *)(data)) + ((n * C0 + c * H0 + h * W) * sizeof(uint16_t));
				memcpy(dst, src, stride);
			}
		}
	}
	return;
}
// clang-format on

static inline void ___ane_tile_send(struct ane_nn *nn, void *from,
				    const uint32_t idx)
{
	const struct ane_model *model = ane_model(nn);
	const int bdx = src_bdx(nn, idx);
	ane_tile(from, nn->chans[bdx].map, model->nchw[bdx][0],
		 model->nchw[bdx][1], model->nchw[bdx][2], model->nchw[bdx][3],
		 model->nchw[bdx][4], model->nchw[bdx][5]);
}

static inline void ___ane_tile_read(struct ane_nn *nn, void *to,
				    const uint32_t idx)
{
	const struct ane_model *model = ane_model(nn);
	const int bdx = dst_bdx(nn, idx);
	ane_untile(to, nn->chans[bdx].map, model->nchw[bdx][0],
		   model->nchw[bdx][1], model->nchw[bdx][2], model->nchw[bdx][3],
		   model->nchw[bdx][4], model->nchw[bdx][5]);
}

void __ane_tile_send(struct ane_nn *nn, void *from, const uint32_t idx)
{
	INDEX_CHECK(ane_src_count(nn), idx, );
	___ane_tile_send(nn, from, idx);
}

void __ane_tile_read(struct ane_nn *nn, void *to, const uint32_t idx)
{
	INDEX_CHECK(ane_dst_count(nn), idx, );
	___ane_tile_read(nn, to, idx);
}
