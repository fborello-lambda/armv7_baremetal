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

nix.build: clean ## Build the project using nix-shell
	nix-shell --run "make bin/image.bin"
.PHONY: nix.build

build: clean bin/image.bin ## Build the project
.PHONY: build

bin/image.bin: obj/image.elf ## Rule to create the binary
	mkdir -p bin
	$(OC) -O binary $< $@

obj/image.elf: $(ALL_OBJ_FILES) ## Rule to link object files into a bootable image
	mkdir -p map
	$(LD) -T $(LINKER_SCRIPT) -o $@ $(ALL_OBJ_FILES) -Map map/image.map

obj/proc/%.o: proc/%.s ## Rule to compile assembly files into object files for proc
	mkdir -p obj/proc
	$(AS) -c $< -g -o $@ -a > $@.lst

obj/sys/%.o: sys/%.s ## Rule to compile assembly files into object files for sys
	mkdir -p obj/sys
	$(AS) -c $< -g -o $@ -a > $@.lst

obj/core/%.o: core/%.s ## Rule to compile assembly files into object files for core
	mkdir -p obj/core
	$(AS) -c $< -g -o $@ -a > $@.lst

obj/kernel/%.o: kernel/%.c ## Rule to compile C files into object files for kernel
	mkdir -p obj/kernel
	$(GCC) -g $(COPT) $(CFLAGS) -c $< -o $@

obj/kernel_rs/drivers.o: kernel/rs/drivers.rs ## Rule to compile RS files into object files for kernel
	mkdir -p obj/kernel_rs
	rustc -Copt-level=s --emit=obj $< --target=armv7a-none-eabi -o $@

nix.objdump: ## Run objdump using nix-shell
	nix-shell --run "arm-none-eabi-objdump -d $(DISASM)"
.PHONY: nix.objdump

objdump: ## Run objdump
	arm-none-eabi-objdump -d $(DISASM)
.PHONY: objdump

nix.debug: ## Debug the project using GDB with nix-shell
	nix-shell --run  'gdb -q \
	-ex "target remote :2159" \
	-ex "set logging file ${LOG_FILE}" \
	-ex "set logging on" \
	-ex "set print pretty on" \
	-ex "list" \
	$<'
.PHONY: nix.debug

debug: obj/image.elf ## Debug the project using GDB
	gdb-multiarch -q \
	-ex "target remote :2159" \
	-ex "set logging file ${LOG_FILE}" \
	-ex "set logging on" \
	-ex "l" \
	$<
.PHONY: debug

nix.ddd.debug: ## Debug the project using DDD and nix-shell, attach to localhost:2159
	nix-shell --run 'ddd \
		--debugger "gdb \
			-iex '\''set auto-load safe-path /'\'' \
			-ex '\''target remote localhost:2159'\''" \
		obj/image.elf'
.PHONY: nix.ddd.debug

nix.qemuA8: bin/image.bin ## Run QEMU with ARMv7 architecture using nix-shell
	nix-shell --run "qemu-system-arm \
	-M realview-pb-a8 -m 64M \
	-no-reboot -nographic \
	-monitor telnet:127.0.0.1:1234,server,nowait \
	-kernel $< -S -gdb tcp::2159"
.PHONY: nix.qemuA8

qemuA8: bin/image.bin ## Run QEMU with ARMv7 architecture
	qemu-system-arm \
	-M realview-pb-a8 -m 64M \
	-no-reboot -nographic \
	-monitor telnet:127.0.0.1:1234,server,nowait \
	-kernel $< -S -gdb tcp::2159
.PHONY: qemuA8

nix.fmt: ## Format the Nix file
	nixfmt shell.nix
.PHONY: nix.fmt

nix.cfmt: ## Format C files using clang-format with nix-shell
	nix-shell --run "clang-format -i kernel/*.c kernel/inc/*.h"
.PHONY: nix.cfmt

cfmt: ## Format C files using clang-format
	clang-format -i kernel/*.c kernel/inc/*.h
.PHONY: cfmt

clean: ## Clean up generated files
	rm -rf obj/*
	rm -rf bin/*
.PHONY: clean

help: ## Show available make targets
	@echo "\033[1;36mAvailable make targets:\033[0m"
	@awk 'BEGIN {FS = ":.*##"} /^[a-zA-Z0-9_.-]+:.*##/ {printf "  \033[1;33m%-20s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST) | sort
	@echo "\033[1;36m\nCommon Usage with nix:\033[0m"
	@echo "  - Use \033[1;33mmake nix.build nix.qemuA8\033[0m to start the QEMU server using nix-shell."
	@echo "  - Then in a new terminal use \033[1;33mmake nix.ddd.debug\033[0m to attach DDD(GDB GUI) to the QEMU server."
	@echo "\033[1;36m\nCommon Usage without nix (you can attach any GDB GUI, using the above target as example):\033[0m"
	@echo "  - Use \033[1;33mmake qemuA8\033[0m to start the QEMU server using nix-shell."
	@echo "  - Then in a new terminal use \033[1;33mmake nix.ddd.debug\033[0m to attach DDD(GDB GUI) to the QEMU server."
.PHONY: help
