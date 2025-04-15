.PHONY: all build run clean

all: build run

build:
	cmake -S . -B build
	cmake --build build

run:
	./build/RaylibBox2DFlecs

clean:
	rm -rf build
