CONAN_PROVIDER=out/conan_provider.cmake
CMAKE_PRESET=Debug
BUILD_DIR=out/build/$(CMAKE_PRESET)

.PHONY: all download build clean info-outdated update

all: workflow

download:
	@if [ ! -f "$(CONAN_PROVIDER)" ]; then \
		mkdir -p out; \
		curl -L -o "$(CONAN_PROVIDER)" https://raw.githubusercontent.com/conan-io/cmake-conan/develop2/conan_provider.cmake; \
		echo "Downloaded conan_provider.cmake"; \
	else \
		echo "conan_provider.cmake already exists"; \
	fi

workflow: download
	cmake --workflow --preset $(CMAKE_PRESET)

build:
	cmake --build $(BUILD_DIR)

info-outdated:
	conan graph outdated . --out-file ./conan/graph.txt

update:
	conan install . -s build_type=$(CMAKE_PRESET) -of=$(BUILD_DIR)/conan --update --build=missing

clean:
	rm -rf out
	rm -rf cmake/conan_provider.cmake
