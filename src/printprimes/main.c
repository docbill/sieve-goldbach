// printprimes - output primes to the commandline for manual verification
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
    if (argc != 4) {
        fprintf(stderr,"Usage: %s <prime_bitmap_file> <start_N> <end_N>\n",argv[0]);
        exit(1);
    }

    const char *prime_file = argv[1];
    const uint64_t start_N = strtoull(argv[2], NULL, 10);
    const uint64_t end_N = strtoull(argv[3], NULL, 10);

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

    uint8_t* prime_bitmap = (uint8_t*)(
        mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)
    );


    if (prime_bitmap == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }

    for (uint64_t N = (start_N < 3)?3:start_N; N <= end_N;N++) {
        if((N&1)&&is_odd_prime_fast(N,prime_bitmap)) {
            printf("%" PRIu64 "\n",N);
        }
    }

    munmap(prime_bitmap, st.st_size);
    close(fd);
    exit(0);
}

