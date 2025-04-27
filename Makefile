.PHONY: all-dev build-dev build-target run-dev run-target leak-dev leak-target clean

all-dev: build-dev run-dev

all-target: build-target run-target

leak-dev: build-dev
	leaks -q --atExit -- ./build-dev/DaGame

build-dev:
	cmake -DCMAKE_C_FLAGS="-DDEV_BUILD" -S . -B build-dev
	cmake --build build-dev
	ln -sf build-dev/compile_commands.json compile_commands.json

run-dev:
	./build-dev/DaGame

leak-target: build-target
	leaks -q --atExit -- ./build-target/DaGame

build-target:
	cmake -DCMAKE_C_FLAGS="-DTARGET_BUILD" -S . -B build-target
	cmake --build build-target
	ln -sf build-target/compile_commands.json compile_commands.json

run-target:
	./build-target/DaGame

clean:
	rm -rf build-dev
	rm -rf build-target
