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
#include <locale.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <types.h>
#include <utils.h>
#include <mecab_helpers.h>
#include <srt.h>
#include <ass.h>
#include <mecab.h>
#include <raylib.h>


int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    if (argc < 2) {
        printf("Usage: %s [-R] file.srt|directory [...]\n", argv[0]);
        printf("  -R  Process directories recursively\n");
        return 1;
    }

    mecab_t *mecab = mecab_new2("");
    if (!mecab) {
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
        .line_spacing = 96
    };

    int recursive = 0;
    int start = 1;

    if (strcmp(argv[1], "-R") == 0) {
        recursive = 1;
        start = 2;
        if (argc < 3) {
            fprintf(stderr, "Error: -R requires at least one path\n");
            mecab_destroy(mecab);
            return 1;
        }
    }

    for (int i = start; i < argc; i++) {
        struct stat st;
        if (stat(argv[i], &st) != 0) {
            fprintf(stderr, "Cannot access: %s\n", argv[i]);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (recursive)
                scan_directory(argv[i], &cfg, mecab);
            else
                fprintf(stderr, "Skipping directory (use -R): %s\n", argv[i]);
        } else if (ends_with_srt(argv[i])) {
            process_file(argv[i], &cfg, mecab);
        }
    }

    mecab_destroy(mecab);
    return 0;
}