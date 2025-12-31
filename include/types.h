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

#ifndef TYPES_H
#define TYPES_H

#include <wchar.h>

/* constants */
#define INITIAL_SUB_CAPACITY 128
#define INITIAL_TOKEN_CAPACITY 64
#define MAX_LINE 2048
#define MAX_PATH 512
#define MAX_TIME 32

#define MS_PER_HOUR   3600000
#define MS_PER_MINUTE 60000
#define MS_PER_SECOND 1000

#define MECAB_READING_FIELD 7

typedef struct {
    char *reading;
    int start_char;
    int char_len;
    float x;
} FuriganaToken;

typedef struct {
    int start_ms;
    int end_ms;
    char *text;
} Subtitle;

typedef struct {
    const char *font_name;
    int main_size;
    int furigana_size;
    int screen_w;
    int screen_h;
    int baseline_y;
    int furigana_offset;
    float char_width;
    int line_spacing;
} FontConfig;

#endif
