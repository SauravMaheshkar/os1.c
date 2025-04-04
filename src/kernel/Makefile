# Nuke built-in rules and variables.
MAKEFLAGS += -rR
.SUFFIXES:

# This is the name that our final executable will have.
override OUTPUT := kernel

# Target architecture: x86_64 only.
ARCH := x86_64

# Install prefix; /usr/local is a good, standard default pick.
PREFIX := /usr/local

# User controllable C compiler command.
CC := x86_64-elf-gcc

# User controllable linker command.
LD := x86_64-elf-ld

# User controllable archiver command.
AR := ar

# User controllable C flags.
CFLAGS := -g -O2 -pipe

# User controllable C preprocessor flags.
CPPFLAGS :=

# User controllable nasm flags.
NASMFLAGS := -F dwarf -g

# User controllable linker flags.
LDFLAGS :=

# Ensure the dependencies have been obtained.
ifneq ($(shell ( test '$(MAKECMDGOALS)' = clean || test '$(MAKECMDGOALS)' = distclean ); echo $$?),0)
    ifeq ($(shell ( ! test -d freestnd-c-hdrs || ! test -d cc-runtime || ! test -f src/limine.h ); echo $$?),0)
        $(error Please run the ./get-deps script first)
    endif
endif

# Check if CC is Clang.
override CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep 'clang' >/dev/null 2>&1; echo $$?)

# Internal C flags.
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections

# Internal C preprocessor flags.
override CPPFLAGS := \
    -I src \
    -isystem freestnd-c-hdrs \
    $(CPPFLAGS) \
    -DLIMINE_API_REVISION=3 \
    -MMD \
    -MP

# Internal nasm flags.
override NASMFLAGS += -Wall

# x86_64 specific flags.
ifeq ($(CC_IS_CLANG),1)
    override CC += -target x86_64-unknown-none
endif

override CFLAGS += \
    -m64 \
    -march=x86-64 \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -mcmodel=kernel

# override LDFLAGS += \
#     -Wl,-m,elf_x86_64

override NASMFLAGS += \
    -f elf64

# Internal linker flags.
override LDFLAGS += \
    --build-id=none \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    --gc-sections \
    -T linker.ld

# Gather source files.
override SRCFILES := $(shell cd src && find -L * -type f | LC_ALL=C sort)
override CFILES := $(filter %.c,$(SRCFILES))
override ASFILES := $(filter %.S,$(SRCFILES))
override NASMFILES := $(filter %.asm,$(SRCFILES))

override OBJ := $(addprefix obj-$(ARCH)/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj-$(ARCH)/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Default target.
.PHONY: all
all: bin-$(ARCH)/$(OUTPUT)

# Include header dependencies.
-include $(HEADER_DEPS)

# Build the C compiler runtime.
cc-runtime-$(ARCH)/cc-runtime.a: Makefile cc-runtime/*
	rm -rf cc-runtime-$(ARCH)
	cp -r cc-runtime cc-runtime-$(ARCH)
	$(MAKE) -C cc-runtime-$(ARCH) -f cc-runtime.mk \
		CC="$(CC)" \
		AR="$(AR)" \
		CFLAGS="$(CFLAGS)" \
		CPPFLAGS='-isystem ../freestnd-c-hdrs -DCC_RUNTIME_NO_FLOAT'

# Link the final executable.
bin-$(ARCH)/$(OUTPUT): Makefile linker.ld $(OBJ) cc-runtime-$(ARCH)/cc-runtime.a
	mkdir -p "$$(dirname $@)"
	# $(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) cc-runtime-$(ARCH)/cc-runtime.a -o $@
	$(LD) $(LDFLAGS) $(OBJ) cc-runtime-$(ARCH)/cc-runtime.a -o $@

# Compilation rules.
obj-$(ARCH)/%.c.o: src/%.c Makefile
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

obj-$(ARCH)/%.S.o: src/%.S Makefile
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

obj-$(ARCH)/%.asm.o: src/%.asm Makefile
	mkdir -p "$$(dirname $@)"
	nasm $(NASMFLAGS) $< -o $@

# Clean targets.
.PHONY: clean
clean:
	rm -rf bin-$(ARCH) obj-$(ARCH) cc-runtime-$(ARCH)

.PHONY: distclean
distclean:
	rm -rf bin-* obj-* freestnd-c-hdrs cc-runtime* src/limine.h

# Install.
.PHONY: install
install: all
	install -d "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
	install -m 644 bin-$(ARCH)/$(OUTPUT) "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-$(ARCH)"

# Uninstall.
.PHONY: uninstall
uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-$(ARCH)"
	-rmdir "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
