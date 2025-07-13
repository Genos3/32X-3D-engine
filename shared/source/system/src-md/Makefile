ifdef $(GENDEV)
ROOTDIR = $(GENDEV)
else
ROOTDIR = /mnt/c/games/opt/toolchains/sega
endif

TARGET = m68k
BUILD = build

LDSCRIPTSDIR = $(ROOTDIR)/ldscripts

PREFIX = $(ROOTDIR)/m68k-elf/bin/m68k-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJC = $(PREFIX)objcopy

CC_VER := $(shell $(CC) -dumpversion)

LIBPATH = -L$(ROOTDIR)/m68k-elf/lib -L$(ROOTDIR)/m68k-elf/lib/gcc/m68k-elf/$(CC_VER) -L$(ROOTDIR)/m68k-elf/m68k-elf/lib
INCPATH = -I$. -I$(ROOTDIR)/m68k-elf/include -I$(ROOTDIR)/m68k-elf/m68k-elf/include

CCFLAGS = -m68000 -Wall -O2 -c -fomit-frame-pointer
ASFLAGS = -m68000 --register-prefix-optional
LINKFLAGS = -T $(LDSCRIPTSDIR)/mars-md.ld -Wl,-Map=$(BUILD)/output.map -nostdlib

DD = dd
RM = rm -f

LIBS = $(LIBPATH) -lc -lgcc -lnosys
OBJS = $(BUILD)/crt0.o $(BUILD)/main.o $(BUILD)/kos.o $(BUILD)/cd.o

#FILES_AS = $(wildcard $(CURDIR)/*.s)
#FILES_CC = $(wildcard $(CURDIR)/*.c)

#OBJS =  $(addprefix $(BUILD)/, $(notdir $(FILES_AS:%.s=%.o)))
#OBJS += $(addprefix $(BUILD)/, $(notdir $(FILES_CC:%.c=%.o)))

.PHONY: all clean

all: $(BUILD) $(TARGET).bin

$(BUILD):
	@[ -d $@ ] || mkdir -p $@

$(TARGET).bin: $(TARGET).elf
	@$(OBJC) -O binary $< $(TARGET).bin
	@echo $(TARGET).bin

$(TARGET).elf: $(OBJS)
	@$(CC) $(LINKFLAGS) $(OBJS) $(LIBS) -o $(TARGET).elf
	@echo $(TARGET).elf

$(BUILD)/%.o: %.c
	@echo $<
	@$(CC) $(CCFLAGS) $(INCPATH) $< -o $@

$(BUILD)/%.o: %.s
	@echo $<
	@$(AS) $(ASFLAGS) $(INCPATH) $< -o $@

clean:
	$(RM) $(BUILD)/*.o *.bin *.elf $(BUILD)/output.map
