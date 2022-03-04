# pico-iie

Apple IIe emulator that runs on the Pi Pico.

![Alt text](images/pico-iie_bootup_screen.jpg?raw=true "Title")



main.py can launch load a ```bin``` or ```dsk``` file.

If you use the menu command Download ROM image for Cosmic Impalas and save locally
https://8bitworkshop.com/v3.9.0/?file=cosmic.c&platform=apple2

You can download the bin file directly to the pico-iie though the USB to serial cable
```python3 ~/pico/pico-iie/main.py cosmic.bin```

The green LED on the pi pico turn on solid green for a few seconds as it downloads


![Alt text](images/pico-iie_bin_file_download.jpg?raw=true "Title")

If you invoke the monitor using ```CALL -151```
then start running the bin file with the command ```0803G```

the game will start


![Alt text](images/pico-iie_cosmic_impalas.jpg?raw=true "Title")




Example:
```python3 main.py Choplifter.dsk```
