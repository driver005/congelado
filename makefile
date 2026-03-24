CONAN_PROVIDER=out/conan_provider.cmake
CMAKE_PRESET=Debug
BUILD_DIR=out/build/$(CMAKE_PRESET)

.PHONY: all download build clean info-outdated update editor dev test

all: dev

dependency:
	yay -S xmake conan libc++ --noconfirm
	conan profile detect --force

install:
	xmake f -c -v

dev: build editor

run:
	xmake run

build:
	xmake build

benchmark:
	xmake run benchmark

test:
	xmake test -v

info-outdated:
	conan graph outdated . --out-file ./build/graph.txt

editor:
	xmake project -k compile_commands

clean-conan:
	conan remove "*" -f

clean:
	xmake clean
	# rm -rf build/ ~/.xmake/
