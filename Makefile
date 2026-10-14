PROJ_NAME := gbcemu
BUILD_DIR := build
SRC_DIR := src

CC := clang
CFLAGS := -Wall -Werror

SOURCES := \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/gbc.c \
	$(SRC_DIR)/mem/bus.c \
	$(SRC_DIR)/mem/cartridge.c \
	$(SRC_DIR)/mem/mbc.c \
	$(SRC_DIR)/sm83/sm83.c \
	$(SRC_DIR)/sm83/sm83_alu.c \
	$(SRC_DIR)/sm83/sm83_opcodes.c \
	$(SRC_DIR)/ppu/ppu.c \
	$(SRC_DIR)/ppu/ppu_fifo.c \
	$(SRC_DIR)/io.c \
	$(SRC_DIR)/apu.c \
	$(SRC_DIR)/joypad.c \
	$(SRC_DIR)/timer.c

OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: build clean

build: $(BUILD_DIR)/$(PROJ_NAME)

$(BUILD_DIR)/$(PROJ_NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
