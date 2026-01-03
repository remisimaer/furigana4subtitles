/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - mecab_helpers.c
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "mecab_helpers.h"
#include "utils.h"

char *extract_mecab_field(const char *feature, int index)
{
	char *copy, *tok, *res;
	int i;

	if (!feature)
		return NULL;

	copy = strdup(feature);
	if (!copy)
		return NULL;

	tok = strtok(copy, ",");
	for (i = 0; i < index && tok; i++)
		tok = strtok(NULL, ",");

	res = tok ? strdup(tok) : NULL;
	free(copy);
	return res;
}

char *katakana_to_hiragana(const char *in)
{
	wchar_t *wbuf;
	char *out;
	size_t len, i;

	if (!in)
		return NULL;

	len = strlen(in);
	wbuf = malloc((len + 1) * sizeof(wchar_t));
	if (!wbuf)
		return NULL;

	if (mbstowcs(wbuf, in, len + 1) == (size_t)-1) {
		free(wbuf);
		return NULL;
	}

	/* Convert katakana (0x30A1-0x30FA) to hiragana by subtracting 0x60 */
	for (i = 0; wbuf[i]; i++) {
		if (wbuf[i] >= 0x30A1 && wbuf[i] <= 0x30FA)
			wbuf[i] -= 0x60;
	}

	out = malloc(len * 4 + 1);
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

/*
 * Extract kanji-only reading by stripping matching hiragana
 * from the beginning and end of the surface form.
 */
static char *extract_kanji_reading(const char *surface, const char *full_reading)
{
	wchar_t *wsurf = NULL;
	wchar_t *wread = NULL;
	wchar_t *wresult = NULL;
	char *result = NULL;
	size_t surf_len, read_len;
	size_t wsurf_len, wread_len;
	size_t prefix_skip, suffix_skip;
	size_t start, end, new_len;

	if (!surface || !full_reading)
		return NULL;

	surf_len = strlen(surface);
	read_len = strlen(full_reading);

	wsurf = malloc((surf_len + 1) * sizeof(wchar_t));
	wread = malloc((read_len + 1) * sizeof(wchar_t));
	if (!wsurf || !wread)
		goto out_free;

	if (mbstowcs(wsurf, surface, surf_len + 1) == (size_t)-1)
		goto out_free;
	if (mbstowcs(wread, full_reading, read_len + 1) == (size_t)-1)
		goto out_free;

	wsurf_len = wcslen(wsurf);
	wread_len = wcslen(wread);

	/* Strip matching hiragana from the beginning */
	prefix_skip = 0;
	while (prefix_skip < wsurf_len && prefix_skip < wread_len &&
	       is_hiragana(wsurf[prefix_skip]) &&
	       wsurf[prefix_skip] == wread[prefix_skip]) {
		prefix_skip++;
	}

	/* Strip matching hiragana from the end */
	suffix_skip = 0;
	while (suffix_skip < (wsurf_len - prefix_skip) &&
	       suffix_skip < (wread_len - prefix_skip) &&
	       is_hiragana(wsurf[wsurf_len - 1 - suffix_skip]) &&
	       wsurf[wsurf_len - 1 - suffix_skip] ==
	       wread[wread_len - 1 - suffix_skip]) {
		suffix_skip++;
	}

	/* Extract the middle part (corresponds to kanji reading) */
	start = prefix_skip;
	end = wread_len - suffix_skip;
	if (start >= end) {
		/* Fallback to full reading */
		result = strdup(full_reading);
		goto out_free;
	}

	new_len = end - start;
	wresult = malloc((new_len + 1) * sizeof(wchar_t));
	if (!wresult)
		goto out_free;

	wcsncpy(wresult, wread + start, new_len);
	wresult[new_len] = L'\0';

	result = malloc(new_len * 4 + 1);
	if (!result)
		goto out_free;

	if (wcstombs(result, wresult, new_len * 4 + 1) == (size_t)-1) {
		free(result);
		result = NULL;
	}

out_free:
	free(wsurf);
	free(wread);
	free(wresult);
	return result;
}

/*
 * Find the kanji span (start position and length) in a surface string.
 */
static void find_kanji_span(const char *surface, int *kanji_start, int *kanji_len)
{
	mbstate_t st = {0};
	int char_idx = 0;
	int first_kanji = -1;
	int last_kanji = -1;
	size_t i;

	*kanji_start = -1;
	*kanji_len = 0;

	for (i = 0; i < strlen(surface); ) {
		wchar_t wc;
		size_t n = mbrtowc(&wc, surface + i, strlen(surface) - i, &st);

		if (n == 0 || n == (size_t)-1 || n == (size_t)-2)
			break;

		if (is_kanji(wc)) {
			if (first_kanji < 0)
				first_kanji = char_idx;
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

/*
 * Count unicode characters from start of string up to byte_offset.
 */
static int count_chars_to_offset(const char *str, size_t byte_offset)
{
	mbstate_t st = {0};
	int count = 0;
	size_t i = 0;

	while (i < byte_offset && str[i]) {
		wchar_t wc;
		size_t n = mbrtowc(&wc, str + i, byte_offset - i, &st);

		if (n == 0 || n == (size_t)-1 || n == (size_t)-2)
			break;
		count++;
		i += n;
	}
	return count;
}

struct furigana_token *analyze_text_with_mecab(mecab_t *mecab, const char *line,
					       int *token_count)
{
	const mecab_node_t *node;
	struct furigana_token *tokens;
	int capacity = INITIAL_TOKEN_CAPACITY;

	*token_count = 0;

	node = mecab_sparse_tonode(mecab, line);
	if (!node)
		return NULL;

	tokens = malloc(capacity * sizeof(struct furigana_token));
	if (!tokens)
		return NULL;

	for (; node; node = node->next) {
		char surface[256] = {0};
		char *reading, *hiragana, *kanji_reading;
		size_t copy_len, byte_offset;
		int surface_chars, char_pos;
		int kanji_start, kanji_len;

		if (node->stat != MECAB_NOR_NODE && node->stat != MECAB_UNK_NODE)
			continue;

		copy_len = node->length < 255 ? node->length : 255;
		strncpy(surface, node->surface, copy_len);
		surface[copy_len] = '\0';

		surface_chars = count_unicode_chars(surface);
		if (surface_chars == 0)
			continue;

		/* Calculate character position from byte offset */
		byte_offset = (size_t)(node->surface - line);
		char_pos = count_chars_to_offset(line, byte_offset);

		find_kanji_span(surface, &kanji_start, &kanji_len);
		if (kanji_len == 0)
			continue;

		reading = extract_mecab_field(node->feature, MECAB_READING_FIELD);
		if (!reading || strcmp(reading, "*") == 0) {
			free(reading);
			continue;
		}

		hiragana = katakana_to_hiragana(reading);
		free(reading);
		if (!hiragana)
			continue;

		kanji_reading = extract_kanji_reading(surface, hiragana);
		free(hiragana);
		if (!kanji_reading)
			continue;

		/* Grow array if needed */
		if (*token_count >= capacity) {
			struct furigana_token *tmp;

			capacity *= 2;
			tmp = realloc(tokens,
				      capacity * sizeof(struct furigana_token));
			if (!tmp) {
				int j;

				for (j = 0; j < *token_count; j++)
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

void calculate_token_positions(const char *line, struct furigana_token *tokens,
			       int token_count, struct font_config *cfg)
{
	int total_chars = count_unicode_chars(line);
	float start_x = (cfg->screen_w - total_chars * cfg->char_width) / 2.0f;
	int i;

	for (i = 0; i < token_count; i++) {
		float center_char = tokens[i].start_char +
				    tokens[i].char_len / 2.0f;
		tokens[i].x = start_x + center_char * cfg->char_width;
	}
}
