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
ASN.1 language constants shared across J2735 parsing modules.

Central repository for ASN.1 syntax tokens and patterns
used throughout the J2735 toolchain.
"""

from re import DOTALL, MULTILINE, Pattern
from re import compile as re_compile
from typing import Final

# ASN.1 syntax tokens
ASN1_COMMENT_PREFIX: Final[str] = "--"
ASN1_EXTENSION_MARKER: Final[str] = "..."
ASN1_FIELD_SEPARATOR: Final[str] = ","
ASN1_OPTIONAL_KEYWORD: Final[str] = "OPTIONAL"
ASN1_SEQUENCE_KEYWORD: Final[str] = "SEQUENCE"

# ASN.1 type definition patterns
# Note: [\w-]+ allows hyphens in type names (e.g., Offset-B10, OffsetLL-B12)
ASN1_TYPE_DEF_PATTERN: Final[Pattern[str]] = re_compile(r"^([\w-]+)\s*::=\s*(.+)$", MULTILINE)
ASN1_TYPE_DEF_DOTALL_PATTERN: Final[Pattern[str]] = re_compile(r"([\w-]+)\s*::=\s*(.+)", DOTALL)
