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

# --- toolchain defaults (can be overridden via env) ---
CC        ?= gcc
CFLAGS    ?= -O3 -Wall -Wextra -I../lib

CXX       ?= g++
CXXFLAGS  ?= -O3 -Wall -Wextra -std=c++17 -I../lib

CPPFLAGS  ?=
LDFLAGS   ?=
LDLIBS    ?= -lm

# HLCorr exact calculation flag (set to 1 to use exact calculations)
HLCORR_USE_EXACT ?=

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

TARGET = gbpairsummary
SRC = gbpairsummary.cpp gbaggregate.cpp gbrange.cpp gbdecade.cpp gbprimorial.cpp gbwindow.cpp eulerproductseries.cpp gblongintervalsummary.cpp availabledeficit.cpp
INCL = gblongintervalsummary.hpp gbaggregate.hpp gbdecade.hpp gbprimorial.hpp gbrange.hpp gbwindow.hpp hlcorrstate.hpp hlcorrinterp.hpp eulerproductseries.hpp availabledeficit.hpp
LIBFILE = ../lib/libprime.a

all: $(TARGET)

$(TARGET): $(SRC) $(INCL) $(LIBFILE)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(if $(HLCORR_USE_EXACT),-DHLCORR_USE_EXACT,) -o $@ $(SRC) $(LIBFILE) $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(TARGET)

