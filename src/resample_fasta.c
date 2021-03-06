/*
 * resample_fasta.c
 *
 * Description:
 *     Subsample a fraction (default 0.5) of a
 *     multiple-sequence alignment in fasta format.
 *     Reads a file, prints to STDOUT.
 *     Change fraction to sample ("XFRAC") below.
 *     Reminder: Sequences need to be aligned
 *     (same length).
 *     No extensive error checking: Caveat emptor!
 *
 * Compile:
 *     gcc -Wall -O3 -o refas resample_fasta.c
 *
 * Usage:
 *     ./refas infile > outfile.fas
 *
 * By:
 *     Johan.Nylander@{nbis|nrm}.se
 *
 * Version:
 *    Thu 15 Feb 2018 10:28:22 AM CET
 * 
 * Thanks to:
 *    Andreas Kähäri for code revision
 *
 * License and copyright:
 *    Copyright (c) 2018-2020 Johan Nylander
                  
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or
 *    sell copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 *    
 *    The above copyright notice and this permission notice shall be
 *    included in all copies or substantial portions of the Software.
 *    
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define XFRAC 0.5 // fraction to sample
#define WRAP 60   // line wrap for fasta seq

int main(int argc, char *argv[]) {

    FILE *fp;
    long int seqlength;
    long int seqlen;
    long int samplesize;
    long int *randvals;
    long int im, in;
    long int j, c;
    int inheader;
    int ngts;
    int r, k;

    if (argc == 1) {
        fprintf(stderr, "Usage: %s <infile>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fp = fopen(argv[1], "r");

    if (fp == NULL) {
        perror("Error: failed in opening file");
        exit(EXIT_FAILURE);
    }

    // Read the first sequence to get sequence length
    inheader = 0;
    ngts = 0;
    seqlength = 0;
    while ((r = fgetc(fp)) != EOF) {
        if (inheader == 1) {
            if (r == '\n') {
                inheader = 0;
            }
        }
        else if (r == '>') {
            ++ngts;
            if (ngts > 1) {
                break;
            }
            else {
                inheader = 1;
            }
        }
        else {
            if (!isspace(r)) {
                seqlength++;
            }
        }
    }

    if (seqlength == 0) {
        fprintf(stderr, "Error: No sequences found in file.\n");
        exit(EXIT_FAILURE);
    }

    rewind(fp);

    samplesize = (long int)(seqlength * XFRAC);

    // Allocate for array. Possible for how large data?
    randvals = malloc(samplesize * sizeof *randvals);

    if (randvals == NULL) {
        perror("Error allocating memory for array randvals");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    // Use Knuth's algorithm to get sorted unique random numbers
    for (in = im = 0; in < seqlength && im < samplesize; ++in) {
        long int rn = seqlength - in;
        long int rm = samplesize - im;
        if (rand() % rn < rm) {
            randvals[im++] = in;
        }
    }

    // Read file again
    inheader = 0;
    ngts = 0;
    seqlen = 0;
    j = c = k = 0;
    while ((r = fgetc(fp)) != EOF) {
        if (inheader == 1) {
            if (r == '\n') {
                inheader = 0;
            }
            putchar(r);
        }
        else if (r == '>') {
            ++ngts;
            if (ngts > 1) {
                putchar('\n');
                if (seqlen != seqlength) { // test if equal length to first seq
                    fprintf(stderr,
                        "Error! Sequence lengths not equal (%li vs %li).\nAborting\n",
                        seqlen, seqlength);
                    free(randvals);
                    exit(EXIT_FAILURE);
                }
                ngts = 1;
            }
            putchar(r);
            inheader = 1;
            j = c = k = 0;
            seqlen = 0;
        }
        else { // r is a sequence character, or a newline in the sequence
            if (r != '\n') {
                ++seqlen;
                if (j == randvals[c]) {
                    putchar(r);
                    if (k > 0) {
                        if (k % WRAP == 0) {
                            putchar('\n');
                            k = 0;
                        }
                    }
                    ++k;
                    ++c;
                }
                ++j;
            }
        }
    }
    putchar('\n');
    
    fclose(fp);
    free(randvals);

    return EXIT_SUCCESS;
}

