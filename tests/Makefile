# Copyright 2026 Yogev Neumann
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Yogev Neumann
# =============================================================================
# J2735 High-Performance Toolkit - Test Makefile
# =============================================================================
# Zero-copy, in-place parser for SAE J2735 UPER encoding
# Test suite build and validation.
# =============================================================================

# Directory structure (relative to this Makefile's location)
MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
ROOT_DIR     := $(realpath $(MAKEFILE_DIR)..)
SRC_DIR      := $(ROOT_DIR)/src
BUILD_DIR    := $(ROOT_DIR)/build

# -----------------------------------------------------------------------------
# Autodiscovery - scales as project grows
# -----------------------------------------------------------------------------
HEADERS     := $(wildcard $(SRC_DIR)/*.h)
SRC_C_FILES := $(wildcard $(SRC_DIR)/*.c)

# Test file organization:
#   J2735_run_tests.c     - Main entry point with main()
#   J2735_*_test.c        - Individual test suites (no main())
#   unity.c               - Third-party test framework
TEST_MAIN   := J2735_run_tests.c
TEST_SUITES := $(wildcard *_test.c)
LIB_SRCS    := $(filter-out $(TEST_MAIN) $(TEST_SUITES),$(wildcard *.c))

# Build artifacts (all in BUILD_DIR)
LIB_OBJS        := $(LIB_SRCS:%.c=$(BUILD_DIR)/%.o)
SUITE_OBJS      := $(TEST_SUITES:%.c=$(BUILD_DIR)/%.o)
TEST_BIN_NAME   := J2735_test
TEST_BIN        := $(BUILD_DIR)/$(TEST_BIN_NAME)
TEST_BIN_ASAN   := $(BUILD_DIR)/$(TEST_BIN_NAME)_asan
TEST_BIN_UBSAN  := $(BUILD_DIR)/$(TEST_BIN_NAME)_ubsan
TEST_BIN_VALGND := $(BUILD_DIR)/$(TEST_BIN_NAME)_valgrind

# Separate project code from third-party frameworks
PROJECT_C_FILES := $(SRC_C_FILES) $(TEST_MAIN) $(TEST_SUITES)
FRAMEWORK_FILES := unity.c

# All files
ALL_C_FILES := $(PROJECT_C_FILES) $(FRAMEWORK_FILES)
ALL_H_FILES := $(HEADERS)

# Include path for headers
INCLUDES := -I$(SRC_DIR)

# -----------------------------------------------------------------------------
# Compiler Detection & Version Enforcement
# Minimum: GCC 11 (2021-07-28) / Clang 13 (2022-02-02)
# -----------------------------------------------------------------------------
COMPILER_ID := $(shell $(CC) --version 2>&1 | head -1)
IS_CLANG := $(findstring clang,$(COMPILER_ID))
IS_GCC := $(findstring gcc,$(COMPILER_ID))

ifdef IS_CLANG
    COMPILER_VERSION := $(shell $(CC) -dumpversion 2>/dev/null | cut -d. -f1)
    MIN_VERSION := 13
    COMPILER_NAME := Clang
else ifdef IS_GCC
    COMPILER_VERSION := $(shell $(CC) -dumpversion 2>/dev/null | cut -d. -f1)
    MIN_VERSION := 11
    COMPILER_NAME := GCC
else
    $(error Unsupported compiler $(CC). Use GCC or Clang.)
endif

# Version check (empty version = detection failed)
ifeq ($(COMPILER_VERSION),)
    $(error Could not detect $(COMPILER_NAME) version. Ensure $(CC) is in PATH.)
endif

VERSION_OK := $(shell test $(COMPILER_VERSION) -ge $(MIN_VERSION) 2>/dev/null && echo yes)
ifneq ($(VERSION_OK),yes)
    $(error $(COMPILER_NAME) $(COMPILER_VERSION) is too old. Minimum required: $(COMPILER_NAME) $(MIN_VERSION)+)
endif

# -----------------------------------------------------------------------------
# Warning Flags - Version-aware for maximum compatibility
# -----------------------------------------------------------------------------
WFLAGS := -Wall
WFLAGS += -Waggregate-return
WFLAGS += -Walloca
WFLAGS += -Wbad-function-cast
WFLAGS += -Wcast-align
WFLAGS += -Wcast-qual
WFLAGS += -Wconversion
WFLAGS += -Wdate-time
WFLAGS += -Wdisabled-optimization
WFLAGS += -Wdouble-promotion
WFLAGS += -Werror
WFLAGS += -Wextra
WFLAGS += -Wfloat-equal
WFLAGS += -Wformat-nonliteral
WFLAGS += -Wformat-security
WFLAGS += -Wformat=2
WFLAGS += -Winit-self
WFLAGS += -Winline
WFLAGS += -Winline
WFLAGS += -Wmissing-declarations
WFLAGS += -Wmissing-include-dirs
WFLAGS += -Wmissing-prototypes
WFLAGS += -Wmultichar
WFLAGS += -Wnested-externs
WFLAGS += -Wnull-dereference
WFLAGS += -Wold-style-definition
WFLAGS += -Wpadded
WFLAGS += -Wpedantic
WFLAGS += -Wpointer-arith
WFLAGS += -Wredundant-decls
WFLAGS += -Wshadow
WFLAGS += -Wstack-protector
WFLAGS += -Wstrict-overflow=5
WFLAGS += -Wstrict-prototypes
WFLAGS += -Wswitch-default
WFLAGS += -Wswitch-enum
WFLAGS += -Wundef
WFLAGS += -Wuninitialized
WFLAGS += -Wunused-macros
WFLAGS += -Wvla
WFLAGS += -Wwrite-strings

ifdef IS_CLANG
    # Clang: Enable ALL warnings, disable only what conflicts with project design
    WFLAGS := -Weverything -Werror
    # C99 feature we intentionally use (declarations after statements)
    WFLAGS += -Wno-declaration-after-statement
    ifeq ($(shell expr $(COMPILER_VERSION) \>= 16), 1)
        # Zero-copy buffer parsing requires pointer arithmetic by design (Clang 16+)
        WFLAGS += -Wno-unsafe-buffer-usage
    endif
else
    # GCC-specific - available in GCC 11+
    WFLAGS += -Walloc-zero
    WFLAGS += -Wanalyzer-too-complex
    WFLAGS += -Warith-conversion
    WFLAGS += -Wcast-align=strict
    WFLAGS += -Wduplicated-branches
    WFLAGS += -Wduplicated-cond
    WFLAGS += -Wformat-overflow=2
    WFLAGS += -Wformat-signedness
    WFLAGS += -Wformat-truncation=2
    WFLAGS += -Wjump-misses-init
    WFLAGS += -Wlogical-op
    WFLAGS += -Wrestrict
    WFLAGS += -Wshift-overflow=2
    WFLAGS += -Wsuggest-attribute=cold
    WFLAGS += -Wsuggest-attribute=const
    WFLAGS += -Wsuggest-attribute=format
    WFLAGS += -Wsuggest-attribute=malloc
    WFLAGS += -Wsuggest-attribute=noreturn
    WFLAGS += -Wsuggest-attribute=pure
    WFLAGS += -Wsuggest-final-methods
    WFLAGS += -Wsuggest-final-types
    WFLAGS += -Wtrampolines
    WFLAGS += -Wtrampolines
    WFLAGS += -Wunsuffixed-float-constants
    ifeq ($(shell expr $(COMPILER_VERSION) \>= 13), 1)
        # GCC-specific - available in GCC 13+
        WFLAGS += -Winvalid-utf8
        WFLAGS += -Wstrict-flex-arrays
        WFLAGS += -fstrict-flex-arrays=3
        ifeq ($(shell expr $(COMPILER_VERSION) \>= 14), 1)
                # GCC-specific - available in GCC 14+
            WFLAGS += -Wuseless-cast
            WFLAGS += -Wmissing-variable-declarations
            WFLAGS += -Wsuggest-attribute=returns_nonnull
        endif
    endif
endif

# -----------------------------------------------------------------------------
# Test Framework Suppressions (for third-party code like Unity)
# -----------------------------------------------------------------------------
TEST_SUPPRESS_WFLAGS := -Wno-float-equal
TEST_SUPPRESS_WFLAGS += -Wno-padded
ifdef IS_CLANG
    TEST_SUPPRESS_WFLAGS += -Wno-covered-switch-default
    # Clang 19+ / Apple Clang warnings (silently ignored by older Clang via -Wno-unknown-warning-option)
    TEST_SUPPRESS_WFLAGS += -Wno-unknown-warning-option
    TEST_SUPPRESS_WFLAGS += -Wno-poison-system-directories
    TEST_SUPPRESS_WFLAGS += -Wno-pre-c11-compat
else
    ifeq ($(shell expr $(COMPILER_VERSION) \>= 14), 1)
        TEST_SUPPRESS_WFLAGS += -Wno-useless-cast
        TEST_SUPPRESS_WFLAGS += -Wno-suggest-attribute=returns_nonnull
    endif
endif

# -----------------------------------------------------------------------------
# Compiler Flags
# C17 is supported by GCC 8+ and Clang 6+, so safe with our minimums.
# -----------------------------------------------------------------------------
CFLAGS := -std=c17 -O3 $(WFLAGS) $(INCLUDES)

# Debug flags for sanitizer/valgrind builds
DBG_CFLAGS := -std=c17 -g -O1 -fno-omit-frame-pointer $(WFLAGS) $(INCLUDES)

# =============================================================================
# Targets
# =============================================================================

.PHONY: all test clean check sanitize valgrind format tidy help info

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Default target - build test binary
all: $(BUILD_DIR) $(TEST_BIN)

# Compile library/framework files (like unity.c) with same strict warnings
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SUPPRESS_WFLAGS) -c -o $@ $<

# Compile test suite files into object files
$(BUILD_DIR)/%_test.o: %_test.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SUPPRESS_WFLAGS) -c -o $@ $<

# Link main + test suites + library into single test binary
$(TEST_BIN): $(TEST_MAIN) $(SUITE_OBJS) $(LIB_OBJS) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SUPPRESS_WFLAGS) -o $@ $(TEST_MAIN) $(SUITE_OBJS) $(LIB_OBJS)

# Run all tests
test: $(TEST_BIN)
	@echo "=== Running Tests ==="
	$(TEST_BIN)
	@echo "=== All Tests Passed ==="

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)/*

# Static analysis with cppcheck
# Note: -D__GNUC__ tells cppcheck to analyze only the GCC code path (not MSVC or unsupported)
# Note: --library=gnu provides __builtin_bswap64 definition to avoid false MISRA 17.3 errors
# TODO: Remove MISRA 2.5 suppression when the library is ready
# TODO: Add `--force` back after the code is stable
check:
	@echo "=== Static Analysis (cppcheck) ==="
	@cppcheck \
		--addon=../.github/workflows/cppcheck/rules.json \
		--addon=threadsafety.py \
		--addon=y2038.py \
		-D__GNUC__ \
		--enable=all \
		--error-exitcode=1 \
		--inconclusive \
		--inline-suppr \
		--library=gnu \
		--std=c17 \
		--suppress=missingIncludeSystem \
		--suppress=misra-c2012-2.5 \
		--suppress=*:unity*.h \
		-I $(SRC_DIR) \
		$(ALL_H_FILES) $(PROJECT_C_FILES)
	@echo "=== Static Analysis Complete ==="

# clang-tidy static analysis
tidy:
	@echo "=== clang-tidy ==="
	@clang-tidy \
		--checks=*,-altera-struct-pack-align,-altera-unroll-loops,-llvm-header-guard \
		--fix-notes \
		--fix-errors \
		--format-style=llvm \
		--header-filter=$(SRC_DIR)/.* \
		--warnings-as-errors=* \
		-p=./build/ \
		$(PROJECT_C_FILES) \
		-- \
		-std=c17 \
		-I$(SRC_DIR)
	@echo "=== clang-tidy Complete ==="

# Format code with clang-format
format:
	@echo "=== Formatting Code ==="
	@clang-format \
		--Werror \
		--verbose \
		-i $(ALL_H_FILES) \
		$(ALL_C_FILES)
	@echo "=== Formatting Complete ==="

# Build and run with sanitizers
sanitize: $(BUILD_DIR) $(SUITE_OBJS) $(LIB_OBJS)
	@echo "=== AddressSanitizer ==="
	$(CC) -fsanitize=address $(DBG_CFLAGS) -o $(TEST_BIN_ASAN) \
		$(TEST_MAIN) $(SUITE_OBJS) $(LIB_OBJS)
	$(TEST_BIN_ASAN)
	@echo ""
	@echo "=== UndefinedBehaviorSanitizer ==="
	$(CC) -fsanitize=undefined $(DBG_CFLAGS) -o $(TEST_BIN_UBSAN) \
		$(TEST_MAIN) $(SUITE_OBJS) $(LIB_OBJS)
	$(TEST_BIN_UBSAN)
	@echo ""
	@echo "=== Sanitizer Checks Complete ==="

# Memory check with Valgrind
valgrind: $(BUILD_DIR) $(SUITE_OBJS) $(LIB_OBJS)
	@echo "=== Valgrind Memory Check ==="
	$(CC) $(DBG_CFLAGS) -o $(TEST_BIN_VALGND) \
		$(TEST_MAIN) $(SUITE_OBJS) $(LIB_OBJS)
	valgrind \
		--leak-check=full \
		--error-exitcode=1 \
		--show-leak-kinds=all \
		--track-origins=yes \
		$(TEST_BIN_VALGND)
	@echo "=== Valgrind Complete ==="

# Show build configuration
info:
	@echo "J2735 Build Configuration"
	@echo "========================="
	@echo "Compiler:    $(CC)"
	@echo "Compiler ID: $(COMPILER_ID)"
	@echo "$(COMPILER_NAME) Version: $(COMPILER_VERSION) (minimum: $(MIN_VERSION))"
	@echo "CFLAGS:      $(CFLAGS)"
	@echo ""
	@echo "Autodiscovered:"
	@echo "  Headers:     $(HEADERS)"
	@echo "  Test Main:   $(TEST_MAIN)"
	@echo "  Test Suites: $(TEST_SUITES)"
	@echo "  Library:     $(LIB_SRCS)"
	@echo "  Binary:      $(TEST_BIN)"

# Help
help:
	@echo "J2735 High-Performance Toolkit"
	@echo "==============================="
	@echo ""
	@echo "Build Targets:"
	@echo "  make           Build all test binaries (build/)"
	@echo "  make test      Build and run all tests"
	@echo "  make clean     Remove build directory"
	@echo ""
	@echo "Quality Targets:"
	@echo "  make check        Static analysis (cppcheck)"
	@echo "  make tidy         Static analysis (clang-tidy)"
	@echo "  make format       Format code (clang-format)"
	@echo "  make sanitize     Build/run with ASan + UBSan"
	@echo "  make valgrind     Memory leak check (valgrind)"
	@echo ""
	@echo "Info:"
	@echo "  make info      Show build configuration"
	@echo "  make help      This message"
	@echo ""
	@echo "Options:"
	@echo "  CC=clang       Use Clang instead of GCC"
	@echo ""
	@echo "Compiler Support: GCC 4.6+ (full: 7+), Clang 6+"
