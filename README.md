# Saintcon2021

![BadgeFrontUnlocked](https://user-images.githubusercontent.com/11616206/141223839-d2ef054d-c6c7-49e2-a71e-41fdbac0ce36.jpg)

This year's badge was created by compukidmike, Professor_Plum, and Sodium_Hydrogen. It's styled after a combination lock and the shackle actually opens!

![BadgeFrontLocked](https://user-images.githubusercontent.com/11616206/141223869-276719d3-f372-4e30-9a6c-e9acd9365706.jpg)

We tried something different for the front PCB. We used pcbbuy.com because they can do 2-color soldermask. It didn't turn out exactly like planned, but it was still a cool option that we'll probably use in the future.

One of the major features of the badge is NFC. The badge can read and write NFC tags, as well as emulate cards. This is used for sharing contact information, reading unlock tags, and trading items.

The main microcontroller is an ATSAME53J18A. The NFC controller is an ST25R95 and the LCD is actually round. The badge is composed of a stack of PCBs (4 boards thick, 5 boards total). Here's an exploded view of the badge:

![Exploded](https://user-images.githubusercontent.com/11616206/141225255-acbc1b2f-5d79-406f-9eae-c2d177453d2b.jpg)
![Inside](https://user-images.githubusercontent.com/11616206/141225260-1cbf47fc-d545-40f6-b3f9-7c3b45770c6a.jpg)

One of the big challenges was fitting parts under the screen. We've got the thickness of 2 boards (3.2mm) to fit everything. The LCD is 2.2mm thick, so that leaves 1mm for components. This made it extremely difficult to find parts, especially with the chip shortages this year.

## Firmware

### Compiling

You'll need to install ~~Atmel~~ Microchip Studio to compile the firmware. Once it's compiled, you can flash it to the badge with an SWD programmer, or follow the directions below to use the USB bootloader.

### Making a UF2 File

When you've compiled the firmware, Microchip Studio will create a bunch of files (likely in the Debug folder inside your project folder). You want the .hex file. You'll need to use a tool called uf2conv.py from https://github.com/microsoft/uf2 (it's in the utils folder). This python script will convert your hex file into a UF2 file that you can use to program your badge. Here's an example command: python .\uf2conv.py .\flash.bin -c -o .\flash.uf2

### Flashing

To re-flash your badge there is a USB bootloader. To activate it, move the power switch to USB and hold the button on the right side of the badge when you connect it to your computer. The screen should show some hardware tests and then it will connect to your computer as a mass storage device. Simply copy the new UF2 firmware file to the badge.

### External SPI Flash

The badge has an 8MB SPI flash chip that stores all the images. It can be reprogrammed via the bootloader, but it's a bit cumbersome. You'll need to use the makeFlash.py script in the ExternalFlash folder to create a binary file from the images. NOTE: The images will be placed in the binary one after the other aligned to 1024 byte boundaries. If you change the images, you'll need to update the list of image locations in flash.h

Once you've got the binary file, you need to convert it to a hex file. This can be done with with bin2hex.py script in the ExternalFlash folder. Here's an example command: python .\bin2hex.py .\flash.bin .\flash.hex

Finally, you need to convert this hex file into a UF2 file that the bootloader can use to reprogram the SPI flash. When using the uf2conv.py script, you'll need to add an offset parameter so that the bootloader knows this data is for the SPI flash. Here's an example command: python .\uf2conv.py .\flash.bin -c -b 0x4000000 -o .\flash.uf2

NOTE: When copying a UF2 file that reprograms the SPI flash, the badge will send an erase command to the SPI flash. This can take a while (30-60 seconds) and you will see an activity indicator on the screen. If this process takes too long, your computer may timeout on copying the file (the badge will change from "erasing" to "writing", but the LED will be pulsing slowly instead of blinking rapidly). If this happens, simply reset the badge into bootloader mode and copy the file again. This time the SPI flash will be empty and the erase command will be quick.
