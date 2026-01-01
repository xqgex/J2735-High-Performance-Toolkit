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
# J2735 High-Performance Toolkit - Root Makefile
# =============================================================================
# Delegates all operations to tests/Makefile
# =============================================================================

# Override Make's built-in CC=cc default with gcc
# To use a different compiler: CC=clang make test
ifeq ($(origin CC),default)
    CC := gcc
endif

.PHONY: all test clean check sanitize valgrind format tidy help info

# Default target
all:
	@$(MAKE) -C tests CC='$(CC)' all

test:
	@$(MAKE) -C tests CC='$(CC)' test

clean:
	@$(MAKE) -C tests CC='$(CC)' clean

check:
	@$(MAKE) -C tests CC='$(CC)' check

tidy:
	@$(MAKE) -C tests CC='$(CC)' tidy

format:
	@$(MAKE) -C tests CC='$(CC)' format

sanitize:
	@$(MAKE) -C tests CC='$(CC)' sanitize

valgrind:
	@$(MAKE) -C tests CC='$(CC)' valgrind

info:
	@$(MAKE) -C tests CC='$(CC)' info

help:
	@echo "J2735 High-Performance Toolkit"
	@echo "==============================="
	@echo ""
	@echo "All targets are delegated to tests/Makefile"
	@echo ""
	@echo "Build Targets:"
	@echo "  make           Build all test binaries"
	@echo "  make test      Build and run all tests"
	@echo "  make clean     Remove build artifacts"
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
