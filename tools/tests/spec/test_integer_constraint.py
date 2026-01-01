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
"""Tests for IntegerConstraint class.

Tests cover UPER bit-width calculation for constrained INTEGER types
per ITU-T X.691.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import IntegerConstraint


class TestIntegerConstraintEdgeCases(TestCase):
    """Edge case tests for IntegerConstraint."""

    def test_single_value_zero_bits(self) -> None:
        """Single value constraint needs 0 bits."""
        c = IntegerConstraint(min_value=0, max_value=0)
        self.assertEqual(c.range_size, 1)
        self.assertEqual(c.uper_bit_width, 0)

    def test_single_value_nonzero(self) -> None:
        """Single value at non-zero needs 0 bits."""
        c = IntegerConstraint(min_value=42, max_value=42)
        self.assertEqual(c.uper_bit_width, 0)

    def test_two_values_one_bit(self) -> None:
        """Two values need 1 bit."""
        c = IntegerConstraint(min_value=0, max_value=1)
        self.assertEqual(c.uper_bit_width, 1)

    def test_power_of_two_boundary(self) -> None:
        """Values at power-of-2 boundaries."""
        # 0..127 = 128 values = 2^7 = 7 bits
        self.assertEqual(IntegerConstraint(min_value=0, max_value=127).uper_bit_width, 7)
        # 0..128 = 129 values = needs 8 bits (ceil(log2(129)))
        self.assertEqual(IntegerConstraint(min_value=0, max_value=128).uper_bit_width, 8)
        # 0..255 = 256 values = 2^8 = 8 bits
        self.assertEqual(IntegerConstraint(min_value=0, max_value=255).uper_bit_width, 8)
        # 0..256 = 257 values = needs 9 bits
        self.assertEqual(IntegerConstraint(min_value=0, max_value=256).uper_bit_width, 9)

    def test_negative_range(self) -> None:
        """Negative number ranges."""
        # -1..0 = 2 values = 1 bit
        self.assertEqual(IntegerConstraint(min_value=-1, max_value=0).uper_bit_width, 1)
        # -128..127 = 256 values = 8 bits
        self.assertEqual(IntegerConstraint(min_value=-128, max_value=127).uper_bit_width, 8)

    def test_large_range(self) -> None:
        """Large ranges (typical J2735 values)."""
        # Latitude: -900000000..900000001 = 1800000002 values = 31 bits
        c = IntegerConstraint(min_value=-900000000, max_value=900000001)
        self.assertEqual(c.uper_bit_width, 31)

    def test_large_64bit_range(self) -> None:
        """64-bit range doesn't overflow."""
        # 2^63 - 1 (max signed 64-bit)
        c = IntegerConstraint(min_value=0, max_value=9223372036854775807)
        self.assertEqual(c.uper_bit_width, 63)

    def test_from_asn1_no_constraint_returns_none(self) -> None:
        """INTEGER without constraint returns None."""
        self.assertIsNone(IntegerConstraint.from_asn1("INTEGER"))
        self.assertIsNone(IntegerConstraint.from_asn1("INTEGER -- unconstrained"))

    def test_from_asn1_malformed_constraint(self) -> None:
        """Malformed constraint returns None."""
        self.assertIsNone(IntegerConstraint.from_asn1("INTEGER (abc)"))
        self.assertIsNone(IntegerConstraint.from_asn1("INTEGER ()"))

    def test_zero_range(self) -> None:
        """Range of exactly 0..0 needs 0 bits."""
        c = IntegerConstraint(min_value=0, max_value=0)
        self.assertEqual(c.uper_bit_width, 0)
