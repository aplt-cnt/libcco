.PHONY: all build test lint format clean coverage help

BUILD_DIR   ?= build
BUILD_TYPE  ?= RelWithDebInfo
CC          ?= cc
SANITIZER   ?=

# GCC accepts -fsanitize=address,undefined (comma separated).
# Accept "+" as an alias for "," so the same value works in CI matrix
# keys and in shell one-liners without quoting surprises.
comma := ,
SANITIZER_LIST := $(subst +,$(comma),$(SANITIZER))

CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_C_COMPILER=$(CC)
ifneq ($(SANITIZER),)
CMAKE_FLAGS += "-DCMAKE_C_FLAGS=-fsanitize=$(SANITIZER_LIST) -fno-omit-frame-pointer -g"
endif

all: help

help:
	@echo "libcco targets:"
	@echo "  make build     - configure and build the library"
	@echo "  make test      - build and run the test suite"
	@echo "  make lint      - format, tidy, and static analysis"
	@echo "  make format    - apply clang-format in place"
	@echo "  make coverage  - capture coverage and emit HTML"
	@echo "  make clean     - remove build artifacts"
	@echo ""
	@echo "Variables:"
	@echo "  CC=<cc>            compiler (default: cc)"
	@echo "  BUILD_TYPE=<cfg>   CMake build type (default: RelWithDebInfo)"
	@echo "  SANITIZER=<list>   address+undefined, memory, ..."

build:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)
	cmake --build $(BUILD_DIR) -j

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

# Lint runs each tool when present, and prints a warning when a tool
# is missing on the local machine. CI installs the full set; local
# contributors are not required to have every tool installed.
lint:
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format --dry-run -Werror $$(find src include -name "*.c" -o -name "*.h"); \
	else \
		echo "WARN: clang-format not found, skipping"; \
	fi
	@if command -v clang-tidy >/dev/null 2>&1; then \
		clang-tidy $$(find src -name "*.c") -- -std=c11; \
	else \
		echo "WARN: clang-tidy not found, skipping"; \
	fi
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --error-exitcode=1 --suppress=missingIncludeSystem src/; \
	else \
		echo "WARN: cppcheck not found, skipping"; \
	fi
	@bash scripts/check-headers.sh
	@bash scripts/check-api-doc.sh
	@bash scripts/diff-feature-macros.sh

format:
	clang-format -i $$(find src include tests examples -name "*.c" -o -name "*.h" -o -name "*.hpp" -o -name "*.cpp")

coverage: build
	lcov --capture --directory $(BUILD_DIR) --output-file coverage.info
	genhtml coverage.info --output-directory coverage/

clean:
	rm -rf $(BUILD_DIR) coverage/ coverage.info
