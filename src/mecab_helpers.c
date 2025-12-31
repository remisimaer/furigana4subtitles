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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "../include/mecab_helpers.h"
#include "../include/utils.h"

char *extract_mecab_field(const char *feature, int index)
{
    if (!feature) return NULL;
    char *copy = strdup(feature);
    if (!copy) return NULL;
    char *tok = strtok(copy, ",");
    for (int i = 0; i < index && tok; i++) tok = strtok(NULL, ",");
    char *res = tok ? strdup(tok) : NULL;
    free(copy);
    return res;
}

char *katakana_to_hiragana(const char *in)
{
    if (!in) return NULL;
    size_t len = strlen(in);
    wchar_t *wbuf = malloc((len + 1) * sizeof(wchar_t));
    if (!wbuf) return NULL;
    mbstowcs(wbuf, in, len + 1);
    for (size_t i = 0; wbuf[i]; i++) {
        if (wbuf[i] >= 0x30A1 && wbuf[i] <= 0x30FA)
            wbuf[i] -= 0x60;
    }
    char *out = malloc(len * 4 + 1);
    wcstombs(out, wbuf, len * 4);
    free(wbuf);
    return out;
}

FuriganaToken *analyze_text_with_mecab(mecab_t *mecab, const char *line, int *token_count)
{
    *token_count = 0;
    const mecab_node_t *node = mecab_sparse_tonode(mecab, line);
    if (!node) return NULL;

    int capacity = INITIAL_TOKEN_CAPACITY;
    FuriganaToken *tokens = malloc(capacity * sizeof(FuriganaToken));

    int char_cursor = 0;
    for (; node; node = node->next) {
        if (node->stat != MECAB_NOR_NODE && node->stat != MECAB_UNK_NODE) continue;

        char surface[256] = {0};
        strncpy(surface, node->surface, node->length);
        int surface_chars = count_unicode_chars(surface);
        if (surface_chars == 0) { char_cursor += 1; continue; }

        int kanji_count = 0;
        mbstate_t st = {0};
        for (size_t i = 0; i < strlen(surface);) {
            wchar_t wc;
            size_t n = mbrtowc(&wc, surface + i, strlen(surface) - i, &st);
            if (n == (size_t)-1 || n == (size_t)-2 || n == 0) break;
            if (is_kanji(wc)) kanji_count++;
            i += n;
        }

        if (kanji_count == 0) { char_cursor += surface_chars; continue; }

        char *reading = extract_mecab_field(node->feature, MECAB_READING_FIELD);
        if (!reading || strcmp(reading, "*") == 0) { free(reading); char_cursor += surface_chars; continue; }

        char *hiragana = katakana_to_hiragana(reading);
        free(reading);
        if (!hiragana) { char_cursor += surface_chars; continue; }

        if (*token_count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, capacity * sizeof(FuriganaToken));
        }

        tokens[*token_count].reading = hiragana;
        tokens[*token_count].start_char = char_cursor;
        tokens[*token_count].char_len = surface_chars;
        tokens[*token_count].x = 0.0f;

        (*token_count)++;
        char_cursor += surface_chars;
    }

    return tokens;
}

void calculate_token_positions(const char *line, FuriganaToken *tokens, int token_count, FontConfig *cfg)
{
    int total_chars = count_unicode_chars(line);
    float start_x = (cfg->screen_w - total_chars * cfg->char_width) / 2.0f;

    for (int i = 0; i < token_count; i++) {
        float center_char = tokens[i].start_char + tokens[i].char_len / 2.0f;
        tokens[i].x = start_x + center_char * cfg->char_width;
    }
}
