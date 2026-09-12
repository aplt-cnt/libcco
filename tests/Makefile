# tests/Makefile

CC       ?= cc
CFLAGS   ?= -Wall -Wextra -Wpedantic
OPT      ?= -O0 -g3
CPPFLAGS += -I../include -I../src/internal

# The main library archive we link against
LIBCCO   := ../build/debug/liblibcco.a

# Find all unit tests
UNIT_SRCS := $(wildcard unit/*.c)
UNIT_OBJS := $(UNIT_SRCS:.c=.o)
UNIT_BIN  := run_unit_tests

# Integration
INTEG_SRC := integration/run_integration.c
INTEG_OBJ := $(INTEG_SRC:.c=.o)
INTEG_BIN := run_integration

# Fuzzers
FUZZ_SRCS := $(wildcard fuzz/*.c)
FUZZ_OBJS := $(FUZZ_SRCS:.c=.o)
FUZZ_BIN  := run_fuzzer

.PHONY: all clean unit_tests integration_tests fuzz_tests

all: unit_tests integration_tests

unit_tests: $(UNIT_BIN)
	@echo "==> Running unit tests..."
	@./$(UNIT_BIN)

$(UNIT_BIN): test_main.o $(UNIT_OBJS) $(LIBCCO)
	$(CC) $(CFLAGS) $(OPT) $^ -o $@ $(LDFLAGS)

integration_tests: $(INTEG_BIN)
	@echo "==> Running integration tests..."
	@failed=0; \
	for f in integration/corpus/valid_*.cco; do \
		if [ -f "$$f" ]; then \
			./$(INTEG_BIN) "$$f" >/dev/null 2>&1; \
			if [ $$? -ne 0 ]; then echo "FAIL: $$f should be valid"; failed=1; else echo "PASS: $$f"; fi \
		fi \
	done; \
	for f in integration/corpus/invalid_*.cco; do \
		if [ -f "$$f" ]; then \
			./$(INTEG_BIN) "$$f" >/dev/null 2>&1; \
			if [ $$? -eq 0 ]; then echo "FAIL: $$f should be invalid"; failed=1; else echo "PASS: $$f"; fi \
		fi \
	done; \
	if [ $$failed -ne 0 ]; then exit 1; fi

$(INTEG_BIN): $(INTEG_OBJ) $(LIBCCO)
	$(CC) $(CFLAGS) $(OPT) $^ -o $@ $(LDFLAGS)

fuzz_tests: $(FUZZ_BIN)
	@echo "==> Running fuzzer (limited runs)..."
	@if [ -x ./$(FUZZ_BIN) ]; then ./$(FUZZ_BIN) -runs=10000; else echo "Fuzzer not built."; fi

# To build fuzzer, one typically uses: CC=clang CFLAGS="-fsanitize=fuzzer,address" make fuzz_tests
$(FUZZ_BIN): $(FUZZ_OBJS) $(LIBCCO)
	$(CC) $(CFLAGS) $(OPT) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(OPT) -c $< -o $@

clean:
	rm -f $(UNIT_BIN) $(INTEG_BIN) $(FUZZ_BIN) test_main.o $(UNIT_OBJS) $(INTEG_OBJ) $(FUZZ_OBJS)
