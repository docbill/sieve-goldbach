# Makefile - builds source
# Copyright (C) 2025 Bill C. Riemers
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# SPDX-License-Identifier: GPL-3.0-or-later

ROOT := $(CURDIR)

# --- toolchain defaults (can be overridden via env) ---
CC        ?= gcc
CFLAGS    ?= -O3 -Wall -Wextra -I$(ROOT)/lib

CXX       ?= g++
CXXFLAGS  ?= -O3 -Wall -Wextra -std=c++17 -I$(ROOT)/lib

CPPFLAGS  ?=
LDFLAGS   ?=
LDLIBS    ?= -lm

# --- OS detection for portability ---
UNAME_S := $(shell uname -s)

# macOS: prefer Homebrew/MacPorts g++-NN if available; add Brew include/lib
ifeq ($(UNAME_S),Darwin)
  GXX_VER := $(firstword $(shell command -v g++-{20..10} 2>/dev/null))
  ifneq ($(GXX_VER),)
    CXX := $(GXX_VER)
  endif
  HB_PREFIX := $(firstword $(wildcard /opt/homebrew /usr/local))
  ifneq ($(HB_PREFIX),)
    CPPFLAGS += -I$(HB_PREFIX)/include
    LDFLAGS  += -L$(HB_PREFIX)/lib
  endif
endif

AR       ?= ar
ARFLAGS  ?= rcs

PROGRAMS := primesieve_bitmap printprimes storeprimes \
            gbpairsummary \
            cpslowerbound findgbpairs certifyprimes certifygbpairs \
            validatepairrangesummary

LIBDIR  := lib
LIBSRC  := $(wildcard $(LIBDIR)/*.c)
LIBOBJ  := $(LIBSRC:.c=.o)
LIBFILE := $(LIBDIR)/libprime.a

.PHONY: all clean $(PROGRAMS) $(PROGRAMS:%=%.clean) help

# Default rule builds library + all programs
all: $(LIBFILE) $(PROGRAMS)

# --- Library rules ---
$(LIBFILE): $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^

# Pattern rule for building .o files in lib
$(LIBDIR)/%.o: $(LIBDIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# --- Per-program targets ---
# Each top-level program target depends on the static library first,
# then delegates to the sub-make. This lets you run, e.g.:
#   make certifyprimes
# from the repo root (or from a parent makefile).
$(PROGRAMS): %: $(LIBFILE)
	$(MAKE) -C $@ \
		CC="$(CC)" CFLAGS="$(CFLAGS)" \
		CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" \
		CPPFLAGS="$(CPPFLAGS)" LDFLAGS="$(LDFLAGS)" LDLIBS="$(LDLIBS)" \
		LIBFILE="../$(LIBFILE)"

# Optional: per-program cleans (useful for selective cleanup)
$(PROGRAMS:%=%.clean):
	$(MAKE) -C $(@:.clean=) clean

# Clean everything
clean:
	$(RM) $(LIBOBJ) $(LIBFILE)
	for dir in $(PROGRAMS); do $(MAKE) -C $$dir clean; done

# Friendly help
help:
	@echo "Targets:"
	@echo "  all                 Build lib and all programs"
	@echo "  <program>           Build just that program (and lib if needed)"
	@echo "  <program>.clean     Clean just that program's build artifacts"
	@echo "  clean               Clean lib and all sub-projects"
	@echo
	@echo "Programs:"
	@printf "  %s\n" $(PROGRAMS)

