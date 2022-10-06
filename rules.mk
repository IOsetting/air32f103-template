# 'make V=1' will show all compiler calls.
V	?= 0
ifeq ($(V),0)
Q	:= @
NULL	:= 2>/dev/null
endif

PREFIX		?= $(ARM_TOOCHAIN)/arm-none-eabi-
CC			= $(PREFIX)gcc
AS			= $(PREFIX)as
LD			= $(PREFIX)ld
OBJCOPY		= $(PREFIX)objcopy
# `$(shell pwd)` or `.`, both works
TOP			= $(shell pwd)
BDIR		= $(TOP)/$(BUILD_DIR)

# For each direcotry, add it to csources
CSOURCES := $(foreach dir, $(CDIRS), $(shell find $(TOP)/$(dir) -maxdepth 1 -name '*.c'))
# Add single c source files to csources
CSOURCES += $(addprefix $(TOP)/, $(CFILES))
# Then assembly source folders and files
ASOURCES := $(foreach dir, $(ADIRS), $(shell find $(TOP)/$(dir) -maxdepth 1 -name '*.s'))
ASOURCES += $(addprefix $(TOP)/, $(AFILES))

# Fill object files with c and asm files (keep source directory structure)
OBJS = $(CSOURCES:$(TOP)/%.c=$(BDIR)/%.o)
OBJS += $(ASOURCES:$(TOP)/%.s=$(BDIR)/%.o)
# d files for detecting h file changes
DEPS=$(CSOURCES:$(TOP)/%.c=$(BDIR)/%.d)

# Global compile flags
CFLAGS		= -O2 -g -Wall -W
ASFLAGS		= -g -Wa,--warn

# Arch and target specified flags
OPT			?= -Os
CSTD		?= -std=c99
ARCH_FLAGS	:= -mthumb -mcpu=cortex-m3

# c flags
TGT_CFLAGS 	+= $(OPT) $(CSTD) $(ARCH_FLAGS) $(addprefix -D, $(LIB_FLAGS)) -ggdb3
# asm flags
TGT_ASFLAGS += $(ARCH_FLAGS)
# ld flags
TGT_LDFLAGS += $(ARCH_FLAGS) -specs=nano.specs -specs=nosys.specs -static -lc -lm \
				-Wl,-Map=$(BDIR)/$(PROJECT).map \
				-Wl,--gc-sections \
				-Wl,--print-memory-usage
# include paths
TGT_INCFLAGS := $(addprefix -I $(TOP)/, $(INCLUDES))


.PHONY: all clean flash echo

all: $(BDIR)/$(PROJECT).elf $(BDIR)/$(PROJECT).bin

# for debug
echo:
	$(info 1. $(AFILES))
	$(info 2. $(ASOURCES))
	$(info 3. $(CSOURCES))
	$(info 4. $(OBJS))
	$(info 5. $(TGT_INCFLAGS))

# include d files without non-exist warning
-include $(DEPS)

# Compile c to obj -- should be `$(BDIR)/%.o: $(TOP)/%.c`, but since $(TOP) is base folder so non-path also works
$(BDIR)/%.o: %.c
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_INCFLAGS) -o $@ -c $< -MD -MF $(BDIR)/$*.d -MP

$(BDIR)/%.o: %.s
	@printf "  AS\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_ASFLAGS) $(ASFLAGS) -o $@ -c $<

$(BDIR)/$(PROJECT).elf: $(OBJS) $(TOP)/$(LDSCRIPT)
	@printf "  LD\t$@\n"
	$(Q)$(CC) $(TGT_LDFLAGS) -T$(TOP)/$(LDSCRIPT) $(OBJS) -o $@

%.bin: %.elf
	@printf "  OBJCP\t$@\n"
	$(Q)$(OBJCOPY) -O binary  $< $@

clean:
	rm -rf $(BDIR)/*

flash:
ifeq ($(FLASH_PROGRM),st-flash)
	$(ST_FLASH) --reset write $(BDIR)/$(PROJECT).bin 0x8000000
else ifeq ($(FLASH_PROGRM),jlink)
	$(JLINKEXE) -device $(JLINK_DEVICE) -if swd -speed 4000 -CommanderScript $(TOP)/Misc/flash.jlink
else
    @echo "FLASH_PROGRM is invalid\n"
endif
