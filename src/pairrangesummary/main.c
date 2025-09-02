// pairrangesummary - counts goldbach pairs in a window
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
        fprintf(stderr,"Usage: %s <prime_raw_file> <end_n>\n",argv[0]);
        exit(1);
    }

    const char *prime_file = argv[1];
    const uint64_t end_n = strtoull(argv[2], NULL, 10);

    // Open and mmap prime file
    int fd = open(prime_file, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        exit(1);
    }

    uint64_t* prime_array = (uint64_t*)(
        mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)
    );

    if (prime_array == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }

    uint64_t *prime_array_end = prime_array + ((uint64_t)st.st_size/sizeof(uint64_t));
    uint64_t *current = prime_array; 

    uint64_t pairCountMin = 4;
    uint64_t pairCountMax = ~0ULL;
    double pairCountMinNorm = 20.0;
    double pairCountMaxNorm = 0.0;
    uint64_t step = 1;
    uint64_t n_start = 4;
    uint64_t n_geom = n_start;
    uint64_t nextN = 5;
    uint64_t nextDecade = 10;
    int decade = 0;
    int d=n_start;
    uint64_t minAt = 0;
    uint64_t maxAt = 0;
    uint64_t minNormAt = 0;
    uint64_t maxNormAt = 0;
    uint64_t pairCountTotal = 0;
    double pairCountTotalNorm = 0;
    // for (uint64_t *p=prime_array, N=0;p < prime_array_end && N < end_n;p++,N++) {
    //    printf("Prime: %" PRIu64 "\n",*p);
    // }
    int exitStatus = 0;
    printf("DECADE,MIN AT,MIN,MAX AT,MAX,n_0,C_min,n_1,C_max,n_geom,<COUNT>,C_avg\n");
    for (uint64_t n = n_start;n < end_n ;) {
        uint64_t delta = (n>>1);
        uint64_t n_min = n-delta-1;
        double logN = log(n);
        double norm = (logN*logN)/delta;
        uint64_t pairCount = countRangedPairs(n,n_min,&current,prime_array,prime_array_end);
	if(pairCount == ~0ULL) {
	    fprintf(stderr,"Failed to count pairs at %" PRIu64 "\n",n);
	    exitStatus = -1;
	    break;
	}
        pairCountTotal += pairCount;
        double pairCountNorm = pairCount*norm;
        pairCountTotalNorm += pairCountNorm;
        //printf("%" PRIu64 ": count = %d,%.6f\n",n,pairCount,pairCountNorm);
        if(pairCountMax == ~0ULL || pairCount > pairCountMax) {
            pairCountMax = pairCount;
            maxAt = n;
        }
        if(pairCount < pairCountMin) {
            pairCountMin = pairCount;
            minAt = n;
        }
        if(pairCountNorm > pairCountMaxNorm) {
            pairCountMaxNorm = pairCountNorm;
            maxNormAt = n;
        }
        if(pairCountNorm < pairCountMinNorm) {
            pairCountMinNorm = pairCountNorm;
            minNormAt = n;
        }
        if( ++n == nextN) {
           printf("%d,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%.6f,%" PRIu64 ",%.6f,%" PRIu64 ",%.6f,%.6f\n",
               decade,
               minAt, pairCountMin,
               maxAt, pairCountMax,
               minNormAt, pairCountMinNorm,
               maxNormAt, pairCountMaxNorm,
               n_geom, ((double)pairCountTotal)/(n-n_start),pairCountTotalNorm/(n-n_start) ); 
           fflush(stdout);
	   int k=decade;
	   if(++d == 10) {
	       k++;
               d=1;
	   }
	   n_geom = ((uint64_t)floorl(powl(10.0L, k)*sqrtl((long double)d*(d+1))))|((k == 0)?0ULL:1ULL);
           if(n == nextDecade) {
               step *= 10;
               decade++;
               nextDecade *= 10;
           }
           nextN += step;
           if(nextN > end_n) {
               break;
           }
           pairCountMin = n<<1;
           pairCountMax = ~0ULL;
           minAt = 0;
           maxAt = 0;
           minNormAt = 0;
           pairCountMinNorm = n+100;
           maxNormAt = 0;
           pairCountMaxNorm = -1.0;
           n_start = n;
           pairCountTotal = 0;
           pairCountTotalNorm = 0.0;
        }
    }

    munmap(prime_array, st.st_size);
    close(fd);
    exit(exitStatus);
}

