CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = cavern

SOURCES = main.cc generation/perlin.cc ray_marching/rayMarching.cc
OBJECTS = $(SOURCES:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
