.PHONY: all

setup:
	conan install conan --output-folder=out --build=missing --profile=./conan/congelado

all:
	cmake --workflow --preset="default"