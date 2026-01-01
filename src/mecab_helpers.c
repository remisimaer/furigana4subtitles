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
    if (mbstowcs(wbuf, in, len + 1) == (size_t)-1) {
        free(wbuf);
        return NULL;
    }
    for (size_t i = 0; wbuf[i]; i++) {
        if (wbuf[i] >= 0x30A1 && wbuf[i] <= 0x30FA)
            wbuf[i] -= 0x60;
    }
    char *out = malloc(len * 4 + 1);
    if (!out) {
        free(wbuf);
        return NULL;
    }
    if (wcstombs(out, wbuf, len * 4 + 1) == (size_t)-1) {
        free(wbuf);
        free(out);
        return NULL;
    }
    free(wbuf);
    return out;
}

static int is_hiragana(wchar_t c)
{
    return (c >= 0x3040 && c <= 0x309F);
}

/* Extract kanji-only reading by stripping matching hiragana from surface */
static char *extract_kanji_reading(const char *surface, const char *full_reading)
{
    if (!surface || !full_reading) return NULL;

    size_t surf_len = strlen(surface);
    size_t read_len = strlen(full_reading);

    /* Convert to wide chars for easier manipulation */
    wchar_t *wsurf = malloc((surf_len + 1) * sizeof(wchar_t));
    wchar_t *wread = malloc((read_len + 1) * sizeof(wchar_t));
    if (!wsurf || !wread) { free(wsurf); free(wread); return NULL; }

    if (mbstowcs(wsurf, surface, surf_len + 1) == (size_t)-1 ||
        mbstowcs(wread, full_reading, read_len + 1) == (size_t)-1) {
        free(wsurf); free(wread);
        return NULL;
    }

    size_t wsurf_len = wcslen(wsurf);
    size_t wread_len = wcslen(wread);

    /* Strip matching hiragana from the beginning */
    size_t prefix_skip = 0;
    while (prefix_skip < wsurf_len && prefix_skip < wread_len &&
           is_hiragana(wsurf[prefix_skip]) && wsurf[prefix_skip] == wread[prefix_skip]) {
        prefix_skip++;
    }

    /* Strip matching hiragana from the end */
    size_t suffix_skip = 0;
    while (suffix_skip < (wsurf_len - prefix_skip) && suffix_skip < (wread_len - prefix_skip) &&
           is_hiragana(wsurf[wsurf_len - 1 - suffix_skip]) &&
           wsurf[wsurf_len - 1 - suffix_skip] == wread[wread_len - 1 - suffix_skip]) {
        suffix_skip++;
    }

    /* Extract the middle part of reading (corresponds to kanji) */
    size_t start = prefix_skip;
    size_t end = wread_len - suffix_skip;
    if (start >= end) {
        free(wsurf); free(wread);
        return strdup(full_reading); /* fallback to full reading */
    }

    size_t new_len = end - start;
    wchar_t *wresult = malloc((new_len + 1) * sizeof(wchar_t));
    if (!wresult) { free(wsurf); free(wread); return NULL; }

    wcsncpy(wresult, wread + start, new_len);
    wresult[new_len] = L'\0';

    char *result = malloc(new_len * 4 + 1);
    if (!result) { free(wsurf); free(wread); free(wresult); return NULL; }

    if (wcstombs(result, wresult, new_len * 4 + 1) == (size_t)-1) {
        free(wsurf); free(wread); free(wresult); free(result);
        return NULL;
    }

    free(wsurf); free(wread); free(wresult);
    return result;
}

/* Find the kanji span (start and length) in a surface string */
static void find_kanji_span(const char *surface, int *kanji_start, int *kanji_len)
{
    *kanji_start = -1;
    *kanji_len = 0;

    mbstate_t st = {0};
    int char_idx = 0;
    int first_kanji = -1;
    int last_kanji = -1;

    for (size_t i = 0; i < strlen(surface);) {
        wchar_t wc;
        size_t n = mbrtowc(&wc, surface + i, strlen(surface) - i, &st);
        if (n == 0 || n == (size_t)-1 || n == (size_t)-2) break;

        if (is_kanji(wc)) {
            if (first_kanji < 0) first_kanji = char_idx;
            last_kanji = char_idx;
        }
        char_idx++;
        i += n;
    }

    if (first_kanji >= 0) {
        *kanji_start = first_kanji;
        *kanji_len = last_kanji - first_kanji + 1;
    }
}

/* Count unicode chars from start of string up to byte_offset */
static int count_chars_to_offset(const char *str, size_t byte_offset)
{
    int count = 0;
    mbstate_t st = {0};
    size_t i = 0;
    while (i < byte_offset && str[i]) {
        wchar_t wc;
        size_t n = mbrtowc(&wc, str + i, byte_offset - i, &st);
        if (n == 0 || n == (size_t)-1 || n == (size_t)-2) break;
        count++;
        i += n;
    }
    return count;
}

FuriganaToken *analyze_text_with_mecab(mecab_t *mecab, const char *line, int *token_count)
{
    *token_count = 0;
    const mecab_node_t *node = mecab_sparse_tonode(mecab, line);
    if (!node) return NULL;

    int capacity = INITIAL_TOKEN_CAPACITY;
    FuriganaToken *tokens = malloc(capacity * sizeof(FuriganaToken));
    if (!tokens) return NULL;

    for (; node; node = node->next) {
        if (node->stat != MECAB_NOR_NODE && node->stat != MECAB_UNK_NODE) continue;

        char surface[256] = {0};
        strncpy(surface, node->surface, node->length);
        int surface_chars = count_unicode_chars(surface);
        if (surface_chars == 0) continue;

        /* Calculate character position using byte offset from original line */
        size_t byte_offset = (size_t)(node->surface - line);
        int char_pos = count_chars_to_offset(line, byte_offset);

        /* Find kanji span in surface */
        int kanji_start, kanji_len;
        find_kanji_span(surface, &kanji_start, &kanji_len);

        if (kanji_len == 0) continue;

        char *reading = extract_mecab_field(node->feature, MECAB_READING_FIELD);
        if (!reading || strcmp(reading, "*") == 0) { free(reading); continue; }

        char *hiragana = katakana_to_hiragana(reading);
        free(reading);
        if (!hiragana) continue;

        /* Extract only the kanji portion of the reading */
        char *kanji_reading = extract_kanji_reading(surface, hiragana);
        free(hiragana);
        if (!kanji_reading) continue;

        if (*token_count >= capacity) {
            capacity *= 2;
            FuriganaToken *tmp = realloc(tokens, capacity * sizeof(FuriganaToken));
            if (!tmp) {
                for (int j = 0; j < *token_count; j++)
                    free(tokens[j].reading);
                free(tokens);
                free(kanji_reading);
                *token_count = 0;
                return NULL;
            }
            tokens = tmp;
        }

        tokens[*token_count].reading = kanji_reading;
        tokens[*token_count].start_char = char_pos + kanji_start;
        tokens[*token_count].char_len = kanji_len;
        tokens[*token_count].x = 0.0f;

        (*token_count)++;
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
