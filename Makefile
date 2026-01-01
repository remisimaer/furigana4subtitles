CC = gcc
CFLAGS = -g -Iinclude -Wall -Wextra
LDFLAGS = -lmecab
SRCS = src/utils.c src/mecab_helpers.c src/srt.c src/ass.c

all: furigana4subtitles

furigana4subtitles: $(SRCS) main.c
	$(CC) $(CFLAGS) $(SRCS) main.c -o furigana4subtitles $(LDFLAGS)

cli: $(SRCS) main_cli.c
	$(CC) $(CFLAGS) $(SRCS) main_cli.c -o furigana4subtitles-cli $(LDFLAGS)

clean:
	rm -f furigana4subtitles furigana4subtitles-cli *.o *.ass
