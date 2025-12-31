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
#include <dirent.h>
#include <sys/stat.h>
#include "../include/utils.h"
#include "../include/types.h"
#include "../include/srt.h"
#include "../include/ass.h"
#include <mecab.h>

int count_unicode_chars(const char *s)
{
    if (!s) return 0;
    mbstate_t st = {0};
    int count = 0;
    size_t len = strlen(s);
    for (size_t i = 0; i < len;) {
        wchar_t wc;
        size_t n = mbrtowc(&wc, s + i, len - i, &st);
        if (n == 0 || n == (size_t)-1 || n == (size_t)-2) break;
        count++;
        i += n;
    }
    return count;
}

void format_ass_time(int ms, char *buf)
{
    int h = ms / MS_PER_HOUR;
    ms %= MS_PER_HOUR;
    int m = ms / MS_PER_MINUTE;
    ms %= MS_PER_MINUTE;
    int s = ms / MS_PER_SECOND;
    int cs = (ms % MS_PER_SECOND) / 10;
    sprintf(buf, "%d:%02d:%02d.%02d", h, m, s, cs);
}

int is_kanji(wchar_t c)
{
    return (c >= 0x4E00 && c <= 0x9FAF) || 
           (c >= 0x3400 && c <= 0x4DBF) || 
           (c >= 0x3005 && c <= 0x3007);
}

int ends_with_srt(const char *path)
{
    size_t len = strlen(path);
    return len > 4 && strcmp(path + len - 4, ".srt") == 0;
}

void process_file(const char *path, FontConfig *cfg, mecab_t *mecab)
{
    int count = 0;
    Subtitle *subs = parse_srt(path, &count);
    if (!subs) return;

    // Remove .srt extension and add .ass
    char outpath[MAX_PATH];
    strncpy(outpath, path, sizeof(outpath) - 1);
    outpath[sizeof(outpath) - 1] = '\0';
    char *dot = strrchr(outpath, '.');
    if (dot && strcmp(dot, ".srt") == 0) *dot = '\0';

    printf("Processing: %s (%d subtitles)\n", path, count);
    generate_ass(outpath, subs, count, cfg, mecab);

    for (int j = 0; j < count; j++)
        free(subs[j].text);
    free(subs);
}

void scan_directory(const char *dir, FontConfig *cfg, mecab_t *mecab)
{
    DIR *d = opendir(dir);
    if (!d) return;

    struct dirent *entry;
    while ((entry = readdir(d))) {
        if (entry->d_name[0] == '.') continue;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        struct stat st;
        if (stat(path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode))
            scan_directory(path, cfg, mecab);
        else if (ends_with_srt(path))
            process_file(path, cfg, mecab);
    }

    closedir(d);
}
