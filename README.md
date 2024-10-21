Tweaked C++ port by [kevlol](https://github.com/kevlol) of the [runezor/PiArtFrame](https://github.com/runezor/PiArtFrame) python project. 

**Changes**
- The concept is the same, but the rendering algorithm is different.
- You can also render the [Julia set](https://en.wikipedia.org/wiki/Julia_set) fractal instead of the Mandelbrot. Check out the [image](https://github.com/giuliomagnifico/PiArtFrame-CPP/tree/master?tab=readme-ov-file#mandelbrot-vs-julia) at the bottom of the README to compare the two fractals. For more details, read the [tips](https://github.com/giuliomagnifico/PiArtFrame-CPP/tree/master#tips).
- I also added a simple bash script to set it up.


## Final result

![final-2](https://github.com/user-attachments/assets/f3e66f7a-36e1-498d-a780-91780960e283)


## Requirement

- Raspberry PI (Zero 2)
- Waveshare 7.5" 800x480px e-ink display: [Link](https://www.google.com/search?q=Waveshare%207.5%22%20800x480px)
- 10x15cm frame, I used [this one](https://amzn.eu/d/ixgJbUD) from Amazon Basics
  
## Install

First, you need to enable the `SPI interface` so the Raspberry Pi can connect to the display. Usually by running:

```
raspi-config
Interfacing Options -> SPI -> Yes - enable SPI interface
```
On DietPi is under:

```
dietpi-config
Advanced Options -> SPI state -> Turn ON
```

After that, you'll need to install some software based on your PI OS. I'm using DietPi, so I used:

```
apt install gcc g++ make liblgpio-dev -y
```
To install and run this project, just download it:
```
git clone https://github.com/giuliomagnifico/PiArtFrame-CPP
```
`cd`intro the folder *PiArtFrame-CPP* and launch the `config.sh` file using:
```
bash config.sh
```
It will ask you how many minutes you want between creating new images on the display (default is 15). After that, it will compile the code with your settings and add the command to launch PiArtFrame at every reboot.

### Render the Julia instead of Mandelbrot

If you want to use the [Julia set](https://en.wikipedia.org/wiki/Julia_set) fractal instead of the Mandelbrot, do the same steps but using the "julia-set" branch:

`git clone --branch julia-set https://github.com/giuliomagnifico/PiArtFrame-CPP.git`

and 

```
bash config.sh
```

Enjoy the fractals!

## Assembly

![Assembly](https://github.com/user-attachments/assets/898496b8-f1ad-42d0-9bd6-e00dbd6262c4)

![final](https://github.com/user-attachments/assets/e1d2aff0-ee0a-43dc-ba22-1317128ee467)

### Tips


You can use any Waveshare display, also smaller or bigger, but you have to change the [main.c](main.c) and [Makefile](https://github.com/giuliomagnifico/PiArtFrame-CPP/blob/master/Makefile) code according to the EPD library of your e-paper. The libraries are all included inside the [lib folder](https://github.com/giuliomagnifico/PiArtFrame-CPP/tree/master/lib/waveshare/e-Paper) of this repository. 
