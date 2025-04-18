.PHONY: all build leak run clean

all: build run

leak: build
	leaks -q --atExit -- ./build/RaylibBox2DFlecs

build:
	cmake -S . -B build
	cmake --build build

run:
	./build/RaylibBox2DFlecs

clean:
	rm -rf build
