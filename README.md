# Orbitals
Pretty screen-candy drawing planets, asteroids and comets on the frame buffer. 
Written as a speed test for the raspberry pi 4 under Linux in c.

This is a test of the speed of the raspberry pi 4. This program plots on the frame buffer a face-on view of the
solar system showing the Sun, 9 planets, 60000 or so asteroids and several hundred comets.

To build this, clone this into a directory (git clone ...)
Then:
1. gunzip ELEMENTS.COMETS.gz
2. gunzip ELEMENTS.NUMBR.gz
3. cc -o orbit orbit.c -lm

Now from a command line prompt while NOT running x-windows run ./orbit

X-windows interfers with the screen buffer. You CAN run from a terminal window in
X but x-windows will paint over the display. Then orbit will paint over the display. And so on.

An easy way to leave x-windows is press ctrl-alt-F1 and run from the command line there. 
To return to x-windows press alt-F7

The speed of rotation is goverened by the #define SPEED nn in the program. 1.0 will rotate as
fast as the processor can run. 10.0 is pretty slow. The current setting is good for me.

There is a zoom feature. This is a number between about 10 and 400. A value of 200 shows out almost to 
the orbit of Jupiter. Bigger zooms in, smaller zooms out.

./orbit [zoom] where zoom is a number from 10 to 400.

To exit, press ctrl-c.
