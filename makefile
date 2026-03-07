CONAN_PROVIDER=out/conan_provider.cmake
CMAKE_PRESET=Debug
BUILD_DIR=out/build/$(CMAKE_PRESET)

.PHONY: all download build clean info-outdated update editor test

all: build

dependency:
	yay -S xmake conan libc++ --noconfirm
	conan profile detect --force

install:
	xmake f -c -v

build:
	xmake run

benchmark:
	xmake run benchmark

test:
	xmake test -v

info-outdated:
	conan graph outdated . --out-file ./build/graph.txt

editor:
	xmake project -k compile_commands

clean:
	rm -rf build/ ~/.conan2/ ~/.xmake/
