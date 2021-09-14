.SUFFIXES:

# --------------------------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include ${DEVKITARM}/gba_rules

# --------------------------------------------------------------------------------------------------

OUTPUT = scratch
OFILES = scratch.o

INCLUDE = -I${LIBGBA}/include
OPTIMISE = -fomit-frame-pointer -ffast-math
ARCH = -mthumb -mthumb-interwork
CFLAGS = -g -Wall -O3 -mcpu=arm7tdmi -mtune=arm7tdmi ${OPTIMISE} ${ARCH} ${INCLUDE}

LD := ${CC}
LDFLAGS = -g ${ARCH} -Wl,-Map,$(notdir $@).map
LIBPATHS=-L${LIBGBA}/lib
LIBS := -lgba

DEPSDIR=${CURDIR}

# --------------------------------------------------------------------------------------------------

${OUTPUT}.gba : ${OUTPUT}.elf

${OUTPUT}.elf : ${OFILES}

all : ${OUTPUT}.gba

clean :
	rm -f *.o *.d *.elf* *.gba

# --------------------------------------------------------------------------------------------------
