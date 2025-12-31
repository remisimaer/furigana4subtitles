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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTILS_H
#define UTILS_H

#include <wchar.h>
#include <mecab.h>
#include <types.h>

int count_unicode_chars(const char *s);
void format_ass_time(int ms, char *buf);
int is_kanji(wchar_t c);

int ends_with_srt(const char *path);
void process_file(const char *path, FontConfig *cfg, mecab_t *mecab);
void scan_directory(const char *dir, FontConfig *cfg, mecab_t *mecab);

#endif
