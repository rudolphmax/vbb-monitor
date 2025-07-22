install:
	conan install . --build=missing --output-folder=build --settings=build_type=Release

install-debug:
	conan install . --build=missing --output-folder=build --settings=build_type=Debug

build: install
	cd ./build
	cmake --preset conan-release
	cmake --build --preset conan-release

build-debug: install-debug
	cd ./build
	cmake --preset conan-debug
	cmake --build --preset conan-debug

run: build
	clear
	./build/bvg_monitor
