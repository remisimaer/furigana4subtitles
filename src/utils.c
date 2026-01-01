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

static FontConfig default_cfg = {
    .font_name = "MS Gothic",
    .main_size = 52,
    .furigana_size = 26,
    .screen_w = 1920,
    .screen_h = 1080,
    .baseline_y = 980,
    .furigana_offset = 48,
    .char_width = 52.0f,
    .line_spacing = 104
};

FontConfig *get_default_config(void)
{
    return &default_cfg;
}

FontConfig *create_scaled_config(int main_size)
{
    static FontConfig scaled_cfg;
    
    /* Base reference: 52px main size */
    const int BASE_SIZE = 52;
    float scale = (float)main_size / BASE_SIZE;
    
    scaled_cfg.font_name = default_cfg.font_name;
    scaled_cfg.main_size = main_size;
    scaled_cfg.furigana_size = (int)(26 * scale + 0.5f);      /* Half of main_size */
    scaled_cfg.screen_w = default_cfg.screen_w;
    scaled_cfg.screen_h = default_cfg.screen_h;
    
    int fixed_margin = 50;  /* Small margin from screen edge */
    scaled_cfg.baseline_y = default_cfg.screen_h - fixed_margin - main_size;
    
    scaled_cfg.furigana_offset = (int)(48 * scale + 0.5f);    /* ~92% of main_size */
    scaled_cfg.char_width = main_size;                        /* Same as main_size */
    scaled_cfg.line_spacing = (int)(104 * scale + 0.5f);      /* 2x main_size */
    
    return &scaled_cfg;
}

void print_banner(void)
{
    printf("\n");
    printf("     ┏━╸╻ ╻┏━┓╻┏━╸┏━┓┏┓╻┏━┓   ╻ ╻   ┏━┓╻ ╻┏┓ ╺┳╸╻╺┳╸╻  ┏━╸┏━┓\n");
    printf("     ┣╸ ┃ ┃┣┳┛┃┃╺┓┣━┫┃┗┫┣━┫   ┗━┫   ┗━┓┃ ┃┣┻┓ ┃ ┃ ┃ ┃  ┣╸ ┗━┓\n");
    printf("     ╹  ┗━┛╹┗╸╹┗━┛╹ ╹╹ ╹╹ ╹     ╹   ┗━┛┗━┛┗━┛ ╹ ╹ ╹ ┗━╸┗━╸┗━┛\n");
    printf("\n");
    printf("  Kanji blocking your anime night? This kitty brings furigana~ ♪\n");
    printf("     漢字が読めなくて困ってる？この子猫がふりがなを届けるよ～♪\n");
    printf("\n");
    printf("                    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡀⠀⠀⠀⠀\n");
    printf("                    ⠀⠀⠀⠀⢀⡴⣆⠀⠀⠀⠀⠀⣠⡀⠀⠀⠀⠀⠀⠀⣼⣿⡗⠀⠀⠀⠀\n");
    printf("                    ⠀⠀⠀⣠⠟⠀⠘⠷⠶⠶⠶⠾⠉⢳⡄⠀⠀⠀⠀⠀⣧⣿⠀⠀⠀⠀⠀\n");
    printf("                    ⠀⠀⣰⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣤⣤⣤⣤⣤⣿⢿⣄⠀⠀⠀⠀\n");
    printf("                    ⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣧⠀⠀⠀⠀⠀⠀⠙⣷⡴⠶⣦\n");
    printf("                    ⠀⠀⢱⡀⠀⠉⠉⠀⠀⠀⠀⠛⠃⠀⢠⡟⠀⠀⠀⢀⣀⣠⣤⠿⠞⠛⠋\n");
    printf("                    ⣠⠾⠋⠙⣶⣤⣤⣤⣤⣤⣀⣠⣤⣾⣿⠴⠶⠚⠋⠉⠁⠀⠀⠀⠀⠀⠀\n");
    printf("                    ⠛⠒⠛⠉⠉⠀⠀⠀⣴⠟⢃⡴⠛⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    printf("                    ⠀⠀⠀⠀⠀⠀⠀⠀⠛⠛⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    printf("\n");
    printf("                      (^_^) Version 1.0.0 (^_^)\n");
    printf("\n");
    printf("  ════════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("  * Find this useful? Buy me a coffee!\n");
    printf("    https://www.paypal.com/donate/?hosted_button_id=2ZYLTYB2R9XGC\n");
    printf("\n");
    printf("  * Want to contribute?\n");
    printf("    https://github.com/remisimaer/furigana4subtitles\n");
    printf("\n");
    printf("  * 開発者を探している採用担当者の方は、ぜひご連絡ください。\n");
    printf("    https://www.remisimaer.com\n");
    printf("\n");
    printf("  ════════════════════════════════════════════════════════════════\n");
    printf("\n");
}

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
    snprintf(buf, MAX_TIME, "%d:%02d:%02d.%02d", h, m, s, cs);
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
