.PHONY: all build leak run clean

all: build run

leak: build
	leaks -q --atExit -- ./build/DaGame

build:
	cmake -S . -B build
	cmake --build build

run:
	./build/DaGame

clean:
	rm -rf build
