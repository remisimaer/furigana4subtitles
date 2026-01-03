# SPDX-License-Identifier: GPL-3.0-or-later
#
# Furigana4subtitles Makefile
#

CC		= gcc
CFLAGS		= -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes
CFLAGS		+= -Wold-style-definition -Werror=implicit-function-declaration
CFLAGS		+= -std=c99 -pedantic -g -Iinclude
LDFLAGS		= -lmecab

# Source files
SRCDIR		= src
SRCS		= $(SRCDIR)/utils.c \
		  $(SRCDIR)/mecab_helpers.c \
		  $(SRCDIR)/srt.c \
		  $(SRCDIR)/ass.c \
		  $(SRCDIR)/cli.c

# Object files
OBJDIR		= obj
OBJS		= $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# Targets
TARGETS		= furigana4subtitles furigana4subtitles-cli

.PHONY: all clean

all: $(TARGETS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

furigana4subtitles: $(OBJS) main.c
	$(CC) $(CFLAGS) $(OBJS) main.c -o $@ $(LDFLAGS)

furigana4subtitles-cli: $(OBJS) main_cli.c
	$(CC) $(CFLAGS) $(OBJS) main_cli.c -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJDIR) $(TARGETS)
