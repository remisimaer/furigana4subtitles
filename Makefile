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
	rm -f *.ass

# Check coding style (requires checkpatch.pl from Linux kernel)
checkpatch:
	@if command -v checkpatch.pl >/dev/null 2>&1; then \
		checkpatch.pl --no-tree -f $(SRCS) main.c main_cli.c include/*.h; \
	else \
		echo "checkpatch.pl not found in PATH"; \
	fi

# Static analysis with sparse (if available)
sparse:
	@if command -v sparse >/dev/null 2>&1; then \
		sparse $(CFLAGS) $(SRCS) main.c main_cli.c; \
	else \
		echo "sparse not found in PATH"; \
	fi
