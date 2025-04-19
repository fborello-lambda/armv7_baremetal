# Define the compiler
AS = arm-none-eabi-as
GCC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OC = arm-none-eabi-objcopy

## Flags
CFLAGS ?= -std=gnu99 -Wall -mcpu=cortex-a8
COPT ?= -O0 # No optimizations

## List of assembly source files
PROC_AS_SRC := $(wildcard proc/*.s)
SYS_AS_SRC := $(wildcard sys/*.s)
CORE_AS_SRC := $(wildcard core/*.s)
KERNEL_C_SRC := $(wildcard kernel/*.c)
KERNEL_RS_SRC := $(wildcard kernel/rs/drivers.rs)

RS := 0

## Path to linker script
LINKER_SCRIPT := linker/mmap.ld

## List of object files generated from assembly source files
PROC_OBJ_FILES := $(patsubst proc/%.s, obj/proc/%.o, $(PROC_AS_SRC))
SYS_OBJ_FILES := $(patsubst sys/%.s, obj/sys/%.o, $(SYS_AS_SRC))
CORE_OBJ_FILES := $(patsubst core/%.s, obj/core/%.o, $(CORE_AS_SRC))
KERNEL_OBJ_FILES := $(patsubst kernel/%.c, obj/kernel/%.o, $(KERNEL_C_SRC))
KERNEL_RS_OBJ_FILES := $(patsubst kernel/rs/%.rs, obj/kernel_rs/%.o, $(KERNEL_RS_SRC))

ALL_C_OBJ_FILES := $(PROC_OBJ_FILES) $(SYS_OBJ_FILES) $(CORE_OBJ_FILES) $(KERNEL_OBJ_FILES)

ifeq ($(RS), 1)
	ALL_OBJ_FILES := $(ALL_C_OBJ_FILES) $(KERNEL_RS_OBJ_FILES)
else
	ALL_OBJ_FILES := $(ALL_C_OBJ_FILES)
endif

.DEFAULT_GOAL := help

## Build the project using nix-shell
nix.build: clean
	nix-shell --run "make bin/image.bin"
.PHONY: nix.build

## Build the project
build: clean bin/image.bin
.PHONY: build

## Rule to create the binary
bin/image.bin: obj/image.elf
	mkdir -p bin
	$(OC) -O binary $< $@

## Rule to link object files into a bootable image
obj/image.elf: $(ALL_OBJ_FILES)
	mkdir -p map
	$(LD) -T $(LINKER_SCRIPT) -o $@ $(ALL_OBJ_FILES) -Map map/image.map

## Rule to compile assembly files into object files for proc
obj/proc/%.o: proc/%.s
	mkdir -p obj/proc
	$(AS) -c $< -g -o $@ -a > $@.lst

## Rule to compile assembly files into object files for sys
obj/sys/%.o: sys/%.s
	mkdir -p obj/sys
	$(AS) -c $< -g -o $@ -a > $@.lst

## Rule to compile assembly files into object files for core
obj/core/%.o: core/%.s
	mkdir -p obj/core
	$(AS) -c $< -g -o $@ -a > $@.lst

## Rule to compile C files into object files for kernel
obj/kernel/%.o: kernel/%.c
	mkdir -p obj/kernel
	$(GCC) -g $(COPT) $(CFLAGS) -c $< -o $@

## Rule to compile RS files into object files for kernel
obj/kernel_rs/drivers.o: kernel/rs/drivers.rs
	mkdir -p obj/kernel_rs
	rustc -Copt-level=s --emit=obj $< --target=armv7a-none-eabi -o $@

DISASM := obj/image.elf

## Run objdump using nix-shell
nix.objdump:
	nix-shell --run "arm-none-eabi-objdump -d $(DISASM)"
.PHONY: nix.objdump

## Run objdump
objdump:
	arm-none-eabi-objdump -d $(DISASM)
.PHONY: objdump


LOG_FILE ?= gdb_session0.log

## Debug the project using GDB with nix-shell
nix.debug: obj/image.elf
	nix-shell --run  'gdb -q \
	-ex "target remote :2159" \
	-ex "set logging file ${LOG_FILE}" \
	-ex "set logging on" \
	-ex "set print pretty on" \
	-ex "list" \
	$<'
.PHONY: nix.debug

## Debug the project using GDB
debug: obj/image.elf
	gdb-multiarch -q \
	-ex "target remote :2159" \
	-ex "set logging file ${LOG_FILE}" \
	-ex "set logging on" \
	-ex "l" \
	$<
.PHONY: debug

## Debug the project using DDD and nix-shell, attach to localhost:2159
nix.ddd.debug: obj/image.elf
	nix-shell --run 'ddd \
		--debugger "gdb \
			-iex '\''set auto-load safe-path /'\'' \
			-ex '\''target remote localhost:2159'\''" \
		obj/image.elf'
.PHONY: nix.ddd.debug

## Run QEMU with ARMv7 architecture using nix-shell
nix.qemuA8: bin/image.bin
	nix-shell --run "qemu-system-arm \
	-M realview-pb-a8 -m 32M \
	-no-reboot -nographic \
	-monitor telnet:127.0.0.1:1234,server,nowait \
	-kernel $< -S -gdb tcp::2159"
.PHONY: nix.qemuA8

## Run QEMU with ARMv7 architecture
qemuA8: bin/image.bin
	qemu-system-arm \
	-M realview-pb-a8 -m 32M \
	-no-reboot -nographic \
	-monitor telnet:127.0.0.1:1234,server,nowait \
	-kernel $< -S -gdb tcp::2159
.PHONY: qemuA8

## Format the Nix file
nix.fmt:
	nixfmt shell.nix
.PHONY: nix.fmt

## Format C files using clang-format with nix-shell
nix.cfmt:
	nix-shell --run "clang-format -i kernel/*.c kernel/inc/*.h"
.PHONY: nix.cfmt

## Format C files using clang-format
cfmt:
	clang-format -i kernel/*.c kernel/inc/*.h
.PHONY: cfmt

## Clean up generated files
clean:
	rm -rf obj/*
	rm -rf bin/*
.PHONY: clean

## Show this help
help:
	@echo "\033[1;36mAvailable make targets:\033[0m"
	@awk ' \
		/^##/ { help = substr($$0, 4); getline; \
			if ($$0 ~ /^([a-zA-Z0-9_.-]+):/) { \
				target = gensub(/^([a-zA-Z0-9_.-]+):.*/, "\\1", "g", $$0); \
				printf "  \033[1;33m%-15s\033[0m - %s\n", target, help; \
			} \
		}' $(MAKEFILE_LIST)
.PHONY: help
