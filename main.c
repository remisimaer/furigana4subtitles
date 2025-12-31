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
#include <locale.h>
#include <mecab.h>
#include "types.h"
#include "utils.h"
#include "mecab_helpers.h"
#include "srt.h"
#include "ass.h"

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    if (argc < 2)
    {
        printf("Usage: %s file.srt [...]\n", argv[0]);
        return 1;
    }

    mecab_t *mecab = mecab_new2("");
    if (!mecab)
    {
        fprintf(stderr, "MeCab initialization failed\n");
        return 1;
    }

    FontConfig cfg = {
        .font_name = "MS Gothic",
        .main_size = 52,
        .furigana_size = 26,
        .screen_w = 1920,
        .screen_h = 1080,
        .baseline_y = 980,
        .furigana_offset = 48,
        .char_width = 52.0f,
        .line_spacing = 96};

    for (int i = 1; i < argc; i++)
    {
        int count = 0;
        Subtitle *subs = parse_srt(argv[i], &count);
        if (!subs)
            continue;

        generate_ass(argv[i], subs, count, &cfg, mecab);

        for (int j = 0; j < count; j++)
            free(subs[j].text);
        free(subs);
    }

    mecab_destroy(mecab);
    return 0;
}
