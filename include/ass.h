/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - ass.h
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#ifndef JPSUB_ASS_H
#define JPSUB_ASS_H

#include <mecab.h>
#include "types.h"

void generate_ass(const char *input, struct subtitle *subs, int count,
		  struct font_config *cfg, mecab_t *mecab);

#endif
