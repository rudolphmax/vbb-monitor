git-hooks-install:
	pre-commit install

install: git-hooks-install
	conan install . --build=missing -of=build --settings=build_type=Release

install-debug: git-hooks-install
	conan install . --build=missing -of=build --settings=build_type=Debug

prebuild:
	pre-commit --version
	cd ./build

postbuild:

cmake-build-release:
	cmake --preset conan-release
	cmake --build --preset conan-release

cmake-build-debug: prebuild
	cmake --preset conan-debug
	cmake --build --preset conan-debug

build: prebuild cmake-build-release postbuild

build-debug: prebuild cmake-build-debug postbuild

run: build
	clear
	./build/vbb_monitor
