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

.PHONY: all check clean format help info pre-push sanitize test tidy valgrind

# Default target
all:
	@$(MAKE) -C tests CC='$(CC)' all

check:
	@$(MAKE) -C tests CC='$(CC)' check

clean:
	@$(MAKE) -C tests CC='$(CC)' clean

format:
	@$(MAKE) -C tests CC='$(CC)' format

help:
	@$(MAKE) -C tests CC='$(CC)' help

info:
	@$(MAKE) -C tests CC='$(CC)' info

pre-push:
	@$(MAKE) -C tests CC='$(CC)' pre-push

sanitize:
	@$(MAKE) -C tests CC='$(CC)' sanitize

test:
	@$(MAKE) -C tests CC='$(CC)' test

tidy:
	@$(MAKE) -C tests CC='$(CC)' tidy

valgrind:
	@$(MAKE) -C tests CC='$(CC)' valgrind
