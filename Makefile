CC = gcc
CFLAGS = -g -Iinclude -Wall -Wextra
LDFLAGS = -lmecab
SRCS = src/utils.c src/mecab_helpers.c src/srt.c src/ass.c

all: main

main: $(SRCS) main.c
	$(CC) $(CFLAGS) $(SRCS) main.c -o furigana4subtitles $(LDFLAGS)

clean:
	rm -f furigana4subtitles *.o *.ass
