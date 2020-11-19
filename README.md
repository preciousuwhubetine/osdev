This is the code for an Operating System i've been working on for some time now.
Written and compiled on Windows 10 and tested in Virtualbox and on the real hardware.

WHATS CURRENTLY INCLUDED:
- 1st stage bootloader that prepares some neccesary stuff and loads the second stage 
  since the first stage is limited to just 512 bytes of code.
- Support for interrupt handling.
- Multitasking support.
- PCI device detection and device drivers for detected devices.
- A keyboard driver.
- An IDE driver for handling ATA hard disks.
- An AHCI driver for handling SATA hard disks.

In the makefile, I write the output binary to /dev/sdb which is the second hdd connected to my PC.
I'm gonna be making a tutorial on how I did all of this really soon...

Whats coming:
- A USB 2.0 EHCI driver.
- A FAT32 filesystem handler.
- Support for some available video (VESA) modes.
- A basic GUI (planning to make this sophisticated).
- Basic GUI controls (textboxes, listboxes, etc).
- Displaying a bitmap image on the screen.
- An application launcher (still not sure how i'm gonna be doing this).

I'm gonna be updating the list as new ideas drop!
