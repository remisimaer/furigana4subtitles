/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - mecab_helpers.h
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#ifndef JPSUB_MECAB_HELPERS_H
#define JPSUB_MECAB_HELPERS_H

#include <mecab.h>
#include "types.h"

char *extract_mecab_field(const char *feature, int index);
char *katakana_to_hiragana(const char *in);

struct furigana_token *analyze_text_with_mecab(mecab_t *mecab, const char *line,
					       int *token_count);
void calculate_token_positions(const char *line, struct furigana_token *tokens,
			       int token_count, struct font_config *cfg);

#endif
