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
#include <ctype.h>
#include "../include/srt.h"
#include "../include/types.h"

Subtitle *parse_srt(const char *path, int *count)
{
    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    int capacity = INITIAL_SUB_CAPACITY;
    Subtitle *subs = calloc(capacity, sizeof(Subtitle));
    if (!subs) {
        fclose(f);
        return NULL;
    }

    char line[MAX_LINE];
    int state = 0;
    int idx = -1;
    *count = 0;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;

        if (state == 0) {
            if (isdigit((unsigned char)line[0])) state = 1;
        }
        else if (state == 1 && strstr(line, "-->")) {
            int h1,m1,s1,ms1,h2,m2,s2,ms2;
            if (sscanf(line, "%d:%d:%d,%d --> %d:%d:%d,%d",
                &h1,&m1,&s1,&ms1,&h2,&m2,&s2,&ms2) == 8) {

                if (*count >= capacity) {
                    capacity *= 2;
                    Subtitle *tmp = realloc(subs, capacity * sizeof(Subtitle));
                    if (!tmp) {
                        for (int j = 0; j < *count; j++)
                            free(subs[j].text);
                        free(subs);
                        fclose(f);
                        *count = 0;
                        return NULL;
                    }
                    subs = tmp;
                }

                subs[*count].start_ms = h1*MS_PER_HOUR + m1*MS_PER_MINUTE + s1*MS_PER_SECOND + ms1;
                subs[*count].end_ms   = h2*MS_PER_HOUR + m2*MS_PER_MINUTE + s2*MS_PER_SECOND + ms2;
                subs[*count].text = NULL;

                idx = *count;
                state = 2;
            }
        }
        else if (state == 2) {
            if (line[0] == 0) {
                (*count)++;
                idx = -1;
                state = 0;
            } else if (idx >= 0) {
                size_t old = subs[idx].text ? strlen(subs[idx].text) : 0;
                size_t add = strlen(line);
                char *tmp = realloc(subs[idx].text, old + add + 2);
                if (!tmp) {
                    continue;
                }
                subs[idx].text = tmp;
                if (old) { subs[idx].text[old] = '\n'; strcpy(subs[idx].text + old + 1, line); }
                else strcpy(subs[idx].text, line);
            }
        }
    }

    if (state == 2 && idx >= 0) (*count)++;
    fclose(f);
    return subs;
}
