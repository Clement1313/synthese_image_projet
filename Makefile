CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = cavern
.DEFAULT_GOAL := all

SOURCES = \
	main.cc \
	generation/perlin.cc \
	vector3/Vector3.cc \
	image/Image.cc \
	camera/Camera.cpp \
	scene/scene.cc \
	ray_marching/rayMarching.cc \
	ray_marching/SDFPrimitives.cc
OBJECTS = $(patsubst %.cc,%.o,$(filter %.cc,$(SOURCES))) $(patsubst %.cpp,%.o,$(filter %.cpp,$(SOURCES)))

camera/Camera.o: camera/Camera.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
