# My Operating System

This is the code for my operating system, written in C++. It is currently under development, but it already includes support for interrupt handling, multitasking, device drivers, a GUI, and a FAT32 filesystem handler.

## Tech Stack
* Assembly Language
* C++

## Features

* 1st stage bootloader
* Support for interrupt handling
* Multitasking support
* PCI device detection and device drivers
* Keyboard driver
* IDE driver for handling ATA hard disks
* AHCI driver for handling SATA hard disks
* USB 2.0 EHCI driver
* FAT32 filesystem handler
* Support for some available video (VESA) modes
* Basic GUI
* Displaying a bitmap image on the screen

### Requirements

* Windows 10 or Linux
* GCC compiler
* QEMU emulator (optional)
* VirtualBox (optional)

### Installation

1. Clone the repository:

```
git clone https://github.com/preciousbetine/osdev.git
```

2. Compile the operating system:

```
cd osdev
make
```

3. Boot the operating system in QEMU:

```
qemu-system-x86_64 -kernel mykernel.bin
```

4. (Optional) Boot the operating system in VirtualBox:
* Create a new virtual machine in VirtualBox.
* Set the operating system to "Other".
* Set the memory size to at least 1024 MB.
* Generate a VMDK and boot the OS by the instructions in [this article](https://www.partitionwizard.com/partitionmanager/virtualbox-boot-from-usb.html).
* Start the virtual machine.

### Usage
Once the operating system is booted, you can interact with it using the keyboard.

### Future Features
* Basic GUI controls (textboxes, list boxes, etc.).
* An application launcher.
* More device drivers.
* A file manager.
* A shell.

### Acknowledgements
I would like to thank these sources for their extensive info:

- OSDev Wiki: https://wiki.osdev.org/.
- [This](https://www.youtube.com/playlist?list=PLBK_0GOKgqn3hjBdrf5zQ0g7UkQP_KLC3) youtube playlist.
- Most importantly, [this](https://www.youtube.com/playlist?list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M) one.

### License
This project is [MIT](./LICENSE) licensed.
