/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - main_cli.c
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 *
 * Interactive CLI for converting SRT subtitles to ASS with furigana.
 */

#include <locale.h>
#include "cli.h"

int main(void)
{
	struct cli_ctx ctx = {0};
	int ret;

	setlocale(LC_ALL, "");

	if (cli_init(&ctx) < 0)
		return 1;

	ret = cli_run(&ctx);

	cli_cleanup(&ctx);
	return ret;
}
