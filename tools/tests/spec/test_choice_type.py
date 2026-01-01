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
"""Tests for ChoiceType class.

Tests cover UPER bit-width calculation for CHOICE types
per ITU-T X.691, including extensibility handling.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import ChoiceType


class TestChoiceTypeEdgeCases(TestCase):
    """Edge case tests for ChoiceType bit-width calculation."""

    def test_extensible_choice_has_none_width(self) -> None:
        """Extensible CHOICE has None bit-width (variable length)."""
        c = ChoiceType(alternatives={"a": "A", "b": "B"}, is_extensible=True)
        self.assertIsNone(c.uper_bit_width)

    def test_single_alternative_zero_bits(self) -> None:
        """Single alternative CHOICE needs 0 bits for index."""
        c = ChoiceType(alternatives={"only": "OnlyType"}, is_extensible=False)
        self.assertEqual(c.uper_bit_width, 0)

    def test_empty_alternatives_zero_bits(self) -> None:
        """Empty alternatives dict (degenerate) needs 0 bits."""
        c = ChoiceType(alternatives={}, is_extensible=False)
        self.assertEqual(c.uper_bit_width, 0)

    def test_two_alternatives_one_bit(self) -> None:
        """Two alternatives need 1 bit."""
        c = ChoiceType(alternatives={"a": "A", "b": "B"}, is_extensible=False)
        self.assertEqual(c.uper_bit_width, 1)

    def test_three_alternatives_two_bits(self) -> None:
        """Three alternatives need 2 bits (ceil(log2(3)))."""
        c = ChoiceType(alternatives={"a": "A", "b": "B", "c": "C"}, is_extensible=False)
        self.assertEqual(c.uper_bit_width, 2)

    def test_four_alternatives_two_bits(self) -> None:
        """Four alternatives need exactly 2 bits."""
        c = ChoiceType(
            alternatives={"a": "A", "b": "B", "c": "C", "d": "D"},
            is_extensible=False,
        )
        self.assertEqual(c.uper_bit_width, 2)

    def test_five_alternatives_three_bits(self) -> None:
        """Five alternatives need 3 bits."""
        c = ChoiceType(
            alternatives={"a": "A", "b": "B", "c": "C", "d": "D", "e": "E"},
            is_extensible=False,
        )
        self.assertEqual(c.uper_bit_width, 3)

    def test_alternative_count_property(self) -> None:
        """alternative_count returns correct count."""
        c = ChoiceType(alternatives={"x": "X", "y": "Y", "z": "Z"}, is_extensible=False)
        self.assertEqual(c.alternative_count, 3)
