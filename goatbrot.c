/**
 * goatbrot -- generate mandelbrot set images in PPM format
 *
 * goatbrot -h for usage information.
 *
 *       )_)
 *    ___|oo)   Mandelbrot Set generation--
 *   '|  |\_|                               --for goats!
 *    |||| #
 *    ````
 *
 * See also:
 *
 *   http://netpbm.sourceforge.net/
 *
 * Copyright (c) 2011  Brian "Beej Jorgensen" Hall <beej@beej.us>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <float.h>
#include <omp.h>
#include <sys/mman.h>
#include <errno.h>

#define VERSION "1.0"

#define APPNAME "goatbrot"

#define DEFAULT_MAX_ITERATIONS 500
#define DEFAULT_CENTER_REAL (-0.75)
#define DEFAULT_CENTER_IMG (0.0)
#define DEFAULT_WIDTH_REAL (3.5)
#define DEFAULT_IMAGE_WIDTH 800 
#define DEFAULT_IMAGE_HEIGHT 600 

/*
 * Theme information
 */
typedef void (*time_to_color_function)(long double t, int *r, int *g, int *b);

struct theme {
    char *name;
    time_to_color_function time_to_color;
};

enum theme_type {
	THEME_BEEJ=0, // array indexes in theme[] !!
	THEME_FIRE,
	THEME_WATER,

	THEME_COUNT,
	THEME_NONE = -1,
};

void time_to_color_beej(long double t, int *r, int *g, int *b);
void time_to_color_fire(long double t, int *r, int *g, int *b);
void time_to_color_water(long double t, int *r, int *g, int *b);

struct theme theme[THEME_COUNT] = {
    { "beej", time_to_color_beej },
    { "fire", time_to_color_fire },
    { "water", time_to_color_water }
};

/**
 * Information about a particular Mandelbrot render
 */
struct mandelbrot_info {
	// !!! do not change the order of these without changing the
	// example_scene[] initializer: !!!

	long double center_re;
	long double center_im;
	long double width_re;

	int max_iterations;
	enum theme_type theme;

	long long width_px;
	long long height_px;

	int antialias;
	int continuous;

	// these are computed at runtime
	long double height_im;
	long double left_re;
	long double right_re;
	long double top_im;
	long double bottom_im;
};

#define EXAMPLE_SCENE_COUNT 8

struct mandelbrot_info example_scene[EXAMPLE_SCENE_COUNT] = {
	{ -0.1762357591832987, -1.085211211753544, 5.238689482212067e-11, 1000, THEME_BEEJ, 1024, 1024, 0, 0, 0,0,0,0,0 },
	{ -0.17623575918340056, -1.0852112117536312, 6.5483618527650835e-12, 1000, THEME_BEEJ, 1024, 1024, 0, 0, 0,0,0,0,0 },
	{ 0.363506765451268, -0.6648038236339705, 3.2741809263825417e-12, 1000, THEME_BEEJ, 1024, 1024, 0, 0, 0,0,0,0,0 },
	{ -0.1832226562500002, -1.0915449218749997, 0.00087890625, 1000, THEME_BEEJ, 1024, 1024, 0, 0, 0,0,0,0,0 },
	{ -0.1875725133825629, -1.096423784281244, 1.0231815394945443e-13, 1000, THEME_BEEJ, 1024, 1024, 0, 0, 0,0,0,0,0 },
	{ 0.3965859374999999, -0.13378125000000013, 0.003515625, 500, THEME_BEEJ, 1024, 1024, 0, 0, 0,0,0,0,0 },
	{ 0.3958608398437499, -0.13431445312500012, 0.0002197265625, 600, THEME_BEEJ, 1024, 1024, 0, 0, 0,0,0,0,0 },
	{ 0.39580558691381873, -0.1343048796035937, 5.1159076974727215e-14, 1500, THEME_BEEJ, 1024, 1024, 0, 0, 0,0,0,0,0 }
};

/**
 * Global information about the app
 */
struct app_context {
	struct mandelbrot_info mandelbrot_info;
	char *outfile_name;
	int quiet;
	int num_threads;
} app_context;

/**
 * Convert an escape time to a color base on theme "beej"
 */
void time_to_color_beej(long double t, int *r, int *g, int *b)
{
	if (t < 0) {
		*r = *g = *b = 0;
		return;
	}

	*r = (127 + sinl(t / 11.0) * 127) * 1.0;
	*g = (127 + sinl(t / 7.0) * 127) * 1.0;
	*b = (127 + sinl(t / 5.0) * 127) * 1.0;

	if (*r < 0) { *r = 0; } else if (*r > 255) { *r = 255; }
	if (*g < 0) { *g = 0; } else if (*g > 255) { *g = 255; }
	if (*b < 0) { *b = 0; } else if (*b > 255) { *b = 255; }
}

/**
 * Convert an escape time to a color base on theme "fire"
 */
void time_to_color_fire(long double t, int *r, int *g, int *b)
{
	if (t < 0) {
		*r = *g = *b = 0;
		return;
	}

	*r = (240 + sinl(t / 7.0) * 35);
	*g = (127 + sinl(t / 7.0) * 200);
	*b = (20 + sinl(t / 9.0) * 20);

	if (*r < 0) { *r = 0; } else if (*r > 255) { *r = 255; }
	if (*g < 0) { *g = 0; } else if (*g > 255) { *g = 255; }
	if (*b < 0) { *b = 0; } else if (*b > 255) { *b = 255; }
}

/**
 * Convert an escape time to a color base on theme "water"
 */
void time_to_color_water(long double t, int *r, int *g, int *b)
{
	if (t < 0) {
		*r = *g = *b = 0;
		return;
	}

	*r = (20 + sinl(t / 9.0) * 20);
	*b = (240 + sinl(t / 7.0) * 35);
	*g = (127 + sinl(t / 7.0) * 200);

	if (*r < 0) { *r = 0; } else if (*r > 255) { *r = 255; }
	if (*g < 0) { *g = 0; } else if (*g > 255) { *g = 255; }
	if (*b < 0) { *b = 0; } else if (*b > 255) { *b = 255; }
}

/**
 * Get the escape time for a particular imaginary coordinate
 */
long double get_escape_time(long double cr, long double ci, int max_iterations, int continuous)
{
	int iteration = 0;
	long double zr = 0;
	long double zi = 0;

	long double zr2;
	long double zi2;

	while (zr*zr + zi*zi <= (2*2) && iteration < max_iterations) {
		zr2 = zr*zr - zi*zi;
		zi2 = 2 * zr * zi;

		zr = zr2 + cr;
		zi = zi2 + ci;

		iteration++;
	}

	if (iteration < max_iterations) {
		if (continuous) {
			long double az = sqrtl(zr*zr + zi*zi);
			//long double log2az = logl(az) / logl(2);
			//long double log2log2az = logl(log2az) / logl(2);
			long double log2log2az = logl(logl(az)) / logl(2);

			//fprintf(stderr, "%Lf\n", iteration - log2log2az);
			return iteration - log2log2az;
		} else {
			return iteration;
		}
	}

	return -1;
}

/**
 * Print an error (or usage), and exit abnormally.
 */
void usage_exit(char *str) {
	int i;

	if (str != NULL) {
		fprintf(stderr, "%s: %s\n", APPNAME, str);
	} else {
		fprintf(stderr, "usage:   %s -o outfilename [options]\n", APPNAME);
		fprintf(stderr, "\nversion: %s\n\n", VERSION);
		fprintf(stderr, "   -e examplenum            example render (1-%d) (should be first option)\n", EXAMPLE_SCENE_COUNT);
		fprintf(stderr, "   -o outfilename           set output name\n");
		fprintf(stderr, "   -c re,im                 set center point\n");
		fprintf(stderr, "   -w width                 set real width\n");
		fprintf(stderr, "   -s width,height          set pixel output size\n");
		fprintf(stderr, "   -i iterations            set max iterations\n");
		fprintf(stderr, "   -a                       antialias\n");
		fprintf(stderr, "   -u                       continuous smoothing\n");
		fprintf(stderr, "   -q                       no informational output\n");
		fprintf(stderr, "   -v                       print version and exit\n");
		fprintf(stderr, "   -t numthreads            set number of threads\n");

		fprintf(stderr, "   -m theme                 set theme--names are:\n");

		for (i = 0; i < THEME_COUNT; i++) {
			fprintf(stderr, "                               %s\n", theme[i].name);
		}
	}

	exit(1);
}

/**
 * Parse the command line
 */
void parse_command_line(int argc, char **argv)
{
	int j, opt, example_num;
	int num_threads = -1;
	enum theme_type t;

	app_context.mandelbrot_info.center_re = DEFAULT_CENTER_REAL;
	app_context.mandelbrot_info.center_im = DEFAULT_CENTER_IMG;
	app_context.mandelbrot_info.width_re = DEFAULT_WIDTH_REAL;
	app_context.mandelbrot_info.max_iterations = DEFAULT_MAX_ITERATIONS;
	app_context.mandelbrot_info.theme = THEME_BEEJ;
	app_context.mandelbrot_info.width_px = DEFAULT_IMAGE_WIDTH;
	app_context.mandelbrot_info.height_px = DEFAULT_IMAGE_HEIGHT;
	app_context.mandelbrot_info.antialias = 0;
	app_context.mandelbrot_info.continuous = 0;
	app_context.outfile_name = "-";
	app_context.quiet = 0;
	app_context.num_threads = 1;

	while ((opt = getopt(argc, argv, "o:c:w:s:i:auqt:m:he:v")) != -1) {
		switch (opt) {
			case 'h':
				usage_exit(NULL);
				break;

			case 'v':
				fprintf(stdout, "%s: version %s\n", APPNAME, VERSION);
				exit(0);
				break;

			case 'o':  // outfile name
				app_context.outfile_name = optarg;
				break;

			case 'c': // center point
				if (sscanf(optarg, "%Lf,%Lf",
					&app_context.mandelbrot_info.center_re,
					&app_context.mandelbrot_info.center_im) != 2) {
					
					usage_exit(NULL);
				}
				break;

			case 'w': // width
				if (sscanf(optarg, "%Lf", &app_context.mandelbrot_info.width_re) != 1) {
					usage_exit(NULL);
				}
				break;

			case 'i': // max iterations
				if (sscanf(optarg, "%d", &app_context.mandelbrot_info.max_iterations) != 1) {
					usage_exit(NULL);
				}
				break;

			case 's': // output image size
				if (sscanf(optarg, "%lld,%lld", &app_context.mandelbrot_info.width_px,
					&app_context.mandelbrot_info.height_px) != 2) {
					
					usage_exit(NULL);
				}
				break;

			case 'a': // antialias
				app_context.mandelbrot_info.antialias = 1;
				break;

			case 'q': // quiet
				app_context.quiet = 1;
				break;

			case 'u': // continuous
				app_context.mandelbrot_info.continuous = 1;
				break;

			case 'e': // prerender example
				if (sscanf(optarg, "%d", &example_num) != 1) {
					usage_exit(NULL);
				}

				if (example_num < 1 || example_num > EXAMPLE_SCENE_COUNT) {
					usage_exit("example scene number out of range");
				}

				app_context.mandelbrot_info = example_scene[example_num-1];
				break;

			case 't': // num threads
				if (sscanf(optarg, "%d", &num_threads) != 1) {
					usage_exit(NULL);
				}

				if (num_threads < 1) {
					usage_exit("number of threads must be positive, silly!");
				}
				break;

			case 'm': // theme
				t = THEME_NONE;
				for (j = 0; j < THEME_COUNT; j++) {
					if (strcmp(theme[j].name, optarg) == 0) {
						t = j;
						break;
					}
				}
				if (t == THEME_NONE) { usage_exit(NULL); }
				app_context.mandelbrot_info.theme = t;
				break;

			default:
				usage_exit(NULL);
				break;
		}
	}

	// set max threads if requested
	if (num_threads > 0) {
		app_context.num_threads = num_threads;
		omp_set_num_threads(app_context.num_threads);
	}

	// make sure stdout not specified
	if (strcmp(app_context.outfile_name, "-") == 0) {
		usage_exit(NULL);
	}

	// calculate other image attributes from those given
	app_context.mandelbrot_info.height_im = app_context.mandelbrot_info.width_re * app_context.mandelbrot_info.height_px / app_context.mandelbrot_info.width_px;
	app_context.mandelbrot_info.left_re = app_context.mandelbrot_info.center_re - (app_context.mandelbrot_info.width_re / 2);
	app_context.mandelbrot_info.right_re = app_context.mandelbrot_info.center_re + (app_context.mandelbrot_info.width_re / 2);
	app_context.mandelbrot_info.top_im = app_context.mandelbrot_info.center_im + (app_context.mandelbrot_info.height_im / 2);
	app_context.mandelbrot_info.bottom_im = app_context.mandelbrot_info.center_im - (app_context.mandelbrot_info.height_im / 2);
}

/**
 * Set a file's length to this size.
 *
 * Returns -1 on error with errno set.
 */
int set_file_length(FILE *fp, off_t total_file_length)
{
	return ftruncate(fileno(fp), total_file_length);
}

/**
 * Memory map a file (for write).
 *
 * @return NULL on error, or a pointer to the data
 */
void *memory_map_file(FILE *fp, size_t total_file_length)
{
	void *addr;

	addr = mmap(NULL, total_file_length, PROT_WRITE,
		MAP_SHARED, fileno(fp), 0);

	if (addr == MAP_FAILED) {
		return NULL;
	}

	return addr;
}

/**
 * memory unmap a file
 */
int memory_unmap_file(void *addr, size_t len)
{
	return munmap(addr, len);
}

/**
 * Write a PPM header
 */
int write_ppm_header(FILE *fp)
{
	int len = 0;

	len += fprintf(fp, "P6\n"); /* raw RGB */
	len += fprintf(fp, "# Created by Goatbrot\n");
	len += fprintf(fp, "%lld %lld\n", app_context.mandelbrot_info.width_px, app_context.mandelbrot_info.height_px);
	len += fprintf(fp, "255\n");

	return len;
}

/**
 * Dump information about this particular render
 */
void output_process_info(void)
{
	fprintf(stderr, "Complex image:\n");
	fprintf(stderr, "            Center: %.*Lg + %.*Lgi\n", LDBL_DIG, app_context.mandelbrot_info.center_re, LDBL_DIG, app_context.mandelbrot_info.center_im);
	fprintf(stderr, "             Width: %.*Lg\n", LDBL_DIG, app_context.mandelbrot_info.width_re);
	fprintf(stderr, "            Height: %.*Lg\n", LDBL_DIG, app_context.mandelbrot_info.height_im);
	fprintf(stderr, "        Upper Left: %.*Lg + %.*Lgi\n", LDBL_DIG, app_context.mandelbrot_info.left_re, LDBL_DIG, app_context.mandelbrot_info.top_im);
	fprintf(stderr, "       Lower Right: %.*Lg + %.*Lgi\n", LDBL_DIG, app_context.mandelbrot_info.right_re, LDBL_DIG, app_context.mandelbrot_info.bottom_im);
	fprintf(stderr, "\nOutput image:\n");
	fprintf(stderr, "          Filename: %s\n", app_context.outfile_name);
	fprintf(stderr, "     Width, Height: %lld, %lld\n", app_context.mandelbrot_info.width_px, app_context.mandelbrot_info.height_px);
	fprintf(stderr, "             Theme: %s\n", theme[app_context.mandelbrot_info.theme].name);
	fprintf(stderr, "       Antialiased: %s\n", app_context.mandelbrot_info.antialias? "yes": "no");
	fprintf(stderr, "\nMandelbrot:\n");
	fprintf(stderr, "    Max Iterations: %d\n", app_context.mandelbrot_info.max_iterations);
	fprintf(stderr, "        Continuous: %s\n", app_context.mandelbrot_info.continuous? "yes": "no");
	fprintf(stderr, "\nGoatbrot:\n");
	fprintf(stderr, "       Num Threads: %d\n", omp_get_max_threads());
	fprintf(stderr, "\n");
}

/**
 * Calculate the set and write to a PPM
 */
void mandelbrot(struct mandelbrot_info *minfo, FILE *fp, int show_progress)
{
	int y;
	long long image_pixels = app_context.mandelbrot_info.width_px * app_context.mandelbrot_info.height_px;
	int total_pixels = 0;
    time_to_color_function time_to_color = theme[app_context.mandelbrot_info.theme].time_to_color;

	unsigned char *filedata, *pixeldata;
	int ppm_header_len;
	long long total_file_length;

	ppm_header_len = write_ppm_header(fp);
	fflush(fp);

	total_file_length = image_pixels * 3 + ppm_header_len;

	// pad the file up to its final length, to make ready for mmap:
	if (set_file_length(fp, total_file_length) == -1) {
		perror("set_file_length");
		exit(3);
	}

	// memory-map the file
	filedata = memory_map_file(fp, total_file_length);
	if (filedata == NULL) {
		perror("memory_map_file");
		exit(2);
	}

	pixeldata = filedata + ppm_header_len;

#pragma omp parallel for
	for (y = 0; y < minfo->height_px; y++) {
		// these must be declared in here to be thread-private for
		// OpenMP:
		int x;
		unsigned char rgb[3];
		long double cx1, cy1; // left, upper
		long double cx2, cy2=0; // right, lower
		int r[4], g[4], b[4]; // ul, ur, ll, lr
		long double t[4];
		
		unsigned char *xbasedata;
		unsigned char *ybasedata = y * minfo->width_px * 3 + pixeldata;

		cy1 = minfo->top_im - (minfo->height_im * y / minfo->height_px);

		if (minfo->antialias) {
			cy2 = minfo->top_im - 
				 (minfo->height_im * (y+0.5) / minfo->height_px);
		}

		for (x = 0; x < minfo->width_px; x++) {
			int max_iterations = app_context.mandelbrot_info.max_iterations;
			int continuous = app_context.mandelbrot_info.continuous;

			xbasedata = ybasedata + x * 3;

			cx1 = (minfo->width_re * x / minfo->width_px) +
				minfo->left_re;

			if (minfo->antialias) {
				cx2 = (minfo->width_re * (x+0.5) / minfo->width_px) +
					minfo->left_re;

				t[0] = get_escape_time(cx1, cy1, max_iterations, continuous);
				t[1] = get_escape_time(cx2, cy1, max_iterations, continuous);
				t[2] = get_escape_time(cx1, cy2, max_iterations, continuous);
				t[3] = get_escape_time(cx2, cy2, max_iterations, continuous);

				time_to_color(t[0], r+0, g+0, b+0);
				time_to_color(t[1], r+1, g+1, b+1);
				time_to_color(t[2], r+2, g+2, b+2);
				time_to_color(t[3], r+3, g+3, b+3);

				rgb[0] = (r[0] + r[1] + r[2] + r[3]) / 4;
				rgb[1] = (g[0] + g[1] + g[2] + g[3]) / 4;
				rgb[2] = (b[0] + b[1] + b[2] + b[3]) / 4;
			} else {
				t[0] = get_escape_time(cx1, cy1, max_iterations, continuous);
				time_to_color(t[0], r+0, g+0, b+0);
				rgb[0] = r[0];
				rgb[1] = g[0];
				rgb[2] = b[0];
			}

			/* write the pixel */
			memcpy(xbasedata, rgb, sizeof rgb);
		} /* for x */

		if (show_progress) {
			#pragma omp critical
			{
				total_pixels += minfo->width_px;
				fprintf(stderr, "   \rCompleted: %.1f%%",
					(100.0 * total_pixels / image_pixels));
				fflush(stderr);
			}
		}

	} /* for y */

	if (show_progress) {
		fprintf(stderr, "\n");
	}

	if (memory_unmap_file(filedata, total_file_length) == -1) {
		perror("memory_unmap_file");
		exit(2);
	}

	fclose(fp);
}

/**
 * Main
 */
int main(int argc, char **argv)
{
	FILE *fout;

	parse_command_line(argc, argv);

	if (!app_context.quiet) {
		output_process_info();
	}

	if (strcmp(app_context.outfile_name, "-") == 0) {
		fout = stdout;
	} else {
		fout = fopen(app_context.outfile_name, "w+b");
		if (fout == NULL) {
			usage_exit("error opening output file");
		}
	}

	mandelbrot(&app_context.mandelbrot_info, fout, !app_context.quiet);

	return 0;
}

