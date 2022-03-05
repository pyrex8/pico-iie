# pico-iie

Apple IIe emulator that runs on the Pi Pico.

![Alt text](images/pico-iie_bootup_screen.jpg?raw=true "Title")

main.py can load a ```bin``` or ```dsk``` file.

If you use the menu command Download ROM image for Cosmic Impalas and save locally
https://8bitworkshop.com/v3.9.0/?file=cosmic.c&platform=apple2

You can download the bin file directly to the pico-iie though the USB to serial cable
```python3 main.py cosmic.bin```

The green LED on the pi pico turn on solid green for a few seconds as it downloads then the game will start automatically

![Alt text](images/pico-iie_cosmic_impalas.jpg?raw=true "Title")

Example:
```python3 main.py Choplifter.dsk```


F1 is used for breaking your program, the same as CRTL-C on the original machine.

There are some simplifications to the emulator.
Only emulates a 48K RAM.
TEXT and HIRES modes only
No blinking text, just NORMAL and INVERSE. FLASH displays as inverse and some odd characters.
