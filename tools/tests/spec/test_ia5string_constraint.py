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
"""Tests for IA5StringConstraint class.

Tests cover UPER bit-width calculation for IA5String types
per ITU-T X.691.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import IA5StringConstraint


class TestIA5StringConstraintEdgeCases(TestCase):
    """Edge case tests for IA5StringConstraint."""

    def test_fixed_size_single_char(self) -> None:
        """Single fixed character is 7 bits."""
        c = IA5StringConstraint(min_size=1, max_size=1)
        self.assertTrue(c.is_fixed_size)
        self.assertEqual(c.uper_bit_width, 7)

    def test_fixed_size_multiple_chars(self) -> None:
        """Multiple fixed characters are 7 bits each."""
        c = IA5StringConstraint(min_size=10, max_size=10)
        self.assertTrue(c.is_fixed_size)
        self.assertEqual(c.uper_bit_width, 70)

    def test_variable_size_no_fixed_width(self) -> None:
        """Variable size has no fixed bit-width."""
        c = IA5StringConstraint(min_size=1, max_size=100)
        self.assertFalse(c.is_fixed_size)
        self.assertIsNone(c.uper_bit_width)

    def test_large_size_no_overflow(self) -> None:
        """Large SIZE doesn't overflow bit-width calculation."""
        c = IA5StringConstraint(min_size=1000000, max_size=1000000)
        # 7 bits per char * 1,000,000 = 7,000,000 bits
        self.assertEqual(c.uper_bit_width, 7000000)
