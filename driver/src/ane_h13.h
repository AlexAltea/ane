// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_H13_H__
#define __ANE_H13_H__

#include "ane.h"

#define ANE_H13_TILE_COUNT 0x20

void ane_h13_tm_enable(struct ane_device *ane);
int ane_h13_tm_enqueue(struct ane_device *ane, struct ane_request *req);
int ane_h13_tm_execute(struct ane_device *ane, struct ane_request *req);

#endif /* __ANE_H13_H__ */
