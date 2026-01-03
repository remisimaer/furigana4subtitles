/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Furigana4subtitles - cli.h
 * Copyright (C) 2026 Rémi SIMAER <rsimaer@gmail.com>
 */

#ifndef JPSUB_CLI_H
#define JPSUB_CLI_H

#include <mecab.h>
#include "types.h"

struct cli_ctx {
	mecab_t *mecab;
	struct font_config *cfg;
};

int cli_init(struct cli_ctx *ctx);
void cli_cleanup(struct cli_ctx *ctx);
int cli_run(struct cli_ctx *ctx);

#endif
