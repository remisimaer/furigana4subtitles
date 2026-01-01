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

#define INPUT_SIZE 512

static FontConfig *cfg;

static void print_menu(void)
{
    printf("  ╔══════════════════════════════════════════════════════════════╗\n");
    printf("  ║                           MAIN MENU                          ║\n");
    printf("  ║              .srt → .ass with furigana (ふりがな)            ║\n");
    printf("  ╠══════════════════════════════════════════════════════════════╣\n");
    printf("  ║                                                              ║\n");
    printf("  ║   [1] ➤ Convert file(s)                                      ║\n");
    printf("  ║   [2] ➤ Convert folder                                       ║\n");
    printf("  ║   [3] ➤ Set font size (current: %3dpx)                       ║\n", cfg->main_size);
    printf("  ║   [4] ➤ Quit                                                 ║\n");
    printf("  ║                                                              ║\n");
    printf("  ╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n  ➜ Choice: ");
}

static void trim_newline(char *str)
{
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
        str[len - 1] = '\0';
}

static void convert_files(mecab_t *mecab)
{
    char input[INPUT_SIZE];
    printf("\n── Convert File(s) ──\n");
    printf("Use quotes around each path to handle spaces and special characters.\n");
    printf("Examples:\n");
    printf("  \"subtitle.srt\"\n");
    printf("  \"my file.srt\" \"other file.srt\"\n");
    printf("  \"./subs/my subtitle.srt\"\n");
    printf("\nEnter file path(s), then press ENTER:\n> ");
    if (!fgets(input, INPUT_SIZE, stdin)) return;
    trim_newline(input);

    char *p = input;
    int count = 0;
    while (*p) {
        while (*p == ' ') p++;
        if (*p == '\0') break;

        char path[INPUT_SIZE];
        int i = 0;

        if (*p == '"') {
            p++;
            while (*p && *p != '"' && i < INPUT_SIZE - 1)
                path[i++] = *p++;
            if (*p == '"') p++;
        } else {
            while (*p && *p != ' ' && i < INPUT_SIZE - 1)
                path[i++] = *p++;
        }
        path[i] = '\0';

        if (i > 0) {
            struct stat st;
            if (stat(path, &st) != 0) {
                fprintf(stderr, "Cannot access: %s\n", path);
            } else if (S_ISDIR(st.st_mode)) {
                scan_directory(path, cfg, mecab);
                count++;
            } else if (ends_with_srt(path)) {
                process_file(path, cfg, mecab);
                count++;
            } else {
                fprintf(stderr, "Not a .srt file: %s\n", path);
            }
        }
    }
    printf("\nDone! Processed %d item(s).\n", count);
}

static void convert_folder(mecab_t *mecab)
{
    char input[INPUT_SIZE];
    printf("\n── Convert Folder ──\n");
    printf("All .srt files in the folder will be converted recursively.\n");
    printf("Use quotes around the path to handle spaces and special characters.\n");
    printf("Examples:\n");
    printf("  \"./subs\"\n");
    printf("  \"/home/user/My Anime Subs/\"\n");
    printf("\nEnter folder path, then press ENTER:\n> ");
    if (!fgets(input, INPUT_SIZE, stdin)) return;
    trim_newline(input);

    /* Handle quoted path */
    char path[INPUT_SIZE];
    char *p = input;
    while (*p == ' ') p++;
    
    if (*p == '"') {
        p++;
        int i = 0;
        while (*p && *p != '"' && i < INPUT_SIZE - 1)
            path[i++] = *p++;
        path[i] = '\0';
    } else {
        strncpy(path, input, INPUT_SIZE - 1);
        path[INPUT_SIZE - 1] = '\0';
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fprintf(stderr, "Cannot access: %s\n", path);
        return;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Not a directory: %s\n", path);
        return;
    }

    scan_directory(path, cfg, mecab);
    printf("\nDone!\n");
}

static void set_font_size(void)
{
    char input[INPUT_SIZE];
    printf("\n── Set Font Size ──\n");
    printf("Current main font size: %dpx\n", cfg->main_size);
    printf("All other properties will be scaled proportionally.\n");
    printf("\nRecommended sizes:\n");
    printf("  • 32px - Small (for dense subtitles)\n");
    printf("  • 42px - Medium\n");
    printf("  • 52px - Default\n");
    printf("  • 62px - Large\n");
    printf("  • 72px - Extra Large\n");
    printf("\nEnter new font size in pixels (16-120), or 0 to cancel:\n> ");
    
    if (!fgets(input, INPUT_SIZE, stdin)) return;
    trim_newline(input);
    
    int size = atoi(input);
    if (size == 0) {
        printf("Cancelled.\n");
        return;
    }
    
    if (size < 16 || size > 120) {
        printf("Invalid size. Please choose a value between 16 and 120 pixels.\n");
        return;
    }
    
    cfg = create_scaled_config(size);
    printf("\n✓ Font configuration updated!\n");
    printf("  Main size:       %dpx\n", cfg->main_size);
    printf("  Furigana size:   %dpx\n", cfg->furigana_size);
    printf("  Char width:      %.1fpx\n", cfg->char_width);
    printf("  Line spacing:    %dpx\n", cfg->line_spacing);
    printf("  Furigana offset: %dpx\n", cfg->furigana_offset);
    printf("  Baseline Y:      %dpx (from top)\n", cfg->baseline_y);
}

int main(void)
{
    setlocale(LC_ALL, "");

    cfg = get_default_config();

    mecab_t *mecab = mecab_new2("");
    if (!mecab) {
        fprintf(stderr, "MeCab initialization failed\n");
        return 1;
    }

    print_banner();

    char input[INPUT_SIZE];
    int running = 1;

    while (running) {
        print_menu();
        if (!fgets(input, INPUT_SIZE, stdin)) break;

        switch (input[0]) {
            case '1':
                convert_files(mecab);
                break;
            case '2':
                convert_folder(mecab);
                break;
            case '3':
                set_font_size();
                break;
            case '4':
            case 'q':
            case 'Q':
                running = 0;
                printf("\nBye! ₍^. .^₎⟆\n");
                break;
            default:
                printf("\nInvalid choice.\n");
        }
        printf("\n");
    }

    mecab_destroy(mecab);
    return 0;
}
