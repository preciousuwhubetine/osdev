
GPPPARAMS_KERNEL = -m32 -Ikernel/include -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings -fcheck-new

device = /dev/sdb

boot_files = out/bin/bootloader1.bin \
            out/bin/bootloader2.bin

kernel_files = out/obj/kernel/kernel_entry.o \
				out/obj/kernel/util/screen.o \
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