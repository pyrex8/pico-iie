# pico-iie

Apple IIe emulator that runs on the Pi Pico.

### Project History

This project started in 2020 from a lack of desk space, curiosity, and a bit of Nostalgia.
I wanted to play some games and possibly write some code on an Apple IIe.
I don't have a large enough desk space to permanently set up an Apple computer and monitor plus I didn't want to go down the rabbit hole of looking at modern storage options.

My first thought was to run an emulator. I use Ubuntu as my primary home machine. It seemed that the most active project on github was LinApple (https://github.com/linappleii/linapple). I had some trouble compiling the project so I went looking for alternatives. I found some historical background on LinApple from the person who originally ported AppleWin over to Linux.
 (http://linapple.sourceforge.net/).  

Then I was able to find a fork of the sourceforge code. From the initial commit it looks like this started in 2012 (https://github.com/LasDesu/linapple.git). I was able to easily compile this version and get it running. This doesn't seem to be actively updated but I found the code simpler than the more active https://github.com/linappleii/linapple.

After looking at the source code for a while I understood a little about how it worked, but only at the most rudimentary level. The project was written in C++, but I couldn't find anything in the code that required C++ over C. I'm far more comfortable writing code in C so I was starting to wonder if it could just be in C. A lot of the modules we coupled together calling each others functions which made the conversion and understanding the code difficult.

I started playing with the code to see if I could accomplish a few things:
1. Simplify the code.
2. Remove features that I don't find useful.
3. Decouple all the modules.
4. Convert all the files to C.

At this point in 2020 I started what I called my deconstruction project. It basically was a process with two steps.
1. Remove a feature, simplify, or refactor some code.
2. Run a game and see if it still works.

This may sound like a tedious process, but it was super fun and gave me an excuse to try out Apple II games on a regular bases.

The original LinApple code uses 6502 emulation that executes a complete instruction every call as opposed to a per cycle emulator that executes one clock cycle per execution. This is one of the main reasons the code had so much coupling between modules. A per instruction emulation requires accessing RAM, ROM, and peripherals from the actual instruction. I looked around for a per cycle emulator. I found this project https://github.com/floooh/chips. It was surprisingly easy to integrate into the project. This allowed decoupling of all the other modules from the 6502 emulation. The trade off is that per cycle emulation is slower than per instruction emulation.

I ended up removing the SDL1 layer used in the project and used Pygame as the interface to the code just to test the C portion of the emulator. This simplified the C code but slowed down the project further. At this point the emulator code was cleaner but running slower than the original project. The other issue besides speed was that emulating sound I have not been able to update sound without glitching/ clicking between updates. I'm not sure if this is an inherent problem with Pygame or just my lack of understanding.

I decide to try running the emulator on a microcontroller. After trying out a few processors I landed on using the Pi Pico that this project is based on.

The projects use both cores and is overclocked at 470MHz.

#### Core 0
- VGA
- All video operations
- UART keyboard, joystick, bin, and disk files
- TEST0_PIN

#### Core 1
- 6502
- Speaker GPIO
- TEST1_PIN

 Even with the overclocking the emulator ats approximately 1.4 microseconds to complete an instruction. Even with slower emulation games are quite playable.

### Hardware
The emulator runs on a Pi Pico using a Pimoroni Pico VGA Demo Base (https://shop.pimoroni.com/products/pimoroni-pico-vga-demo-base?variant=32369520672851) with a few modifications.


![Alt text](images/pico-iie_board.jpg?raw=true "board")

### Pi Pico Pinout Block Diagram

From the block diagram it can be seen that the peripherals use are:
- GPIO
- UART1
- PWM0, PWM1, PWM2
- PIO0
- DMA0, DMA1

VGA is data is produced using PIO0 as a 16 bit parallel port controlled by a circular DMA. The length of the buffer is equivalent to one VGA scan line. This would allow a similar implementation on other Microcontrollers.

![Alt text](images/pico-iie_block_diagram.drawio.png?raw=true "Pi Pico Block Diagram")

Once the board connected to a VGA monitor and an audio output device and is powered up either by the USB micro connector on the Pi Pico or the USB to UART cable the Apple IIe prompt and the familiar beep can be heard.

![Alt text](images/pico-iie_bootup_screen.jpg?raw=true "Title")

The disk is not automatically detected. By starting ```main.py``` communication starts between a Linux PC and the Pi Pico. If main.py is started with a .dsk file the emulator will automatically reboot with the dsk imag.

main.py can load a ```bin``` or ```dsk``` file.

If you use the menu command Download ROM image for Cosmic Impalas and save locally
https://8bitworkshop.com/v3.9.0/?file=cosmic.c&platform=apple2

You can download the bin file directly to the pico-iie though the USB to serial cable
```python3 main.py cosmic.bin```

The green LED on the pi pico turn on solid green for a few seconds as it downloads then the game will start automatically

![Alt text](images/pico-iie_cosmic_impalas.jpg?raw=true "Title")

Example:
```python3 main.py Choplifter.dsk```


After the file is downloaded and running the main.py can be used for keyoard input and/or game controller input through the serial cable. This eliminates the need for direct connection to peripherals to the Pi Pico.

F1 is used for breaking your program, the same as CRTL-C on the original machine.
F12 exits main.py program
ESC key will pause program


There are some simplifications to the emulator.

Only emulates a 48K RAM.

TEXT and HIRES modes only

No blinking text, just NORMAL and INVERSE. FLASH displays as inverse and some odd characters.

Most of the soft switch read "side affect" are not emulated.

Vertical blanking register is not updated. This is due to it only being in the IIe so most games don't use it.
