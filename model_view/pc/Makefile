# CC = gcc
CC = C:\games\msys64\mingw64\bin\gcc

SRC_DIR = ../source
SHARED_DIR = ../../shared/source

CFLAGS = -O2 -DPC -std=c99 -mwindows -Wall -Wfatal-errors -Wno-maybe-uninitialized -fms-extensions -I$(SRC_DIR) -I$(SHARED_DIR)
LDFLAGS = -lgdi32 -lwinmm

BUILD = build
SRCS = $(SRC_DIR) $(SRC_DIR)/data $(SRC_DIR)/pc $(SHARED_DIR) $(SHARED_DIR)/pc $(SHARED_DIR)/engine

FILES_CC = $(foreach dir, $(SRCS), $(wildcard $(dir)/*.c))

OBJS = $(addprefix $(BUILD)/, $(notdir $(FILES_CC:%.c=%.o)))

# SRCS += $(foreach dir, $(wildcard data/*), $(wildcard $(dir)/*.c))

# OBJS = $(SRCS:%.c=$(BUILD)/%.o)

TARGET = 3d_model

.PHONY: all clean

all: $(TARGET)

VPATH = $(SRCS)

$(TARGET): $(OBJS)
	@echo linking
	@$(CC) $^ $(LDFLAGS) -o $@
	@echo built $(TARGET)

$(BUILD)/%.o: %.c | $(BUILD)
	@echo $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	@mkdir -p $@

clean:
	rm -rf $(BUILD)/* $(TARGET)
