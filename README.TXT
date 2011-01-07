goatbrot -- generate mandelbrot set images in PPM format

      )_)
   ___|oo)   Mandelbrot Set generation--
  '|  |\_|                               --for goats!
   |||| #
   ````

Features:

  * Compiles with optional OpenMP support for multicore machines

  * Supports very large images

  * Multiple built-in themes

  * Both discrete and continuous rendering

  * Antialiasing

  * Harms no goats!


Usage: goatbrot -o outfilename [options]

   -e examplenum            example render (1-8) (should be first option)
   -o outfilename           set output name
   -c re,im                 set center point
   -w width                 set real width
   -s width,height          set pixel output size
   -i iterations            set max iterations
   -a                       antialias
   -u                       continuous smoothing
   -q                       no informational output
   -t numthreads            set number of threads
   -m theme                 set theme--names are:
                               beej
                               fire
                               water

   Output is a PPM file, which can be converted to another format with
   netpbm, ImageMagick, GIMP, or anything similar.

   Currently only non-OpenMP builds support streaming output.

   
Build info:

  See the Makefile for setting OpenMP vs non-OpenMP build options.

  Define GBOPENMP for OpenMP support.

  OpenMP depends on mmap and ftruncate, currently.

  gcc 4.2+ needed for OpenMP support.

  Seen to work under Linux and OSX.  Probably works under all Unices.
  Needs work for Windows.

  GCC: Use -fopenmp on command line.

  XCode: In project, select GCC 4.2+, check "OpenMP" box.

  Visual Studio: See "Future mods", below.  Use /openmp compiler switch.


Future mods:

  Win32:
    Non-portable code in the following functions:

       parse_command_line() calls getopt, replace with ???

       These functions are only needed if GBOPENMP is set:

       set_file_length()    calls ftruncate, replace with _chsize?
       memory_map_file()    calls mmap, replace with MapViewOfFile?
       memory_unmap_file()  calls munmap, replace with ???


See also:

  http://netpbm.sourceforge.net/


Related:

  beej@beej.us
  http://beej.us/


License (MIT):

  Copyright (c) 2011  Brian "Beej Jorgensen" Hall <beej@beej.us>

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

