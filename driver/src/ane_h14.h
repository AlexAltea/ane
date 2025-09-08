// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2025 Alexandro Sanchez Bach <alexandro@phi.nz> */

#ifndef __ANE_H14_H__
#define __ANE_H14_H__

#include "ane.h"

void ane_h14_tm_enable(struct ane_device *ane);
int ane_h14_tm_enqueue(struct ane_device *ane, struct ane_request *req);
int ane_h14_tm_execute(struct ane_device *ane, struct ane_request *req);

#endif /* __ANE_H14_H__ */
