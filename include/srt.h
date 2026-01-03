/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - srt.h
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#ifndef JPSUB_SRT_H
#define JPSUB_SRT_H

#include "types.h"

struct subtitle *parse_srt(const char *path, int *count);

#endif