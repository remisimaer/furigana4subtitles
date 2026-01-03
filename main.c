/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - main.c
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 *
 * Command-line tool for converting SRT subtitles to ASS with furigana.
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <sys/stat.h>
#include <mecab.h>

#include "types.h"
#include "utils.h"
#include "mecab_helpers.h"
#include "srt.h"
#include "ass.h"

int main(int argc, char **argv)
{
	mecab_t *mecab;
	struct font_config *cfg;
	int i;

	setlocale(LC_ALL, "");

	if (argc < 2) {
		fprintf(stderr, "Usage: %s file.srt|directory [...]\n", argv[0]);
		return 1;
	}

	print_banner();

	mecab = mecab_new2("");
	if (!mecab) {
		fprintf(stderr, "MeCab initialization failed\n");
		return 1;
	}

	cfg = get_default_config();

	for (i = 1; i < argc; i++) {
		struct stat st;

		if (stat(argv[i], &st) != 0) {
			fprintf(stderr, "Cannot access: %s\n", argv[i]);
			continue;
		}

		if (S_ISDIR(st.st_mode))
			scan_directory(argv[i], cfg, mecab);
		else if (ends_with_srt(argv[i]))
			process_file(argv[i], cfg, mecab);
	}

	mecab_destroy(mecab);
	return 0;
}
