/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - srt.c
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "srt.h"
#include "types.h"

enum parse_state {
	STATE_INDEX,
	STATE_TIME,
	STATE_TEXT
};

static int parse_time_line(const char *line, struct subtitle *sub)
{
	int h1, m1, s1, ms1;
	int h2, m2, s2, ms2;

	if (sscanf(line, "%d:%d:%d,%d --> %d:%d:%d,%d",
		   &h1, &m1, &s1, &ms1, &h2, &m2, &s2, &ms2) != 8)
		return -1;

	sub->start_ms = h1 * MS_PER_HOUR + m1 * MS_PER_MINUTE +
			s1 * MS_PER_SECOND + ms1;
	sub->end_ms = h2 * MS_PER_HOUR + m2 * MS_PER_MINUTE +
		      s2 * MS_PER_SECOND + ms2;
	return 0;
}

static int append_text(struct subtitle *sub, const char *line)
{
	size_t old_len, add_len;
	char *tmp;

	old_len = sub->text ? strlen(sub->text) : 0;
	add_len = strlen(line);

	tmp = realloc(sub->text, old_len + add_len + 2);
	if (!tmp)
		return -1;

	sub->text = tmp;
	if (old_len) {
		sub->text[old_len] = '\n';
		strcpy(sub->text + old_len + 1, line);
	} else {
		strcpy(sub->text, line);
	}
	return 0;
}

static int grow_subs_array(struct subtitle **subs, int *capacity)
{
	struct subtitle *tmp;

	*capacity *= 2;
	tmp = realloc(*subs, *capacity * sizeof(struct subtitle));
	if (!tmp)
		return -1;

	*subs = tmp;
	return 0;
}

static void free_subs(struct subtitle *subs, int count)
{
	int i;

	for (i = 0; i < count; i++)
		free(subs[i].text);
	free(subs);
}

struct subtitle *parse_srt(const char *path, int *count)
{
	FILE *f;
	struct subtitle *subs;
	char line[MAX_LINE];
	enum parse_state state = STATE_INDEX;
	int capacity = INITIAL_SUB_CAPACITY;
	int idx = -1;

	*count = 0;

	f = fopen(path, "r");
	if (!f)
		return NULL;

	subs = calloc(capacity, sizeof(struct subtitle));
	if (!subs) {
		fclose(f);
		return NULL;
	}

	while (fgets(line, sizeof(line), f)) {
		line[strcspn(line, "\r\n")] = '\0';

		switch (state) {
		case STATE_INDEX:
			if (isdigit((unsigned char)line[0]))
				state = STATE_TIME;
			break;

		case STATE_TIME:
			if (!strstr(line, "-->"))
				break;

			if (*count >= capacity) {
				if (grow_subs_array(&subs, &capacity) < 0) {
					free_subs(subs, *count);
					fclose(f);
					*count = 0;
					return NULL;
				}
			}

			if (parse_time_line(line, &subs[*count]) < 0)
				break;

			subs[*count].text = NULL;
			idx = *count;
			state = STATE_TEXT;
			break;

		case STATE_TEXT:
			if (line[0] == '\0') {
				(*count)++;
				idx = -1;
				state = STATE_INDEX;
			} else if (idx >= 0) {
				append_text(&subs[idx], line);
			}
			break;
		}
	}

	/* Handle last subtitle if file doesn't end with blank line */
	if (state == STATE_TEXT && idx >= 0)
		(*count)++;

	fclose(f);
	return subs;
}
