CC = gcc
CFLAGS = -g -Iinclude -Wall -Wextra
LDFLAGS = -lmecab -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
SRCS = src/utils.c src/mecab_helpers.c src/srt.c src/ass.c

all: furigana4subtitles

furigana4subtitles: $(SRCS) main.c
	$(CC) $(CFLAGS) $(SRCS) main.c -o furigana4subtitles $(LDFLAGS)

clean:
	rm -f furigana4subtitles *.o *.ass
