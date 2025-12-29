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
#include <ctype.h>
#include <mecab.h>

typedef struct {
    char *kanji;
    char *furigana;
    int byte_position;
    float x_position;
} FuriganaToken;

typedef struct {
    int start_ms;
    int end_ms;
    char *text;
    FuriganaToken *tokens;
    int token_count;
} Subtitle;

typedef struct {
    char *font_name;
    int main_font_size;
    int furigana_font_size;
    int screen_width;
    int screen_height;
    int baseline_y;
    int furigana_offset;
    float fixed_char_width;
    int line_spacing;
} FontConfig;

char **get_path_to_files(int argc, char **argv);
static bool is_kanji_char(wchar_t wc);
static wchar_t to_hiragana(wchar_t c);
char* get_mecab_node_field(const char* str, int n);
char* katakana_to_hiragana(const char *katakana);
Subtitle* parse_srt_file(const char *filepath, int *subtitle_count);
FuriganaToken* analyze_text_syllabic(mecab_t *mecab, const char *text, int *token_count);
int generate_ass_file(const char *input_path, Subtitle *subtitles, int count, FontConfig *config, mecab_t *mecab);
void calculate_positions_for_line(const char *line, FuriganaToken *tokens, int token_count, FontConfig *config);
void free_subtitles(Subtitle *subs, int count);
void ms_to_ass_time(int ms, char *buffer);
void calculate_positions(Subtitle *sub, FontConfig *config);
int count_chars(const char *text);
int split_furigana_by_kanji_count(const char *furigana, int kanji_count, char ***output);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    char **subtitle_files = get_path_to_files(argc, argv);
    if (!subtitle_files) return 1;

    FontConfig config = {
        .font_name = "MS Gothic",
        .main_font_size = 52,
        .furigana_font_size = 26,
        .screen_width = 1920,
        .screen_height = 1080,
        .baseline_y = 980,
        .furigana_offset = 50,
        .fixed_char_width = 52.0f,
        .line_spacing = 100
    };

    mecab_t *mecab = mecab_new2("");
    if (!mecab) {
        fprintf(stderr, "Error: Failed to initialize MeCab\n");
        free(subtitle_files);
        return 1;
    }

    for (int i = 0; subtitle_files[i]; i++) {
        printf("\nProcessing: %s\n", subtitle_files[i]);
        
        int subtitle_count = 0;
        Subtitle *subtitles = parse_srt_file(subtitle_files[i], &subtitle_count);
        
        if (!subtitles) {
            fprintf(stderr, "Error: Failed to parse %s\n", subtitle_files[i]);
            continue;
        }

        for (int j = 0; j < subtitle_count; j++) {
            subtitles[j].tokens = analyze_text_syllabic(mecab, subtitles[j].text, &subtitles[j].token_count);
            calculate_positions(&subtitles[j], &config);
        }

        if (generate_ass_file(subtitle_files[i], subtitles, subtitle_count, &config, mecab) == 0) {
            printf("Generated: %s.ass\n", subtitle_files[i]);
        }

        free_subtitles(subtitles, subtitle_count);
    }

    mecab_destroy(mecab);
    free(subtitle_files);
    return 0;
}

char **get_path_to_files(int argc, char **argv)
{
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s file1.srt file2.srt ...\n", argv[0]);
        return NULL;
    }

    char **file_paths = calloc(argc, sizeof(char *));
    if (!file_paths) return NULL;

    int count = 0;
    for (int i = 1; i < argc; i++) {
        const char *ext = strrchr(argv[i], '.');
        if (ext && strcmp(ext, ".srt") == 0) {
            file_paths[count++] = argv[i];
            printf("File #%d: %s\n", count, argv[i]);
        } else {
            fprintf(stdout, "Warning: %s is not a .srt file\n", argv[i]);
        }
    }

    if (count == 0) {
        fprintf(stderr, "Error: no valid .srt files provided.\n");
        free(file_paths);
        return NULL;
    }

    file_paths[count] = NULL;
    return file_paths;
}

static bool is_kanji_char(wchar_t wc)
{
    return (wc >= 0x4e00 && wc <= 0x9faf) ||
           (wc >= 0x3400 && wc <= 0x4dbf) ||
           (wc >= 0x3005 && wc <= 0x3007);
}

static wchar_t to_hiragana(wchar_t c)
{
    if (c >= 0x30A1 && c <= 0x30FE) return c - 0x60;
    if (c >= 0xff66 && c <= 0xff9d) return c - 0xcf25;
    return c;
}

char* katakana_to_hiragana(const char *katakana)
{
    if (!katakana) return NULL;
    
    size_t len = strlen(katakana) + 1;
    wchar_t *wbuf = malloc(len * sizeof(wchar_t));
    if (!wbuf) return NULL;

    size_t wlen = mbstowcs(wbuf, katakana, len);
    if (wlen == (size_t)-1) {
        free(wbuf);
        return NULL;
    }

    for (size_t i = 0; i < wlen; i++) {
        if (wbuf[i] != L'\n' && wbuf[i] != L'\0') {
            wbuf[i] = to_hiragana(wbuf[i]);
        }
    }

    char *result = malloc(len * MB_CUR_MAX);
    if (!result) {
        free(wbuf);
        return NULL;
    }

    if (wcstombs(result, wbuf, len * MB_CUR_MAX) == (size_t)-1) {
        free(wbuf);
        free(result);
        return NULL;
    }

    free(wbuf);
    return result;
}

char* get_mecab_node_field(const char* str, int n)
{
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

int count_chars(const char *text)
{
    if (!text) return 0;
    
    int count = 0;
    mbstate_t state = {0};
    size_t len = strlen(text);
    
    for (size_t i = 0; i < len;) {
        wchar_t wc;
        size_t bytes = mbrtowc(&wc, text + i, len - i, &state);
        if (bytes == (size_t)-1 || bytes == (size_t)-2 || bytes == 0) break;
        count++;
        i += bytes;
    }
    return count;
}

int split_furigana_by_kanji_count(const char *furigana, int kanji_count, char ***output)
{
    if (!furigana || kanji_count <= 0) return 0;
    
    int total_chars = count_chars(furigana);
    if (total_chars == 0) return 0;
    
    *output = malloc(kanji_count * sizeof(char*));
    if (!*output) return 0;
    
    int chars_per_kanji = (total_chars + kanji_count - 1) / kanji_count;
    mbstate_t state = {0};
    size_t len = strlen(furigana);
    int char_idx = 0, token_idx = 0;
    size_t token_start = 0;
    
    for (size_t i = 0; i < len && token_idx < kanji_count;) {
        size_t bytes = mbrlen(furigana + i, len - i, &state);
        if (bytes == (size_t)-1 || bytes == (size_t)-2 || bytes == 0) break;
        
        char_idx++;
        i += bytes;
        
        if (char_idx >= chars_per_kanji * (token_idx + 1) || i >= len) {
            size_t token_len = i - token_start;
            (*output)[token_idx] = malloc(token_len + 1);
            if ((*output)[token_idx]) {
                memcpy((*output)[token_idx], furigana + token_start, token_len);
                (*output)[token_idx][token_len] = '\0';
            }
            token_idx++;
            token_start = i;
        }
    }
    
    return token_idx;
}

FuriganaToken* analyze_text_syllabic(mecab_t *mecab, const char *text, int *token_count)
{
    *token_count = 0;
    if (!text || !mecab) return NULL;

    const mecab_node_t *node = mecab_sparse_tonode(mecab, text);
    if (!node) return NULL;

    int capacity = 100;
    FuriganaToken *tokens = malloc(capacity * sizeof(FuriganaToken));
    if (!tokens) return NULL;

    int byte_pos = 0;

    for (; node; node = node->next) {
        if (node->stat != MECAB_NOR_NODE && node->stat != MECAB_UNK_NODE) continue;
        
        char surface[256] = {0};
        strncpy(surface, node->surface, node->length < 255 ? node->length : 255);
        
        int kanji_count = 0;
        mbstate_t state = {0};
        size_t slen = strlen(surface);
        
        for (size_t i = 0; i < slen;) {
            wchar_t wc;
            size_t bytes = mbrtowc(&wc, surface + i, slen - i, &state);
            if (bytes == (size_t)-1 || bytes == (size_t)-2 || bytes == 0) break;
            if (is_kanji_char(wc)) kanji_count++;
            i += bytes;
        }
        
        if (kanji_count > 0) {
            char* yomi = get_mecab_node_field(node->feature, 7);
            
            if (yomi && strcmp(yomi, "*") != 0) {
                char *furigana_full = katakana_to_hiragana(yomi);
                
                if (furigana_full) {
                    char **furigana_parts = NULL;
                    int parts_count = split_furigana_by_kanji_count(furigana_full, kanji_count, &furigana_parts);
                    
                    int kanji_idx = 0;
                    mbstate_t state2 = {0};
                    
                    for (size_t i = 0; i < slen;) {
                        wchar_t wc;
                        size_t bytes = mbrtowc(&wc, surface + i, slen - i, &state2);
                        if (bytes == (size_t)-1 || bytes == (size_t)-2 || bytes == 0) break;
                        
                        if (is_kanji_char(wc)) {
                            if (*token_count >= capacity) {
                                capacity *= 2;
                                FuriganaToken *new_tokens = realloc(tokens, capacity * sizeof(FuriganaToken));
                                if (!new_tokens) break;
                                tokens = new_tokens;
                            }
                            
                            tokens[*token_count].kanji = malloc(bytes + 1);
                            memcpy(tokens[*token_count].kanji, surface + i, bytes);
                            tokens[*token_count].kanji[bytes] = '\0';
                            
                            tokens[*token_count].furigana = (kanji_idx < parts_count && furigana_parts[kanji_idx]) 
                                ? strdup(furigana_parts[kanji_idx]) : strdup("");
                            
                            tokens[*token_count].byte_position = byte_pos + i;
                            tokens[*token_count].x_position = 0.0f;
                            
                            (*token_count)++;
                            kanji_idx++;
                        }
                        i += bytes;
                    }
                    
                    if (furigana_parts) {
                        for (int j = 0; j < parts_count; j++) free(furigana_parts[j]);
                        free(furigana_parts);
                    }
                    free(furigana_full);
                }
            }
            free(yomi);
        }
        
        byte_pos += node->length;
    }

    return tokens;
}

void calculate_positions(Subtitle *sub, FontConfig *config)
{
    if (!sub || !sub->text || sub->token_count == 0) return;

    int total_chars = count_chars(sub->text);
    float start_x = (config->screen_width - total_chars * config->fixed_char_width) / 2.0f;
    
    mbstate_t state = {0};
    size_t text_len = strlen(sub->text);
    size_t byte_idx = 0;
    int char_idx = 0, token_idx = 0;
    
    while (byte_idx < text_len && token_idx < sub->token_count) {
        if (byte_idx == sub->tokens[token_idx].byte_position) {
            sub->tokens[token_idx].x_position = start_x + (char_idx * config->fixed_char_width);
            byte_idx += strlen(sub->tokens[token_idx].kanji);
            char_idx++;
            token_idx++;
        } else {
            wchar_t wc;
            size_t bytes = mbrtowc(&wc, sub->text + byte_idx, text_len - byte_idx, &state);
            if (bytes == (size_t)-1 || bytes == (size_t)-2 || bytes == 0) break;
            byte_idx += bytes;
            char_idx++;
        }
    }
}

void calculate_positions_for_line(const char *line, FuriganaToken *tokens, int token_count, FontConfig *config)
{
    if (!line || token_count == 0) return;

    int total_chars = count_chars(line);
    float start_x = (config->screen_width - total_chars * config->fixed_char_width) / 2.0f;

    mbstate_t state = {0};
    size_t text_len = strlen(line);
    size_t byte_idx = 0;
    int char_idx = 0;
    int token_idx = 0;

    while (byte_idx < text_len && token_idx < token_count) {
        if ((int)byte_idx == tokens[token_idx].byte_position) {
            tokens[token_idx].x_position = start_x + (char_idx * config->fixed_char_width);
            byte_idx += strlen(tokens[token_idx].kanji);
            char_idx++;
            token_idx++;
        } else {
            wchar_t wc;
            size_t bytes = mbrtowc(&wc, line + byte_idx, text_len - byte_idx, &state);
            if (bytes == (size_t)-1 || bytes == (size_t)-2 || bytes == 0) break;
            byte_idx += bytes;
            char_idx++;
        }
    }
}

Subtitle* parse_srt_file(const char *filepath, int *subtitle_count)
{
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filepath);
        return NULL;
    }

    int capacity = 100;
    Subtitle *subs = malloc(capacity * sizeof(Subtitle));
    if (!subs) {
        fclose(file);
        return NULL;
    }

    *subtitle_count = 0;
    char line[1024];
    int state = 0;
    int current = -1;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;

        if (state == 0) {
            if (line[0] && isdigit((unsigned char)line[0])) {
                state = 1;
            }
        }
        else if (state == 1 && strstr(line, "-->")) {
            int h1, m1, s1, ms1, h2, m2, s2, ms2;
            if (sscanf(line, "%d:%d:%d,%d --> %d:%d:%d,%d",
                      &h1, &m1, &s1, &ms1, &h2, &m2, &s2, &ms2) == 8) {

                if (*subtitle_count >= capacity) {
                    capacity *= 2;
                    Subtitle *new_subs = realloc(subs, capacity * sizeof(Subtitle));
                    if (!new_subs) break;
                    subs = new_subs;
                }

                subs[*subtitle_count].start_ms = h1 * 3600000 + m1 * 60000 + s1 * 1000 + ms1;
                subs[*subtitle_count].end_ms = h2 * 3600000 + m2 * 60000 + s2 * 1000 + ms2;
                subs[*subtitle_count].text = NULL;
                subs[*subtitle_count].tokens = NULL;
                subs[*subtitle_count].token_count = 0;
                current = *subtitle_count;
                state = 2;
            }
        }
        else if (state == 2) {
            if (strlen(line) == 0) {
                // end of block
                if (current >= 0) (*subtitle_count)++;
                current = -1;
                state = 0;
            } else if (current >= 0) {
                // append line (keep newline between lines)
                size_t oldlen = subs[current].text ? strlen(subs[current].text) : 0;
                size_t addlen = strlen(line);
                size_t need = oldlen + (oldlen ? 1 : 0) + addlen + 1;
                char *newtext = realloc(subs[current].text, need);
                if (!newtext) {
                    free(subs[current].text);
                    subs[current].text = NULL;
                    break;
                }
                if (oldlen) newtext[oldlen] = '\n';
                memcpy(newtext + oldlen + (oldlen ? 1 : 0), line, addlen);
                newtext[oldlen + (oldlen ? 1 : 0) + addlen] = '\0';
                subs[current].text = newtext;
            }
        }
    }

    if (state == 2 && current >= 0) {
        (*subtitle_count)++;
    }

    fclose(file);
    return subs;
}

void ms_to_ass_time(int ms, char *buffer)
{
    int h = ms / 3600000;
    ms %= 3600000;
    int m = ms / 60000;
    ms %= 60000;
    int s = ms / 1000;
    int cs = (ms % 1000) / 10;
    sprintf(buffer, "%d:%02d:%02d.%02d", h, m, s, cs);
}

int generate_ass_file(const char *input_path, Subtitle *subtitles, int count, FontConfig *config, mecab_t *mecab)
{
    char output_path[512];
    strncpy(output_path, input_path, sizeof(output_path) - 5);
    char *ext = strrchr(output_path, '.');
    if (ext) *ext = '\0';
    strcat(output_path, ".ass");

    FILE *file = fopen(output_path, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot create %s\n", output_path);
        return 1;
    }

    fprintf(file, "[Script Info]\n");
    fprintf(file, "Title: Furigana Subtitles\n");
    fprintf(file, "ScriptType: v4.00+\n");
    fprintf(file, "PlayResX: %d\n", config->screen_width);
    fprintf(file, "PlayResY: %d\n\n", config->screen_height);

    fprintf(file, "[V4+ Styles]\n");
    fprintf(file, "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n");
    fprintf(file, "Style: Main,%s,%d,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,0,2,10,10,10,1\n",
            config->font_name, config->main_font_size);
    fprintf(file, "Style: Furigana,%s,%d,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,1,0,5,10,10,10,1\n\n",
            config->font_name, config->furigana_font_size);

    fprintf(file, "[Events]\n");
    fprintf(file, "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n");

    for (int i = 0; i < count; i++) {
        char start_time[20], end_time[20];
        ms_to_ass_time(subtitles[i].start_ms, start_time);
        ms_to_ass_time(subtitles[i].end_ms, end_time);

        if (!subtitles[i].text) continue;

        // split subtitle text into lines
        char *copy = strdup(subtitles[i].text);
        if (!copy) continue;
        int lines_cap = 8;
        char **lines = malloc(lines_cap * sizeof(char*));
        int lines_count = 0;
        char *saveptr = NULL;
        char *tok = strtok_r(copy, "\n", &saveptr);
        while (tok) {
            if (lines_count >= lines_cap) {
                lines_cap *= 2;
                lines = realloc(lines, lines_cap * sizeof(char*));
            }
            lines[lines_count++] = strdup(tok);
            tok = strtok_r(NULL, "\n", &saveptr);
        }

        for (int li = 0; li < lines_count; li++) {
            char *line_text = lines[li];
            float center_x = config->screen_width / 2.0f;
            int line_spacing = config->line_spacing;
            // compute Y so that first line (li==0) is above line 1, last line is baseline_y
            int y = config->baseline_y - ((lines_count - 1 - li) * line_spacing);

            fprintf(file, "Dialogue: 0,%s,%s,Main,,0,0,0,,{\\pos(%.1f,%d)\\an5}%s\n",
                    start_time, end_time, center_x, y,
                    line_text ? line_text : "");

            int token_count = 0;
            FuriganaToken *tokens = analyze_text_syllabic(mecab, line_text, &token_count);
            if (token_count > 0 && tokens) {
                calculate_positions_for_line(line_text, tokens, token_count, config);
                int furigana_y = y - config->furigana_offset;
                for (int tj = 0; tj < token_count; tj++) {
                    FuriganaToken *t = &tokens[tj];
                        float x = t->x_position + (config->fixed_char_width / 2.0f);
                        // ensure furigana text is centered on the position (avoid overlap)
                        fprintf(file, "Dialogue: 1,%s,%s,Furigana,,0,0,0,,{\\pos(%.1f,%d)\\an5}%s\n",
                            start_time, end_time, x, furigana_y,
                            t->furigana ? t->furigana : "");
                    free(t->kanji);
                    free(t->furigana);
                }
                free(tokens);
            }

            free(line_text);
        }

        free(lines);
        free(copy);
    }

    fclose(file);
    return 0;
}

void free_subtitles(Subtitle *subs, int count)
{
    if (!subs) return;
    
    for (int i = 0; i < count; i++) {
        free(subs[i].text);
        if (subs[i].tokens) {
            for (int j = 0; j < subs[i].token_count; j++) {
                free(subs[i].tokens[j].kanji);
                free(subs[i].tokens[j].furigana);
            }
            free(subs[i].tokens);
        }
    }
    free(subs);
}