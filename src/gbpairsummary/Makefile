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

# Assumes LIBFILE is passed from the top Makefile

TARGET = gbpairsummary
SRC = gbpairsummary.cpp gbaggregate.cpp gbrange.cpp gbdecade.cpp gbprimorial.cpp gbwindow.cpp eulerproductseries.cpp gblongintervalsummary.cpp
INCL = gblongintervalsummary.hpp gbaggregate.hpp gbdecade.hpp gbprimorial.hpp gbrange.hpp gbwindow.hpp hlcorrstate.hpp eulerproductseries.hpp

all: $(TARGET)

$(TARGET): $(SRC) $(LIBFILE) $(INCL)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LIBFILE) -lm
#	$(CC) $(CFLAGS) -o $@ $(SRC) $(LIBFILE) -lm

clean:
	$(RM) $(TARGET)

