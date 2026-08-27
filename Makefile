PROJ_NAME = gbcemu
BUILD_DIR = build
SRC_DIR = src

CC = clang
CFLAGS = -Wall -Werror

TARGETS = main cartridge sm83/sm83 sm83/sm83_opcodes sm83/bus

.PHONY: build clean

build:
	$(CC) $(CFLAGS) $(foreach src, $(TARGETS), $(SRC_DIR)/$(src).c) -o $(PROJ_NAME)
	mkdir -p $(BUILD_DIR)
	mv $(foreach obj, $(TARGETS), $(obj).o) $(PROJ_NAME) $(BUILD_DIR)/

clean:
	rm -rf build
