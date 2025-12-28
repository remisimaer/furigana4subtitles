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
#include <mecab.h>

char **get_path_to_files(int argc, char **argv);
static int is_fullwidth_katakana(wchar_t wc);
static int is_halfwidth_katakana(wchar_t wc);
static wchar_t to_hiragana(wchar_t c);
static int print_yomi_katakana_to_hiragana(const char *yomi);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    // Retrieve the path to the .str passed as argument
    char **subtitle_files = get_path_to_files(argc, argv);
    if (subtitle_files == NULL)
    {
        return 1;
    }

    // Parse the .str file

    // Initialize Mecab
    mecab_t *mecab = mecab_new2("-Oyomi");
    if (mecab == NULL)
    {
        fprintf(stderr, "Error: mecab_new2 failed.\n");
        free(subtitle_files);
        return 1;
    }

    // Example text to analyze
    const char *input = "頑張って";
    const char *result = mecab_sparse_tostr(mecab, input);
    if (result == NULL)
    {
        fprintf(stderr, "Error: mecab_sparse_tostr returned NULL.\n");
        mecab_destroy(mecab);
        free(subtitle_files);
        return 1;
    }

    printf("Full complete phrase in katakana: %s\n", result);

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

    for (int i = 1; i < argc; i++)
    {
        file_paths[i - 1] = argv[i];
        printf("Path to file #%d: %s\n", i, file_paths[i - 1]);
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
