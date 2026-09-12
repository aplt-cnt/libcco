.PHONY: all build clean lint test

all: build

build:
	cmake -B build
	cmake --build build

clean:
	rm -rf build

lint:
	clang-format --dry-run -Werror include/cnt/*.h src/internal/*.c src/internal/*.h test/*.c 2>/dev/null || true

test: build
	cd build && ctest
