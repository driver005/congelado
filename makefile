CONAN_PROVIDER=out/conan_provider.cmake
CMAKE_PRESET=Debug
BUILD_DIR=out/build/$(CMAKE_PRESET)

.PHONY: all build

all: build

dependency:
	sudo pacman -S --noconfirm bazelisk

build:
	bazelisk build //...

editor:
	bazelisk run //:refresh_editor
