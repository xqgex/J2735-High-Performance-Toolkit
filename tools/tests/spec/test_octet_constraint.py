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
"""Tests for OctetStringConstraint class.

Tests cover UPER bit-width calculation for OCTET STRING types
per ITU-T X.691.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import OctetStringConstraint


class TestOctetStringConstraintEdgeCases(TestCase):
    """Edge case tests for OctetStringConstraint."""

    def test_fixed_single_byte(self) -> None:
        """Single fixed byte."""
        c = OctetStringConstraint(min_size=1, max_size=1)
        self.assertTrue(c.is_fixed_size)
        self.assertEqual(c.uper_bit_width, 8)

    def test_fixed_four_bytes(self) -> None:
        """TemporaryID is 4 bytes = 32 bits."""
        c = OctetStringConstraint(min_size=4, max_size=4)
        self.assertTrue(c.is_fixed_size)
        self.assertEqual(c.uper_bit_width, 32)

    def test_variable_size_no_fixed_width(self) -> None:
        """Variable size has no fixed bit-width."""
        c = OctetStringConstraint(min_size=1, max_size=8)
        self.assertFalse(c.is_fixed_size)
        self.assertIsNone(c.uper_bit_width)

    def test_large_size_no_overflow(self) -> None:
        """Large SIZE doesn't overflow bit-width calculation."""
        c = OctetStringConstraint(min_size=1000000, max_size=1000000)
        # 8 bits per byte * 1,000,000 = 8,000,000 bits
        self.assertEqual(c.uper_bit_width, 8000000)
