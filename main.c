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
#include <locale.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mecab.h>

#define CHECK(eval) if (! eval) { \
    fprintf (stderr, "Exception:%s\n", mecab_strerror (mecab)); \
    mecab_destroy(mecab); \
    return -1; }

char **get_path_to_files(int argc, char **argv);
static int is_fullwidth_katakana(wchar_t wc);
static int is_halfwidth_katakana(wchar_t wc);
static wchar_t to_hiragana(wchar_t c);
static bool is_kanji(char *c);
static int print_yomi_katakana_to_hiragana(const char *yomi);
char* get_mecab_node_field(const char* str, int n);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    // Retrieve the path to the .srt passed as argument
    char **subtitle_files = get_path_to_files(argc, argv);
    if (subtitle_files == NULL)
    {
        return 1;
    }

    // Parse the .srt file

    // Initialize Mecab
    mecab_t *mecab = mecab_new2("-Oyomi");
    CHECK(mecab);

    // Example text to analyze
    const char *input = "心理的瑕疵物件で困るね。";
    const char *result = mecab_sparse_tostr(mecab, input);
    CHECK(result);
    printf("Full complete phrase in katakana: %s\n", result);

    const mecab_node_t *node;
    node = mecab_sparse_tonode(mecab, input);
    CHECK(node);

    for (; node; node = node->next) {
        if (node->stat == MECAB_NOR_NODE || node->stat == MECAB_UNK_NODE) {
            fwrite (node->surface, sizeof(char), node->length, stdout);
            printf("feature:\t%s\n", node->feature);

            // Get the full word
            char* full_word = get_mecab_node_field(node->feature, 6); // 6 for full word.
            if (full_word) {
                // Check if it's a kanji
                if (is_kanji(full_word)) {
                    printf("kanji:\t%s\n", full_word);
                }
            }
            
            free(full_word);
        }
    }

    if (print_yomi_katakana_to_hiragana(result) != 0)
    {
        mecab_destroy(mecab);
        free(subtitle_files);
        return 1;
    }

    mecab_destroy(mecab);
    free(subtitle_files);

    return 0;
}

char **get_path_to_files(int argc, char **argv)
{
    if (argc <= 1)
    {
        fprintf(stderr, "Error: path to file is missing.\n");
        return NULL;
    }

    char **file_paths = malloc((argc - 1) * sizeof(char *));
    if (file_paths == NULL)
    {
        fprintf(stderr, "Error: malloc file_paths failed.\n");
        return NULL;
    }

    int count_files = 0;
    for (int i = 1; i < argc; i++)
    {
        const char *extension = strrchr(argv[i], '.');
        if (extension == NULL || strcmp(extension, ".srt") != 0) {
            fprintf(stdout, "Warning: %s is not a .srt file\nSkipping that file.\n", argv[i]);
            continue;
        }
        file_paths[count_files] = argv[i];
        printf("Path to file #%d: %s\n", count_files + 1, file_paths[count_files]);
        count_files++;
    }

    if (count_files == 0)
    {
        fprintf(stderr, "Error: no valid .srt files provived.\n");
        free(file_paths);
        return NULL;
    }

    return file_paths;
}

static int is_fullwidth_katakana(wchar_t wc)
{
    return (L'\u30A1' <= wc) && (wc <= L'\u30FE');
}

static int is_halfwidth_katakana(wchar_t wc)
{
    return (L'\uff66' <= wc) && (wc <= L'\uff9d');
}

static wchar_t to_hiragana(wchar_t c)
{
    if (is_fullwidth_katakana(c))
    {
        return (c - 0x60);
    }
    else if (is_halfwidth_katakana(c))
    {
        return (c - 0xcf25);
    }
    return c;
}

static bool is_kanji(char *c)
{
    if ((L'\u4e00' <= c) && (c <= L'\u9fa5')) {
			return true;
		}
		if ((L'\u3005' <= c) && (c <= L'\u3007')) {
			return true;
		}
		return false;
}

static int print_yomi_katakana_to_hiragana(const char *yomi)
{
    size_t size_alloc = strlen(yomi) + 1;
    wchar_t *w_result = malloc(size_alloc * sizeof(wchar_t));
    if (w_result == NULL)
    {
        fprintf(stderr, "Error: w_result malloc failed.\n");
        return 1;
    }

    size_t converted = mbstowcs(w_result, yomi, size_alloc);
    if (converted == (size_t)-1)
    {
        fprintf(stderr, "Conversion mbstowcs failed.\n");
        free(w_result);
        return 1;
    }

    // Build a wide-character hiragana sentence
    wchar_t *hira_w = malloc(size_alloc * sizeof(wchar_t));
    if (hira_w == NULL)
    {
        fprintf(stderr, "Error: hira_w malloc failed.\n");
        free(w_result);
        return 1;
    }
    size_t out = 0;
    for (size_t i = 0; i < converted && out < size_alloc - 1; i++)
    {
        wchar_t katakana = w_result[i];
        if (katakana == L'\n' || katakana == L'\0')
        {
            continue;
        }
        wchar_t hiragana = to_hiragana(katakana);
        hira_w[out++] = hiragana;
    }
    hira_w[out] = L'\0';

    free(w_result);

    // Convert wide hiragana back to multibyte for printing
    size_t mbbuf = (out + 1) * MB_CUR_MAX;
    char *hira_mb = malloc(mbbuf);
    if (hira_mb == NULL)
    {
        fprintf(stderr, "Error: hira_mb malloc failed.\n");
        free(hira_w);
        return 1;
    }
    size_t mb_len = wcstombs(hira_mb, hira_w, mbbuf);
    if (mb_len == (size_t)-1)
    {
        fprintf(stderr, "Error: Conversion wcstombs failed.\n");
        free(hira_mb);
        free(hira_w);
        return 1;
    }

    printf("Full complete phrase in hiragana: %s\n", hira_mb);

    free(hira_w);
    free(hira_mb);

    return 0;
}

// Get the a node of Mecab by its index
char* get_mecab_node_field(const char* str, int n) {
    if (!str) return NULL;
    char* copy = strdup(str);
    if (!copy) return NULL;
    char* token = strtok(copy, ",");
    for (int i = 0; i < n && token; i++) {
        token = strtok(NULL, ",");
    }
    char* result = token ? strdup(token) : NULL;
    free(copy);
    return result;
}
