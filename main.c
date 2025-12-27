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

#define BUFFER_SIZE 1024

char *get_first_argument(int argc, char **argv);
static int is_fullwidth_katakana(wchar_t wc);
static int is_halfwidth_katakana(wchar_t wc);
static wchar_t to_hiragana(wchar_t c);
static int print_yomi_katakana_to_hiragana(const char *yomi);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    // Retrieve the path to the .str passed as argument
    char *argument_dot_str_subtitle = get_first_argument(argc, argv);
    if (argument_dot_str_subtitle == NULL)
    {
        return 1;
    }

    // Parse the .str file
    

    // Initialize Mecab
    mecab_t *mecab = mecab_new2("-Oyomi");
    if (mecab == NULL)
    {
        fprintf(stderr, "mecab_new2 failed\n");
        return 1;
    }

    // Example text to analyze
    const char *input = "頑張って";
    const char *result = mecab_sparse_tostr(mecab, input);
    if (result == NULL)
    {
        fprintf(stderr, "mecab_sparse_tostr returned NULL\n");
        mecab_destroy(mecab);
        return 1;
    }

    printf("Full complete phrase in katakana: %s\n", result);

    if (print_yomi_katakana_to_hiragana(result) != 0) {
        mecab_destroy(mecab);
        return 1;
    }

    mecab_destroy(mecab);

    return 0;
}

char *get_first_argument(int argc, char **argv)
{
    if (argc > 1)
    {
        printf("Argument: %s\n", argv[1]);
        return argv[1];
    }
    else
    {
        fprintf(stderr, "Argument is missing\n");
        return NULL;
    }
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
    wchar_t w_result[BUFFER_SIZE];
    size_t converted = mbstowcs(w_result, yomi, BUFFER_SIZE);
    if (converted == (size_t)-1)
    {
        fprintf(stderr, "Conversion mbstowcs failed\n");
        return -1;
    }

    // Build a wide-character hiragana sentence
    wchar_t hira_w[BUFFER_SIZE];
    size_t out = 0;
    for (size_t i = 0; i < converted && out < BUFFER_SIZE - 1; i++)
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

    // Convert wide hiragana back to multibyte for printing
    char hira_mb[BUFFER_SIZE];
    size_t mb_len = wcstombs(hira_mb, hira_w, BUFFER_SIZE);
    if (mb_len == (size_t)-1)
    {
        fprintf(stderr, "Conversion wcstombs failed\n");
        return -1;
    }

    printf("Full complete phrase in hiragana: %s\n", hira_mb);

    return 0;
}
