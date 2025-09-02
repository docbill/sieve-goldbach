// pairrange2sgbll - generates HL-A prediction file
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

double hlCorr(uint64_t n,uint64_t delta) {
    double invlogNlogN = 1.0/(log(n)*log(n));
    double invSum = 0;
    double sum = 0;
    for(uint64_t m=1+(n&1);m <= delta;m+=2) {
        sum += 1.0/(log(n-m)*log(n+m));
	invSum += invlogNlogN;
    }
    return (invSum > 0)?(sum/invSum):1.0;
}

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

    uint64_t prime_array_len = ((uint64_t)st.st_size/sizeof(uint64_t));
    // uint64_t *current = prime_array; 

    double pairCountMin = 100.0;
    double pairCountMax = 0;
    double cpred_min = 20.0;
    double cpred_max = 0.0;
    uint64_t step = 1;
    uint64_t n_start = 4;
    uint64_t nextN = 5;
    uint64_t nextDecade = 10;
    int decade = 0;
    uint64_t minAt = 0;
    uint64_t maxAt = 0;
    uint64_t n_0 = 0;
    uint64_t n_1 = 0;
    uint64_t n_geom = n_start;
    double hlCorrAvg = 1.0;
    double pairCountTotal = 0;
    double cpred_sum = 0;
    int d=n_start;
    // for (uint64_t *p=prime_array, N=0;p < prime_array_end && N < end_n;p++,N++) {
    //    printf("Prime: %" PRIu64 "\n",*p);
    // }
    printf("DECADE,MIN AT,MIN,MAX AT,MAX,n_0,Cpred_min,n_1,Cpred_max,N_geom,<COUNT>,Cpred_avg,HLCorr\n");
    for (uint64_t n = n_start;n < end_n ;) {
        uint64_t delta = (n>>1);
        // uint64_t n_min = n-delta-1;
	double logN=log(n);
	double norm = delta/(logN*logN);
	double cpred = twoSGB(n,prime_array,prime_array_len);
	double pairCount = cpred*norm;
        cpred_sum += cpred;
        pairCountTotal += pairCount;
        //printf("%" PRIu64 ": count = %d,%.6f\n",n,pairCount,cpred);
        if(pairCount > pairCountMax) {
            pairCountMax = pairCount;
            maxAt = n;
        }
        if(pairCount < pairCountMin) {
            pairCountMin = pairCount;
            minAt = n;
        }
        if(cpred > cpred_max) {
            cpred_max = cpred;
            n_1 = n;
        }
        if(cpred < cpred_min) {
            cpred_min = cpred;
            n_0 = n;
        }
        if( ++n == nextN) {
	   pairCountTotal *= hlCorrAvg;
           cpred_sum *= hlCorrAvg;
	   pairCountMin *= hlCorr(minAt,minAt>>1);
	   pairCountMax *= hlCorr(maxAt,maxAt>>1);
	   cpred_min *= hlCorr(n_0,n_0>>1);
	   cpred_max *= hlCorr(n_1,n_0>>1);
           printf("%d,%" PRIu64 ",%.6f,%" PRIu64 ",%.6f,%" PRIu64 ",%.8f,%" PRIu64 ",%.8f,%" PRIu64 ",%.8f,%.8f,%.8f\n",
               decade,
               minAt, pairCountMin,
               maxAt, pairCountMax,
               n_0, cpred_min,
               n_1, cpred_max,
               n_geom, ((double)pairCountTotal)/(n-n_start),cpred_sum/(n-n_start),hlCorrAvg ); 
           fflush(stdout);
	   int k=decade;
	   if(++d == 10) {
	       k++;
               d=1;
	   }
	   n_geom = ((uint64_t)floorl(powl(10.0L, k)*sqrtl((long double)d*(d+1))))|((k == 0)?0ULL:1ULL);
	   hlCorrAvg = 0.5*(hlCorr(n_geom,n_geom>>1)+hlCorr(n_geom+1,(n_geom+1)>>1));
           if(n == nextDecade) {
               step *= 10;
               decade++;
               nextDecade *= 10;
           }
           nextN += step;
           if(nextN > end_n) {
               break;
           }
           pairCountMin = n <<3;
           pairCountMax = -1;
           minAt = 0;
           maxAt = 0;
           n_0 = 0;
           cpred_min = n << 32;
           n_1 = 0;
           cpred_max = -1.0;
           n_start = n;
           pairCountTotal = 0;
           cpred_sum = 0.0;
        }
    }

    munmap(prime_array, st.st_size);
    close(fd);
    exit(0);
}

