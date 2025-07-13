ifdef $(GENDEV)
ROOTDIR = $(GENDEV)
else
ROOTDIR = /mnt/c/games/opt/toolchains/sega
endif

SRC_DIR = source
SHARED_DIR = ../shared/source

TARGET = view_model
BUILD = build
SOURCES = $(SRC_DIR) $(SRC_DIR)/data $(SHARED_DIR) $(SHARED_DIR)/engine $(SHARED_DIR)/system $(SHARED_DIR)/asm

LDSCRIPTSDIR = $(ROOTDIR)/ldscripts

PREFIX = $(ROOTDIR)/sh-elf/bin/sh-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJC = $(PREFIX)objcopy

CC_VER := $(shell $(CC) -dumpversion)

LIBPATH = -L$(ROOTDIR)/sh-elf/lib -L$(ROOTDIR)/sh-elf/lib/gcc/sh-elf/$(CC_VER) -L$(ROOTDIR)/sh-elf/sh-elf/lib
INCPATH = -I$(SRC_DIR) -I$(SHARED_DIR) -I$(ROOTDIR)/sh-elf/include -I$(ROOTDIR)/sh-elf/sh-elf/include

CCFLAGS = -m2 -mb -Wall -Wextra -fomit-frame-pointer -fno-builtin -ffunction-sections -fdata-sections -gdwarf-4
CCFLAGS += -fno-align-loops -fno-align-functions -fno-align-jumps -fno-align-labels
CCFLAGS += -D__32X__ -DMARS

HWFLAGS := $(CCFLAGS)
HWFLAGS += -Os -fno-lto

CCFLAGS += -g3 -O2 -Wno-maybe-uninitialized -funroll-loops -lto

LDFLAGS = -T $(LDSCRIPTSDIR)/mars.ld -Wl,-Map=$(BUILD)/output.map -nostdlib -Wl,--gc-sections --specs=nosys.specs -flto
ASFLAGS = --big

DD = dd
RM = rm -f

LIBS = $(LIBPATH) -lc -lgcc -lgcc-Os-4-200 -lnosys
OBJS = $(BUILD)/crt0.o

FILES_AS = $(foreach dir, $(SOURCES), $(filter-out $(wildcard $(dir)/crt0.s), $(wildcard $(dir)/*.s)))
#FILES_AS = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.s))
FILES_CC = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))

OBJS += $(addprefix $(BUILD)/, $(notdir $(FILES_AS:%.s=%.o)))
OBJS += $(addprefix $(BUILD)/, $(notdir $(FILES_CC:%.c=%.o)))

#$(info OBJS: $(OBJS))
#print:
#	@echo $(OBJS)

VPATH := $(SOURCES)

.PHONY: all clean

all: $(BUILD) m68k.bin $(TARGET).32x

$(BUILD):
	@[ -d $@ ] || mkdir -p $@

m68k.bin:
	make -C $(SHARED_DIR)/system/src-md

$(TARGET).32x: $(TARGET).elf
	@echo $@
	@$(OBJC) -O binary $< temp.bin
	$(DD) if=temp.bin of=$@ bs=128K conv=sync

$(TARGET).elf: $(OBJS)
	@echo $@
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET).elf

$(BUILD)/crt0.o: $(SHARED_DIR)/system/crt0.s $(SHARED_DIR)/system/src-md/m68k.bin
	@echo $<
	@$(AS) $(ASFLAGS) -o $@ $<

#$(BUILD)/hw_32x.o: $(SHARED_DIR)/system/hw_32x.c
#	@echo $<
#	@$(CC) $(HWFLAGS) $(INCPATH) -c $< -o $@

$(BUILD)/%.o: %.c
	@echo $<
	@$(CC) $(CCFLAGS) $(INCPATH) -c $< -o $@

#$(BUILD)/%.o: %.s
#	@echo $<
#	@$(AS) $(ASFLAGS) $(INCPATH) -c $< -o $@

$(BUILD)/%.o: %.s
	@echo $<
	@$(CC) -x assembler-with-cpp -m2 -mb $(INCPATH) -c $< -o $@

clean:
#	make clean -C $(SHARED_DIR)/system/src-md
	$(RM) $(BUILD)/* $(TARGET).32x $(TARGET).elf temp.bin
