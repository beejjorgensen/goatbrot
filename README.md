# Goatbrot

Generate Mandelbrot set images in PPM format

`````
      )_)
   ___|oo)   Mandelbrot Set generation--
  '|  |\_|                               --for goats!
   |||| #
   ````
`````

## Features

* OpenMP support for multicore machines
* Supports very large images
* Multiple built-in themes
* Both discrete and continuous rendering
* Antialiasing
* Harms no goats!

Output is a PPM file (uncompressed RGB), which can be converted to
another format with netpbm, ImageMagick, GIMP, PhotoChop, or anything
similar.

## Build info

Seen to build with Gnu `make` under Linux and OSX. Add more platforms in
the `Makefile`.

