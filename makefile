CONAN_PROVIDER=out/conan_provider.cmake
CMAKE_PRESET=Debug
BUILD_DIR=out/build/$(CMAKE_PRESET)

.PHONY: all download build clean info-outdated update

all: build

dependency:
	yay -S cmake conan --noconfirm
	conan profile detect --force

download:
	@if [ ! -f "$(CONAN_PROVIDER)" ]; then \
		mkdir -p out; \
		curl -L -o "$(CONAN_PROVIDER)" https://raw.githubusercontent.com/conan-io/cmake-conan/develop2/conan_provider.cmake; \
		echo "Downloaded conan_provider.cmake"; \
	else \
		echo "conan_provider.cmake already exists"; \
	fi

install: download update
	cmake --workflow --preset $(CMAKE_PRESET)

build:
	cmake --build $(BUILD_DIR)

info-outdated:
	conan graph outdated . --out-file ./conan/graph.txt

update:
	conan install . -s build_type=$(CMAKE_PRESET) -of=$(BUILD_DIR)/conan --update --build=missing

clean:
	rm -rf out
	# conan remove "*" -c
