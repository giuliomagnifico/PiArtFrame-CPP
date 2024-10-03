Tweaked C++ port by [kevlol](https://github.com/kevlol) of the [runezor/PiArtFrame](https://github.com/runezor/PiArtFrame) python project. 

The concept is the same, but the rendering algorithm is a bit different. I also added a simple bash script to set it up.


## Final result

![final-2](https://github.com/user-attachments/assets/04984b90-a46a-4192-91cb-3b98e74e3419)
![final-1](https://giuliomagnifico.blog/_images/2024/cPiArtFrame/build_b.jpg)

## Requirement:

- Raspberry Pi (best with Zero 2 or Zero)
- Waveshare 7.5" 800x480px display: [Link](https://www.google.com/search?q=Waveshare%207.5%22%20800x480px)
- 10x15cm frame, I used [this one](https://amzn.eu/d/ixgJbUD) from Amazon Basics
  
## Install

*(You need to install gcc, make, and all the other required C++ packages and libraries for your specific Raspberry Pi OS)*

To install and run it, download the project:
```
git clone https://github.com/giuliomagnifico/PiArtFrame-CPP
```
Then launch the `config.sh` file using:
```
bash config.sh
```
It will ask you how many minutes you want to use between the creating of  a new image on the display (default is 3s to test). After that, it will compile the code with your settings and add the command to launch PiArtFrame at every reboot.

Enjoy the fractals!

## Build images 

![build](https://giuliomagnifico.blog/_images/2024/cPiArtFrame/build_a.jpg)

### Tip

You can use any Waveshare display, also smaller or bigger, but you have to change the [main.c](main.c) code according to the EPD library of your e-paper. The libraries are all included inside the [lib folder](https://github.com/giuliomagnifico/PiArtFrame-CPP/tree/master/lib/waveshare/e-Paper) of this repository. 
