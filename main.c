/*
    This file is part of snd2nes.

    snd2nes is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    snd2nes is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with snd2nes.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static char program_version[] = "snd2nes 1.0";

/* Prints usage message and exits. */
static void usage()
{
    printf(
        "Usage: snd2nes [--output=FILE]\n"
        "               [--verbose]\n"
        "               [--help] [--usage] [--version]\n"
        "               FILE\n");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("Usage: snd2nes [OPTION...] FILE\n"
           "snd2nes converts sound files to NES DMC format.\n\n"
           "Options:\n\n"
           "  --output=FILE                   Store output in FILE\n"
           "  --verbose                       Print progress information to standard output\n"  
           "  --help                          Give this help list\n"
           "  --usage                         Give a short usage message\n"
           "  --version                       Print program version\n");
    exit(0);
}

/* Prints version and exits. */
static void version()
{
    printf("%s\n", program_version);
    exit(0);
}

extern void snd2nes(const char *, FILE *);

/**
  Program entrypoint.
*/
int main(int argc, char *argv[])
{
    int verbose = 0;
    const char *input_filename = 0;
    const char *output_filename = 0;
    /* Process arguments. */
    {
        char *p;
        while ((p = *(++argv))) {
            if (!strncmp("--", p, 2)) {
                const char *opt = &p[2];
                if (!strncmp("output=", opt, 7)) {
                    output_filename = &opt[7];
                } else if (!strcmp("verbose", opt)) {
                    verbose = 1;
                } else if (!strcmp("help", opt)) {
                    help();
                } else if (!strcmp("usage", opt)) {
                    usage();
                } else if (!strcmp("version", opt)) {
                    version();
                } else {
                    fprintf(stderr, "snd2nes: unrecognized option `%s'\n"
			    "Try `snd2nes --help' or `snd2nes --usage' for more information.\n", p);
                    return(-1);
                }
            } else {
                input_filename = p;
            }
        }
    }

    if (!input_filename) {
        fprintf(stderr, "snd2nes: no filename given\n"
                        "Try `snd2nes --help' or `snd2nes --usage' for more information.\n");
        return(-1);
    }

    {
	FILE *out;
        if (!output_filename)
            out = stdout;
        else
            out = fopen(output_filename, "wt");
        if (!out) {
            fprintf(stderr, "snd2nes: failed to open `%s' for writing\n", output_filename);
            return(-1);
        }
        snd2nes(input_filename, out);
        fclose(out);
    }


    return 0;
}
