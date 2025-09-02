// findgbpairs - generates a gbpairs file
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

    uint64_t n_start = 4;
    int exitStatus = 0;
    printf("2N,N-M,N+M,2M\n");
    printf("4,2,2,0\n");
    printf("6,3,3,0\n");
    for (uint64_t n = n_start;n < end_n ;n++) {
        int m = findPair(n,&current,prime_array,prime_array_end);
	if(m < 0) {
	    fprintf(stderr,"Failed to find pair at %" PRIu64 "\n",n);
	    exitStatus= -1;
	    break;
	}
        printf("%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%d\n",n<<1,n-m,n+m,m<<1);
    }

    munmap(prime_array, st.st_size);
    close(fd);
    exit(exitStatus);
}

