PROJ_NAME := gbc_emulator
BUILD_DIR := build
SRC_DIR := src

SDL_SRC := lib/SDL
SDL_BUILD := build/sdl
SDL_PREFIX := $(CURDIR)/build/sdl-install

CC := clang
CPPFLAGS += -I$(SDL_PREFIX)/include
CFLAGS += -Wall -Werror
LDFLAGS += -L$(SDL_PREFIX)/lib
LDLIBS += -lSDL3

SOURCES := \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/gbc.c \
	$(SRC_DIR)/bus.c \
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
TARGET := $(BUILD_DIR)/$(PROJ_NAME)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all build lib clean

all: build

build: $(BUILD_DIR)/$(PROJ_NAME)

lib: 
	cmake -S $(SDL_SRC) -B $(SDL_BUILD) \
		-DCMAKE_INSTALL_PREFIX=$(SDL_PREFIX) \
		-DSDL_SHARED=ON \
		-DSDL_STATIC=OFF \
		-DSDL_TESTS=OFF
	cmake --build $(SDL_BUILD) --parallel
	cmake --install $(SDL_BUILD)

$(OBJECTS): | lib

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
