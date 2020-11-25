
GPPPARAMS_KERNEL = -m32 -Ikernel/include -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings -fcheck-new

device = /dev/sdb

boot_files = out/bin/bootloader1.bin \
            out/bin/bootloader2.bin

kernel_files = out/obj/kernel/kernel_entry.o \
				out/obj/kernel/util/screen.o \
				out/obj/kernel/util/system.o \
				out/obj/kernel/util/events.o \
				out/obj/kernel/util/number.o \
				out/obj/kernel/util/string.o \
				out/obj/kernel/util/images/bitmap.o \
				out/obj/kernel/IO/port.o \
				out/obj/kernel/IO/interrupts.o \
				out/obj/kernel/IO/interruptstubs.o \
				out/obj/kernel/IO/pci.o \
				out/obj/kernel/drivers/driver.o \
				out/obj/kernel/drivers/keyboard.o \
				out/obj/kernel/drivers/ide.o \
				out/obj/kernel/drivers/ahci.o \
				out/obj/kernel/drivers/ehci.o \
				out/obj/kernel/drivers/usb.o \
				out/obj/kernel/drivers/usb_mass_storages.o \
				out/obj/kernel/drivers/filesystem.o \
				out/obj/kernel/graphics/desktop.o \
				out/obj/kernel/graphics/widget.o \
				out/obj/kernel/graphics/window.o \
				out/obj/kernel/memory_manager.o \
				out/obj/kernel/multitasking.o \
				out/obj/kernel/kernel_main.o

out/bin/%.bin: bootloader/%.asm
	mkdir -p out
	mkdir -p out/bin
	nasm -f bin $< -o $@

out/obj/kernel/%.o: kernel/src/%.asm
	mkdir -p out
	mkdir -p out/obj
	mkdir -p out/obj/kernel
	nasm -f elf32 $< -o $@

out/obj/kernel/%.o: kernel/src/%.cpp
	mkdir -p out
	mkdir -p out/obj
	mkdir -p out/obj/kernel
	mkdir -p out/obj/kernel/util
	mkdir -p out/obj/kernel/util/images
	mkdir -p out/obj/kernel/IO
	mkdir -p out/obj/kernel/drivers
	mkdir -p out/obj/kernel/graphics
	i586-elf-g++ $(GPPPARAMS_KERNEL) -c $< -o $@

kernel.bin: $(kernel_files)
	mkdir -p out
	mkdir -p out/bin
	i586-elf-ld -Tkernel/linker.ld -o out/bin/kernel.bin $(kernel_files)

install: $(boot_files) kernel.bin
	dd if=out/bin/bootloader1.bin of=$(device)
	dd if=out/bin/bootloader2.bin of=$(device) seek=1
	dd if=out/bin/kernel.bin of=$(device) seek=3

run: install
	VirtualBox --startvm "CrystalOS"

.PHONY: clean
clean:
	rm -rf out