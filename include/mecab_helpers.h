/*
 * Furigana4subtitles
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MECAB_HELPERS_H
#define MECAB_HELPERS_H

#include <mecab.h>
#include "types.h"

char *extract_mecab_field(const char *feature, int index);
char *katakana_to_hiragana(const char *in);
FuriganaToken *analyze_text_with_mecab(mecab_t *mecab, const char *line, int *token_count);
void calculate_token_positions(const char *line, FuriganaToken *tokens, int token_count, FontConfig *cfg);

#endif
