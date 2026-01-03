/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - ass.c
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ass.h"
#include "types.h"
#include "utils.h"
#include "mecab_helpers.h"

static void write_ass_header(FILE *f, struct font_config *cfg)
{
	fprintf(f, "[Script Info]\n");
	fprintf(f, "ScriptType: v4.00+\n");
	fprintf(f, "PlayResX: %d\n", cfg->screen_w);
	fprintf(f, "PlayResY: %d\n\n", cfg->screen_h);
}

static void write_ass_styles(FILE *f, struct font_config *cfg)
{
	fprintf(f, "[V4+ Styles]\n");
	fprintf(f, "Format: Name,Fontname,Fontsize,PrimaryColour,OutlineColour,"
		   "BackColour,Bold,Italic,BorderStyle,Outline,Shadow,Alignment,"
		   "MarginL,MarginR,MarginV,Effect,Encoding\n");

	fprintf(f, "Style: Main,%s,%d,&H00FFFFFF,&H00000000,&H00000000,"
		   "0,0,1,2,0,5,10,10,10,\n",
		cfg->font_name, cfg->main_size);

	fprintf(f, "Style: Furi,%s,%d,&H00FFFFFF,&H00000000,&H00000000,"
		   "0,0,1,1,0,5,10,10,10,\n\n",
		cfg->font_name, cfg->furigana_size);
}

static int count_lines_in_text(const char *text)
{
	int count = 1;
	const char *p;

	for (p = text; *p; p++) {
		if (*p == '\n')
			count++;
	}
	return count;
}

/*
 * Count lines in subtitles that share the same timing (simultaneous subs).
 */
static int count_simultaneous_lines(struct subtitle *subs, int count,
				    int current, int direction)
{
	int lines = 0;
	int i;

	if (direction > 0) {
		/* Count lines after current */
		for (i = current + 1; i < count; i++) {
			if (subs[i].start_ms != subs[current].start_ms ||
			    subs[i].end_ms != subs[current].end_ms)
				break;
			lines += count_lines_in_text(subs[i].text);
		}
	} else {
		/* Count lines before current */
		for (i = current - 1; i >= 0; i--) {
			if (subs[i].start_ms != subs[current].start_ms ||
			    subs[i].end_ms != subs[current].end_ms)
				break;
			lines += count_lines_in_text(subs[i].text);
		}
	}
	return lines;
}

static void write_subtitle_line(FILE *f, const char *ts, const char *te,
				const char *line, int y,
				struct font_config *cfg, mecab_t *mecab)
{
	struct furigana_token *tokens;
	int tcount = 0;
	int t;

	fprintf(f, "Dialogue: 0,%s,%s,Main,,0,0,0,,{\\pos(%.1f,%d)\\an5}%s\n",
		ts, te, cfg->screen_w / 2.0f, y, line);

	tokens = analyze_text_with_mecab(mecab, line, &tcount);
	if (tokens)
		calculate_token_positions(line, tokens, tcount, cfg);

	for (t = 0; t < tcount; t++) {
		fprintf(f, "Dialogue: 1,%s,%s,Furi,,0,0,0,,"
			   "{\\pos(%.1f,%d)\\an5}%s\n",
			ts, te, tokens[t].x, y - cfg->furigana_offset,
			tokens[t].reading);
		free(tokens[t].reading);
	}
	free(tokens);
}

static void process_subtitle(FILE *f, struct subtitle *subs, int count,
			     int idx, struct font_config *cfg, mecab_t *mecab)
{
	char ts[MAX_TIME], te[MAX_TIME];
	char *copy, *line, *save;
	int num_lines, lines_after, line_idx;
	int line_from_bottom, y;

	format_ass_time(subs[idx].start_ms, ts);
	format_ass_time(subs[idx].end_ms, te);

	num_lines = count_lines_in_text(subs[idx].text);
	lines_after = count_simultaneous_lines(subs, count, idx, 1);

	copy = strdup(subs[idx].text);
	if (!copy)
		return;

	save = NULL;
	line_idx = 0;
	line = strtok_r(copy, "\n", &save);

	while (line) {
		line_from_bottom = lines_after + (num_lines - 1 - line_idx);
		y = cfg->baseline_y - line_from_bottom * cfg->line_spacing;

		write_subtitle_line(f, ts, te, line, y, cfg, mecab);

		line = strtok_r(NULL, "\n", &save);
		line_idx++;
	}

	free(copy);
}

void generate_ass(const char *input, struct subtitle *subs, int count,
		  struct font_config *cfg, mecab_t *mecab)
{
	FILE *f;
	char out[JPSUB_MAX_PATH];
	int i;

	snprintf(out, sizeof(out), "%s.ass", input);

	f = fopen(out, "w");
	if (!f) {
		perror("fopen");
		return;
	}

	write_ass_header(f, cfg);
	write_ass_styles(f, cfg);

	fprintf(f, "[Events]\n");
	fprintf(f, "Format: Layer,Start,End,Style,Name,MarginL,MarginR,"
		   "MarginV,Effect,Text\n");

	for (i = 0; i < count; i++)
		process_subtitle(f, subs, count, i, cfg, mecab);

	fclose(f);
}
