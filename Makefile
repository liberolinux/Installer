CC ?= gcc
CFLAGS ?= -O2

PROJECT := libero-installer
SRC_DIR := src
INC_DIR := include

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)

CFLAGS += -Wall -Wextra -Wpedantic -std=c17 -g -I$(INC_DIR) -D_GNU_SOURCE
LDFLAGS += -lncurses

.PHONY: all clean format

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROJECT)

