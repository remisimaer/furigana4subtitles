/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - types.h
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#ifndef JPSUB_TYPES_H
#define JPSUB_TYPES_H

#include <wchar.h>

/*
 * Buffer and capacity constants
 */
#define INITIAL_SUB_CAPACITY	128
#define INITIAL_TOKEN_CAPACITY	64
#define MAX_LINE		2048
#define JPSUB_MAX_PATH		512
#define MAX_TIME		32

/*
 * Time conversion constants
 */
#define MS_PER_HOUR		3600000
#define MS_PER_MINUTE		60000
#define MS_PER_SECOND		1000

/*
 * MeCab field indices
 */
#define MECAB_READING_FIELD	7

/*
 * Furigana token - represents a single furigana annotation
 * positioned above kanji characters.
 */
struct furigana_token {
	char *reading;		/* hiragana reading */
	int start_char;		/* start character position */
	int char_len;		/* character length */
	float x;		/* x position for rendering */
};

/*
 * Subtitle entry - a single subtitle with timing and text
 */
struct subtitle {
	int start_ms;
	int end_ms;
	char *text;
};

/*
 * Font configuration for ASS output
 */
struct font_config {
	const char *font_name;
	int main_size;
	int furigana_size;
	int screen_w;
	int screen_h;
	int baseline_y;
	int furigana_offset;
	float char_width;
	int line_spacing;
};

#endif
