/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - cli.c
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cli.h"
#include "utils.h"

#define INPUT_SIZE	512
#define FONT_MIN	16
#define FONT_MAX	120

static char *read_line(char *buf, int size)
{
	if (!fgets(buf, size, stdin))
		return NULL;

	buf[strcspn(buf, "\r\n")] = '\0';
	return buf;
}

static int next_path(char **cursor, char *out, int max_len)
{
	char *p = *cursor;
	int i = 0;

	while (*p == ' ')
		p++;

	if (*p == '\0')
		return 0;

	if (*p == '"') {
		for (p++; *p && *p != '"' && i < max_len - 1; p++)
			out[i++] = *p;
		if (*p == '"')
			p++;
	} else {
		for (; *p && *p != ' ' && i < max_len - 1; p++)
			out[i++] = *p;
	}

	out[i] = '\0';
	*cursor = p;
	return i;
}

static int process_path(const char *path, struct cli_ctx *ctx)
{
	struct stat st;

	if (stat(path, &st) != 0) {
		fprintf(stderr, "  ✗ Cannot access: %s\n", path);
		return -1;
	}

	if (S_ISDIR(st.st_mode)) {
		printf("Scanning folder: %s\n", path);
		scan_directory(path, ctx->cfg, ctx->mecab);
		return 1;
	}

	if (ends_with_srt(path)) {
		process_file(path, ctx->cfg, ctx->mecab);
		return 1;
	}

	fprintf(stderr, "Not a .srt file: %s\n", path);
	return 0;
}

static void cmd_convert(struct cli_ctx *ctx)
{
	char input[INPUT_SIZE], path[INPUT_SIZE];
	char *cursor;
	int count = 0;

	printf("\n");
	printf("  ┌─────────────────────────────────────────────────────────────┐\n");
	printf("  │                    CONVERT SUBTITLES                        │\n");
	printf("  ├─────────────────────────────────────────────────────────────┤\n");
	printf("  │                                                             │\n");
	printf("  │  FILES (.srt)                                               │\n");
	printf("  │    One file:       movie.srt                                │\n");
	printf("  │    Multiple:       ep01.srt ep02.srt ep03.srt               │\n");
	printf("  │    With spaces:    \"my subtitle.srt\"                        │\n");
	printf("  │                                                             │\n");
	printf("  │  FOLDERS (recursive scan)                                   │\n");
	printf("  │    One folder:     ./subs/                                  │\n");
	printf("  │    Multiple:       ./season1/ ./season2/                    │\n");
	printf("  │                                                             │\n");
	printf("  │  MIX                                                        │\n");
	printf("  │    ./folder/ \"special.srt\" ./other/                         │\n");
	printf("  │                                                             │\n");
	printf("  ├─────────────────────────────────────────────────────────────┤\n");
	printf("  │  [q] Back to main menu                                      │\n");
	printf("  └─────────────────────────────────────────────────────────────┘\n");
	printf("\n  > ");

	if (!read_line(input, sizeof(input)))
		return;

	if (input[0] == '\0' || input[0] == 'q' || input[0] == 'Q') {
		printf("  Cancelled.\n");
		return;
	}

	printf("\n");
	cursor = input;
	while (next_path(&cursor, path, sizeof(path))) {
		if (process_path(path, ctx) > 0)
			count++;
	}

	printf("\nProcessed %d item(s).\n", count);
}

static void cmd_set_font_size(struct cli_ctx *ctx)
{
	char input[INPUT_SIZE];
	int size;

	printf("\n");
	printf("  ┌─────────────────────────────────────────────────────────────┐\n");
	printf("  │                      SET FONT SIZE                          │\n");
	printf("  ├─────────────────────────────────────────────────────────────┤\n");
	printf("  │  Current: %3dpx                                             │\n", ctx->cfg->main_size);
	printf("  │  Range:   %d-%dpx                                          │\n", FONT_MIN, FONT_MAX);
	printf("  │                                                             │\n");
	printf("  │  Recommended:                                               │\n");
	printf("  │    32px  Small   (dense subtitles)                          │\n");
	printf("  │    42px  Medium                                             │\n");
	printf("  │    52px  Large   (default)                                  │\n");
	printf("  │    62px  X-Large                                            │\n");
	printf("  │                                                             │\n");
	printf("  ├─────────────────────────────────────────────────────────────┤\n");
	printf("  │  [q] Back to main menu                                      │\n");
	printf("  └─────────────────────────────────────────────────────────────┘\n");
	printf("\n  > ");

	if (!read_line(input, sizeof(input)))
		return;

	if (input[0] == '\0' || input[0] == 'q' || input[0] == 'Q') {
		printf("  Cancelled.\n");
		return;
	}

	size = atoi(input);

	if (size < FONT_MIN || size > FONT_MAX) {
		fprintf(stderr, "Invalid size.\n");
		return;
	}

	ctx->cfg = create_scaled_config(size);

	printf("\nUpdated: %dpx main, %dpx furigana\n",
	       ctx->cfg->main_size, ctx->cfg->furigana_size);
}

static void print_menu(int font_size)
{
	printf("  ╔══════════════════════════════════════════════════════════════╗\n"
	       "  ║                           MAIN MENU                          ║\n"
	       "  ║              .srt → .ass with furigana (ふりがな)            ║\n"
	       "  ╠══════════════════════════════════════════════════════════════╣\n"
	       "  ║   [1] Convert subtitles                                      ║\n"
	       "  ║   [2] Set font size (current: %3dpx)                         ║\n"
	       "  ║   [q] Quit                                                   ║\n"
	       "  ╚══════════════════════════════════════════════════════════════╝\n"
	       "\n  ➜ ", font_size);
}

static int menu_loop(struct cli_ctx *ctx)
{
	char input[INPUT_SIZE];

	print_menu(ctx->cfg->main_size);

	if (!read_line(input, sizeof(input)))
		return 0;

	switch (input[0]) {
	case '1':
		cmd_convert(ctx);
		break;
	case '2':
		cmd_set_font_size(ctx);
		break;
	case 'q':
	case 'Q':
	case '3':
		printf("\nBye! ₍^. .^₎⟆\n");
		return 0;
	default:
		printf("\nInvalid choice.\n");
	}

	printf("\n");
	return 1;
}


int cli_init(struct cli_ctx *ctx)
{
	ctx->mecab = mecab_new2("");
	if (!ctx->mecab) {
		fprintf(stderr, "MeCab initialization failed\n");
		return -1;
	}

	ctx->cfg = get_default_config();
	return 0;
}

void cli_cleanup(struct cli_ctx *ctx)
{
	if (ctx->mecab)
		mecab_destroy(ctx->mecab);
}

int cli_run(struct cli_ctx *ctx)
{
	print_banner();

	while (menu_loop(ctx))
		;

	return 0;
}
