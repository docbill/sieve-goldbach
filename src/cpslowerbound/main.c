// cpslowerbound - calculates lower bounds for a list of n values
// Copyright (C) 2025 Bill C. Riemers
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include "libprime.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr,"Usage: %s <prime_raw_file> <inputlist>\n",argv[0]);
        exit(1);
    }

    const char *prime_file = argv[1];
    const char *input_file = argv[2];
    // const uint64_t end_n = strtoull(argv[2], NULL, 10);

    FILE *fin = fopen(input_file,"r");
    if(fin == NULL) {
        perror("fopen");
        exit(1);
    }
    // Open and mmap prime file
    int fd = open(prime_file, O_RDONLY);
    if (fd < 0) {
        perror("open");
        fclose(fin);
        exit(1);
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        fclose(fin);
        close(fd);
        exit(1);
    }

    uint64_t* prime_array = (uint64_t*)(
        mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)
    );

    if (prime_array == MAP_FAILED) {
        perror("mmap");
        fclose(fin);
        close(fd);
        exit(1);
    }

    if(prime_array[0] != 2) {
        fprintf(stderr, "Error: The first prime in %s must be 2.\n",prime_file);
        fclose(fin);
        munmap(prime_array, st.st_size);
        close(fd);
        exit(1);
    }


    uint64_t *prime_array_end = prime_array + ((uint64_t)st.st_size/sizeof(uint64_t));
    uint64_t *prime_array_last = prime_array_end-1;
    uint64_t *current = prime_array; 

    long double l32 = log((long double)(3.0L/2.0L));
    uint64_t n=0;
    // primePtr1[0] is reserved for sanity-checking sorted input
    // primePtr1[1] is the actual current prime
    uint64_t *primePtr1 = prime_array;
    long double prodSeries1 = 1.0L;
    // primePtr2[0] is reserved for sanity-checking sorted input
    // primePtr2[1] is the actual current prime
    uint64_t *primePtr2 = prime_array;
    long double prodSeries2 = 1.0L;
    printf("Dec.,n_0,Cmin,Cminus,Cmin-Cminus,CminusAsym,Cmin-CminusAsym\n");
    long double invlog10 = 1.0L/log(10.0L);
    while(fscanf(fin,"%" PRIu64,&n) == 1) {
        uint64_t y2 = (uint64_t)floor(sqrt(1.5L*(long double)n));
        if(primePtr2[0] > y2) {
           primePtr2 = prime_array; 
           prodSeries2 = 1.0L;
        }
        for(;primePtr2[1] < 3;primePtr2++);
        for(;primePtr2 < prime_array_last && primePtr2[1] <= y2;primePtr2++) {
            prodSeries2 *= 1.0L - 1.0L/((long double)(primePtr2[1]-1));
        }
        if(primePtr2 == prime_array_last) {
            fprintf(stderr, "Error: more primes needed for n=%" PRIu64 "\n", n);
            break;
        }
        uint64_t y1 = (uint64_t)floor(sqrt((long double)n));
        if(primePtr1[0] > y1) {
           primePtr1 = prime_array; 
           prodSeries1 = 1.0L;
        }
        for(;primePtr1[1] < 3;primePtr1++);
        for(;primePtr1[1] <= y1;primePtr1++) {
            prodSeries1 *= 1.0L - 1.0L/((long double)(primePtr1[1]-1));
        }
        long double logN  = log((long double)n);
        long double ln3  = l32 + logN;
	long double logNlogN = logN*logN;
        long double Cminus = logNlogN*prodSeries1*prodSeries2;
        long double CminusAsym = KPRODKPROD*logN/ln3;
        uint64_t delta = (n>>1);
        uint64_t n_min = n-delta-1;
        uint64_t pairCount = countRangedPairs(n,n_min,&current,prime_array,prime_array_end);
        long double Cmin = ((logNlogN)/delta)*pairCount;
	int decade = (int)(logN*invlog10);
        printf("%d,%" PRIu64 ",%0.6LF,%0.6LF,%0.6LF,%0.6LF,%0.6LF\n",
            decade,n,Cmin,Cminus,Cmin-Cminus,CminusAsym,Cmin-CminusAsym);
    }
    fclose(fin);
    munmap(prime_array, st.st_size);
    close(fd);
    exit(0);
}

