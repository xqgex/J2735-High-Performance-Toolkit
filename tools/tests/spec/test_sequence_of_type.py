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
"""Tests for SequenceOfType class.

Tests cover UPER encoding behavior for SEQUENCE OF types
per ITU-T X.691.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import IntegerConstraint, SequenceOfType


class TestSequenceOfTypeEdgeCases(TestCase):
    """Edge case tests for SequenceOfType."""

    def test_uper_bit_width_always_none(self) -> None:
        """SEQUENCE OF always has None bit-width (variable length)."""
        s = SequenceOfType(
            element_type=IntegerConstraint(min_value=0, max_value=255),
            min_size=1,
            max_size=10,
        )
        self.assertIsNone(s.uper_bit_width)

    def test_fixed_size_still_none_width(self) -> None:
        """Even fixed-size SEQUENCE OF has None bit-width."""
        s = SequenceOfType(
            element_type=IntegerConstraint(min_value=0, max_value=7),
            min_size=5,
            max_size=5,
        )
        # SEQUENCE OF with variable-length elements has None width
        self.assertIsNone(s.uper_bit_width)

    def test_element_type_preserved(self) -> None:
        """element_type is preserved in dataclass."""
        elem = IntegerConstraint(min_value=-128, max_value=127)
        s = SequenceOfType(element_type=elem, min_size=0, max_size=100)
        self.assertEqual(s.element_type, elem)
        self.assertEqual(s.min_size, 0)
        self.assertEqual(s.max_size, 100)
