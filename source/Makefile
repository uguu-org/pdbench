# Makefile for PDBenchmark executables.
#
# Tools needed:
# - PlaydateSDK from https://play.date/dev
#
# - ARM GNU toolchain, available here:
#   https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain
#
#   This is the one I used:
#   arm-gnu-toolchain-14.2.rel1-mingw-w64-x86_64-arm-none-eabi.exe
#
# - MingW, available through Cygwin.
#
#   Needed to build Windows binaries.
#
# - Ruby with libpng-ruby, ImageMagick.
#
#   Used to build title card image.


# ......................................................................
# {{{ Settings.

# Check environment variables.
# PLAYDATE_SDK_PATH is where Playdate SDK is installed, e.g.:
# "C:\users\omoikane\Documents\PlaydateSDK"
#
# TL_PATH is where ARM SDK is installed, e.g.:
# "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin"
ifeq ($(PLAYDATE_SDK_PATH),)
$(error need to set PLAYDATE_SDK_PATH environment)
endif
ifeq ($(TL_PATH),)
$(error need to set TL_PATH environment)
endif

INC_PATH = "$(PLAYDATE_SDK_PATH)/C_API"

# Tool settings to build for windows simulator, using MingW on Cygwin.
SIM_PREFIX = x86_64-w64-mingw32-
SIM_EXT = dll
SIM_BUILD_DIR = sim_build
SIM_CC = $(SIM_PREFIX)gcc
SIM_CP = $(SIM_PREFIX)objcopy
SIM_AS = $(SIM_PREFIX)gcc -x assembler-with-cpp
SIM_STRIP = $(SIM_PREFIX)strip
SIM_TOOL_ROOT = /usr/x86_64-w64-mingw32/sys-root/mingw

SIM_ASFLAGS =
SIM_CFLAGS = \
	-DTARGET_SIMULATOR=1 -DTARGET_EXTENSION=1 \
	-O2 -Wall -Wstrict-prototypes -Wno-unknown-pragmas -Wdouble-promotion
SIM_LFLAGS = -shared -fPIC

# Tool settings to build for the device.
DEVICE_PREFIX = "$(TL_PATH)/arm-none-eabi-"
DEVICE_BUILD_DIR = device_build
DEVICE_CC = $(DEVICE_PREFIX)gcc
DEVICE_CP = $(DEVICE_PREFIX)objcopy
DEVICE_AS = $(DEVICE_PREFIX)gcc -x assembler-with-cpp
DEVICE_STRIP = $(DEVICE_PREFIX)strip

HEAP_SIZE = 8388208
STACK_SIZE = 61800
MCFLAGS = -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__FPU_USED=1

DEVICE_ASFLAGS = \
	-x assembler-with-cpp \
	-D__HEAP_SIZE=$(HEAP_SIZE) \
	-D__STACK_SIZE=$(STACK_SIZE)
DEVICE_CFLAGS = \
	-DNDEBUG \
	-DTARGET_PLAYDATE=1 -DTARGET_EXTENSION=1 \
	-O2 -Wall -Wno-unknown-pragmas -Wdouble-promotion \
	-gdwarf-2 \
	-falign-functions=16 -fomit-frame-pointer \
	-fverbose-asm \
	-ffunction-sections -fdata-sections \
	-fno-common -fno-exceptions \
	-mword-relocations \
	$(MCFLAGS)
DEVICE_LFLAGS = \
	-nostartfiles \
	$(MCFLAGS) \
	-Tlink_map.ld \
	"-Wl,-Map=$(DEVICE_BUILD_DIR)/game.map,--cref,--gc-sections,--no-warn-mismatch,--emit-relocs" \
	--entry eventHandlerShim

BUILD_DIR = build

# }}}

# ......................................................................
# {{{ Build targets.

# Toplevel target.
all: \
	$(SIM_BUILD_DIR)/pdex.$(SIM_EXT) \
	$(DEVICE_BUILD_DIR)/pdex.elf \
	$(BUILD_DIR)/launcher/card.png \
	$(BUILD_DIR)/launcher/icon.png

# Compile rules.
SRC = main.c setup.c arith.c memory.c ruler.c screen.c sprite.c
OBJS = $(SRC:.c=.o)
SIM_OBJS = $(addprefix $(SIM_BUILD_DIR)/, $(OBJS))
DEVICE_OBJS = $(addprefix $(DEVICE_BUILD_DIR)/, $(OBJS))

$(SIM_BUILD_DIR)/%.o: %.c | make_sim_build_dir
	$(SIM_CC) $(SIM_CFLAGS) -I $(INC_PATH) -c $< -o $@

$(SIM_BUILD_DIR)/%.o: %.s | make_sim_build_dir
	$(SIM_AS) $(SIM_ASFLAGS) -c $< -o $@

$(DEVICE_BUILD_DIR)/%.o: %.c | make_device_build_dir
	$(DEVICE_CC) $(DEVICE_CFLAGS) -I $(INC_PATH) -c $< -o $@

$(DEVICE_BUILD_DIR)/%.o: %.s | make_device_build_dir
	$(DEVICE_AS) $(DEVICE_ASFLAGS) -c $< -o $@

main.c: $(BUILD_DIR)/version.h

# Build version string from pdxinfo.  We would like to access this
# programmatically, but the C API doesn't have metadata access, so we
# will generate it during the build process.
$(BUILD_DIR)/version.h: pdxinfo | make_build_dir
	echo -n 'const char kPDBenchVersion[] = "' > $@
	sed -ne 's/name=\(.*\)/\1/;T;p' pdxinfo | tr -d '\n' >> $@
	echo -n ' v' >> $@
	sed -ne 's/version=\(.*\)/\1/;T;p' pdxinfo | tr -d '\n' >> $@
	echo '";' >> $@

# Link rules.
$(SIM_BUILD_DIR)/pdex.$(SIM_EXT): $(SIM_BUILD_DIR)/pdex_unstripped.$(SIM_EXT)
	$(SIM_STRIP) --strip-unneeded -g $< -o $@

$(SIM_BUILD_DIR)/pdex_unstripped.$(SIM_EXT): $(SIM_OBJS)
	$(SIM_CC) $(SIM_LFLAGS) -lm -DTARGET_SIMULATOR=1 -DTARGET_EXTENSION=1 $(SIM_OBJS) -o $@

$(DEVICE_BUILD_DIR)/pdex.elf: $(DEVICE_BUILD_DIR)/pdex_unstripped.elf
	$(DEVICE_STRIP) --strip-unneeded -R .comment -g $< -o $@

$(DEVICE_BUILD_DIR)/pdex_unstripped.elf: $(DEVICE_OBJS) link_map.ld
	$(DEVICE_CC) $(DEVICE_LFLAGS) $(DEVICE_OBJS) -o $@

# Launcher images.
$(BUILD_DIR)/launcher/card.png: $(BUILD_DIR)/title_card_background.png | make_launcher_dir
	convert $< -colorspace Gray -depth 1 -font Helvetica-Bold -fill black -pointsize 60 -gravity center -annotate +0+0 'PDBench' $@

$(BUILD_DIR)/title_card_background.png: title_card.rb | make_build_dir
	ruby $< $@

$(BUILD_DIR)/launcher/icon.png: $(BUILD_DIR)/pd.pgm $(BUILD_DIR)/b.pgm | make_launcher_dir
	pnmcat -tb $^ | pamthreshold -simple -threshold 0.9 | pnmtopng -compression 9 > $@

$(BUILD_DIR)/pd.pgm: | make_build_dir
	convert -size 32x28 'xc:#ffffff' -colorspace Gray -font Helvetica-Bold -fill black -pointsize 23 -gravity center -annotate +0+2 'PD' +repage -resize '32x19!' $@

$(BUILD_DIR)/b.pgm: | make_build_dir
	convert -size 28x28 'xc:#ffffff' -colorspace Gray -font Helvetica-Bold -fill black -pointsize 30 -gravity center -annotate +0-2 'B' +repage -resize '32x13!' $@

# Maintenance rules.
make_sim_build_dir: $(SIM_BUILD_DIR)

$(SIM_BUILD_DIR):
	mkdir -p $(SIM_BUILD_DIR)

make_device_build_dir: $(DEVICE_BUILD_DIR)

$(DEVICE_BUILD_DIR):
	mkdir -p $(DEVICE_BUILD_DIR)

make_build_dir: $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $@

make_launcher_dir: $(BUILD_DIR)/launcher

$(BUILD_DIR)/launcher: | make_build_dir
	mkdir -p $@

clean:
	-rm -rf $(SIM_BUILD_DIR) $(DEVICE_BUILD_DIR) $(BUILD_DIR)

# }}}
