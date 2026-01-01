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
"""
J2735 Specification Tools.

This package provides production-grade tools for parsing SAE J2735
specifications and generating zero-copy C parser code.

Modules:
    j2735_spec_parser: Parse J2735 specification text files
    j2735_c_generator: Generate C headers from parsed specifications
"""

from importlib.metadata import PackageNotFoundError, version

from .j2735_spec_parser import (
    parse_spec_file,
)

__all__ = [
    "parse_spec_file",
]

try:
    __version__ = version("j2735-tools")
except PackageNotFoundError:
    # Package not installed (running from source)
    __version__ = "0.0.0+dev"
