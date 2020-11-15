This is the code for an Operating System i've been working on for some time now.
Written and compiled on Windows 10 and tested in Virtualbox and on the real hardware.

WHATS CURRENTLY INCLUDED:
- 1st stage bootloader that prepares some neccesary stuff and loads the second stage 
  since the first stage is limited to just 512 bytes of code.
- A basic kernel with a print string routine.

WHATS COMING:
- Support for interrupt handling.
- Multitasking support.
- PCI device detection and device drivers for detected devices.
- A keyboard driver.
- An IDE driver for handling ATA hard disks.
- An AHCI driver for handling SATA hard disks.
- A USB 2.0 EHCI driver.
- A FAT32 filesystem handler.
- Support for some available video (VESA) modes.
- A basic GUI (planning to make this sophisticated).
- Basic GUI controls (textboxes, listboxes, etc).
- Displaying a bitmap image on the screen.
- An application launcher (still not sure how i'm gonna be doing this).

I'm gonna be updating the list as new ideas drop!
