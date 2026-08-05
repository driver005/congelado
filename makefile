CONAN_PROVIDER=out/conan_provider.cmake
CMAKE_PRESET=Debug
BUILD_DIR=out/build/$(CMAKE_PRESET)

.PHONY: all download build clean info-outdated update editor dev test compose-env-up compose-env-rm

all: dev

dependency:
	yay -S xmake conan libc++ --noconfirm
	conan profile detect --force

install:
	xmake f -c -v -y

dev: build editor

config-debug:
	xmake f -m debug --debugger=gdb

debug: config-debug 
	xmake run -D -d congelado

run:
	xmake run

run-worker:
	xmake run congelado_worker worker.toml ./build/workers

run-worker-docker:
	xmake run congelado_worker worker.toml ./build/workers

build:
	xmake build

rebuild:
	xmake require --force
	xmake -r

#Configuration

windows:
	xmake f -p mingw --toolchain=mingw -c -v

linux:
	xmake f -p linux -c -v

benchmark:
	xmake run benchmark

test:
	xmake test -v

info-outdated:
	conan graph outdated . --out-file ./build/graph.txt

editor:
	xmake project -k compile_commands

compose-env-up:
	podman compose -f docker/docker-compose.environment.yml up -d

compose-env-rm:
	podman compose -f docker/docker-compose.environment.yml down

clean-conan:
	conan remove "*"

clean:
	xmake clean

clean-all: clean-conan
	rm -rf build/ ~/.xmake/
