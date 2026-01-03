/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - utils.h
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#ifndef JPSUB_UTILS_H
#define JPSUB_UTILS_H

#include <wchar.h>
#include <mecab.h>
#include "types.h"

/* Unicode helpers */
int count_unicode_chars(const char *s);
int is_kanji(wchar_t c);

/* Time formatting */
void format_ass_time(int ms, char *buf);

/* File operations */
int ends_with_srt(const char *path);
void process_file(const char *path, struct font_config *cfg, mecab_t *mecab);
void scan_directory(const char *dir, struct font_config *cfg, mecab_t *mecab);

/* Configuration */
struct font_config *get_default_config(void);
struct font_config *create_scaled_config(int main_size);

/* UI */
void print_banner(void);

#endif
