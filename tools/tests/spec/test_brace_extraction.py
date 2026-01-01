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
"""Tests for _extract_outermost_braces utility function.

This function is critical for SEQUENCE/CHOICE parsing in safety-critical
V2X environments. Edge cases must be thoroughly tested.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import (
    _extract_outermost_braces,  # pyright: ignore[reportPrivateUsage]
)


class TestExtractOutermostBraces(TestCase):
    """Tests for brace extraction - critical for ASN.1 parsing."""

    def test_no_braces_returns_none(self) -> None:
        """Input without braces returns None."""
        self.assertIsNone(_extract_outermost_braces("no braces here"))

    def test_empty_string_returns_none(self) -> None:
        """Empty string returns None."""
        self.assertIsNone(_extract_outermost_braces(""))

    def test_unbalanced_open_brace_returns_none(self) -> None:
        """Unclosed brace returns None (no match)."""
        self.assertIsNone(_extract_outermost_braces("SEQUENCE { a A, b B"))

    def test_unbalanced_multiple_open_braces(self) -> None:
        """Multiple unclosed braces returns None."""
        self.assertIsNone(_extract_outermost_braces("{ { { never closes"))

    def test_closing_before_opening(self) -> None:
        """Closing brace before opening - finds first valid pair."""
        result = _extract_outermost_braces("} { a }")
        self.assertEqual(result, " a ")

    def test_empty_braces(self) -> None:
        """Empty braces return empty string."""
        self.assertEqual(_extract_outermost_braces("{}"), "")

    def test_nested_braces_preserved(self) -> None:
        """Nested braces are preserved in output."""
        result = _extract_outermost_braces("SEQUENCE { a {{X}}, b {{Y}} }")
        self.assertEqual(result, " a {{X}}, b {{Y}} ")

    def test_deeply_nested_braces(self) -> None:
        """Deeply nested braces (5 levels) handled correctly."""
        result = _extract_outermost_braces("{{{{{deep}}}}}extra")
        self.assertEqual(result, "{{{{deep}}}}")

    def test_only_opening_brace(self) -> None:
        """Single opening brace with no close returns None."""
        self.assertIsNone(_extract_outermost_braces("{"))

    def test_brace_in_middle_of_text(self) -> None:
        """Braces not at start are still found."""
        result = _extract_outermost_braces("PREFIX { content } SUFFIX")
        self.assertEqual(result, " content ")
