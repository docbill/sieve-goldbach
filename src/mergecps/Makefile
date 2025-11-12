CXX = /opt/homebrew/bin/g++-15
CXXFLAGS = -O3 -Wall -Wextra -std=c++17
LDFLAGS = 
LDLIBS = -lm

TARGET = mergecps
SOURCE = mergecps.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean
