# tests/Makefile

CC       ?= cc
CFLAGS   ?= -Wall -Wextra -Wpedantic
OPT      ?= -O0 -g3
CPPFLAGS += -I../include -I../src/internal

TARGET_DIR := ../target/tests
LIBCCO     := ../target/debug/liblibcco.a

UNIT_SRCS := $(wildcard unit/*.c)
UNIT_OBJS := $(patsubst %.c,$(TARGET_DIR)/%.o,$(UNIT_SRCS))
UNIT_BIN  := $(TARGET_DIR)/run_unit_tests

INTEG_SRC := integration/run_integration.c
INTEG_OBJ := $(TARGET_DIR)/integration/run_integration.o
INTEG_BIN := $(TARGET_DIR)/run_integration

FUZZ_SRCS := $(wildcard fuzz/*.c)
FUZZ_OBJS := $(patsubst %.c,$(TARGET_DIR)/%.o,$(FUZZ_SRCS))
FUZZ_BIN  := $(TARGET_DIR)/run_fuzzer

.PHONY: all clean unit_tests integration_tests fuzz_tests dirs

all: dirs unit_tests integration_tests

dirs:
	@mkdir -p $(TARGET_DIR)/unit $(TARGET_DIR)/integration $(TARGET_DIR)/fuzz

unit_tests: dirs $(UNIT_BIN)
	@echo "==> Running unit tests..."
	@$(UNIT_BIN)

$(UNIT_BIN): $(TARGET_DIR)/test_main.o $(UNIT_OBJS) $(LIBCCO)
	$(CC) $(CFLAGS) $(OPT) $^ -o $@ $(LDFLAGS)

integration_tests: dirs $(INTEG_BIN)
	@echo "==> Running integration tests..."
	@failed=0; \
	for f in integration/corpus/valid_*.cco; do \
		if [ -f "$$f" ]; then \
			$(INTEG_BIN) "$$f" >/dev/null 2>&1; \
			if [ $$? -ne 0 ]; then echo "FAIL: $$f should be valid"; failed=1; else echo "PASS: $$f"; fi \
		fi \
	done; \
	for f in integration/corpus/invalid_*.cco; do \
		if [ -f "$$f" ]; then \
			$(INTEG_BIN) "$$f" >/dev/null 2>&1; \
			if [ $$? -eq 0 ]; then echo "FAIL: $$f should be invalid"; failed=1; else echo "PASS: $$f"; fi \
		fi \
	done; \
	if [ $$failed -ne 0 ]; then exit 1; fi

$(INTEG_BIN): $(INTEG_OBJ) $(LIBCCO)
	$(CC) $(CFLAGS) $(OPT) $^ -o $@ $(LDFLAGS)

fuzz_tests: dirs $(FUZZ_BIN)
	@echo "==> Running fuzzer (limited runs)..."
	@if [ -x $(FUZZ_BIN) ]; then $(FUZZ_BIN) -runs=10000; else echo "Fuzzer not built."; fi

$(FUZZ_BIN): $(FUZZ_OBJS) $(LIBCCO)
	$(CC) $(CFLAGS) $(OPT) $^ -o $@ $(LDFLAGS)

$(TARGET_DIR)/%.o: %.c | dirs
	$(CC) $(CFLAGS) $(CPPFLAGS) $(OPT) -c $< -o $@

clean:
	rm -rf $(TARGET_DIR)
