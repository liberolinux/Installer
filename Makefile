CC ?= gcc
CFLAGS ?= -O2
PKG_CONFIG ?= pkg-config

PROJECT := libero-installer
SRC_DIR := src
INC_DIR := include

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)

NCURSES_CFLAGS :=
NCURSES_LIBS :=

ifneq ($(shell command -v $(PKG_CONFIG) >/dev/null 2>&1 && echo yes),)
NCURSES_PKG := $(shell $(PKG_CONFIG) --exists ncursesw >/dev/null 2>&1 && echo ncursesw)
ifeq ($(NCURSES_PKG),)
NCURSES_PKG := $(shell $(PKG_CONFIG) --exists ncurses >/dev/null 2>&1 && echo ncurses)
endif
ifneq ($(NCURSES_PKG),)
NCURSES_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(NCURSES_PKG))
NCURSES_LIBS := $(shell $(PKG_CONFIG) --libs $(NCURSES_PKG))
endif
endif

ifeq ($(strip $(NCURSES_LIBS)),)
NCURSES_LIBS := -lncurses
endif

CFLAGS += -Wall -Wextra -Wpedantic -std=c17 -g -I$(INC_DIR) -D_GNU_SOURCE $(NCURSES_CFLAGS)
LDFLAGS += $(NCURSES_LIBS)

.PHONY: all clean format

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROJECT)
