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
#include "../include/ass.h"
#include "../include/types.h"
#include "../include/utils.h"
#include "../include/mecab_helpers.h"

void generate_ass(const char *input, Subtitle *subs, int count, FontConfig *cfg, mecab_t *mecab)
{
    char out[MAX_PATH];
    snprintf(out, sizeof(out), "%s.ass", input);

    FILE *f = fopen(out, "w");
    if (!f) {
        perror("fopen");
        return;
    }

    fprintf(f,
        "[Script Info]\nScriptType: v4.00+\nPlayResX: %d\nPlayResY: %d\n\n",
        cfg->screen_w, cfg->screen_h);

    fprintf(f,
        "[V4+ Styles]\n"
        "Format: Name,Fontname,Fontsize,PrimaryColour,OutlineColour,BackColour,Bold,Italic,BorderStyle,Outline,Shadow,Alignment,MarginL,MarginR,MarginV,Effect,Encoding\n"
        "Style: Main,%s,%d,&H00FFFFFF,&H00000000,&H00000000,0,0,1,2,0,5,10,10,10,\n"
        "Style: Furi,%s,%d,&H00FFFFFF,&H00000000,&H00000000,0,0,1,1,0,5,10,10,10,\n\n",
        cfg->font_name, cfg->main_size, cfg->font_name, cfg->furigana_size);

    fprintf(f, "[Events]\nFormat: Layer,Start,End,Style,Name,MarginL,MarginR,MarginV,Effect,Text\n");

    for (int i = 0; i < count; i++) {
        char ts[MAX_TIME], te[MAX_TIME];
        format_ass_time(subs[i].start_ms, ts);
        format_ass_time(subs[i].end_ms, te);

        char *copy = strdup(subs[i].text);
        char *save = NULL;
        char *line = strtok_r(copy, "\n", &save);
        int line_idx = 0;

        while (line) {
            int y = cfg->baseline_y - line_idx * cfg->line_spacing;
            fprintf(f, "Dialogue: 0,%s,%s,Main,,0,0,0,,{\\pos(%.1f,%d)\\an5}%s\n",
                ts, te, cfg->screen_w / 2.0f, y, line);

            int tcount = 0;
            FuriganaToken *tokens = analyze_text_with_mecab(mecab, line, &tcount);
            if (tokens) calculate_token_positions(line, tokens, tcount, cfg);

            for (int t = 0; t < tcount; t++) {
                fprintf(f, "Dialogue: 1,%s,%s,Furi,,0,0,0,,{\\pos(%.1f,%d)\\an5}%s\n",
                    ts, te, tokens[t].x, y - cfg->furigana_offset, tokens[t].reading);
                free(tokens[t].reading);
            }
            free(tokens);

            line = strtok_r(NULL, "\n", &save);
            line_idx++;
        }

        free(copy);
    }

    fclose(f);
}
