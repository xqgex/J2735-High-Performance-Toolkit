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
"""Tests for EnumeratedType class.

Tests cover UPER bit-width calculation for ENUMERATED types
per ITU-T X.691, including extensibility handling.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import EnumeratedType


class TestEnumeratedTypeEdgeCases(TestCase):
    """Edge case tests for EnumeratedType."""

    def test_single_value_enum(self) -> None:
        """Single value enumeration."""
        e = EnumeratedType(values={"only": 0}, is_extensible=False)
        self.assertEqual(e.value_count, 1)
        self.assertEqual(e.uper_bit_width, 0)  # Only one choice

    def test_two_value_enum(self) -> None:
        """Two values need 1 bit."""
        e = EnumeratedType(values={"off": 0, "on": 1}, is_extensible=False)
        self.assertEqual(e.uper_bit_width, 1)

    def test_extensible_adds_bit(self) -> None:
        """Extensibility marker ensures at least 1 bit."""
        e_single = EnumeratedType(values={"a": 0}, is_extensible=True)
        # Single value extensible still needs 1 bit (for extension flag)
        self.assertEqual(e_single.uper_bit_width, 1)
        # Non-extensible single value needs 0 bits
        e_single_normal = EnumeratedType(values={"a": 0}, is_extensible=False)
        self.assertEqual(e_single_normal.uper_bit_width, 0)


class TestEnumeratedParsing(TestCase):
    """Tests for EnumeratedType.from_asn1() parsing."""

    def test_parse_explicit_values(self) -> None:
        """Parse ENUMERATED with explicit value assignments."""
        result = EnumeratedType.from_asn1("ENUMERATED { off (0), on (1), auto (2) }")
        self.assertIsNotNone(result)
        assert result is not None
        self.assertEqual(result.values, {"off": 0, "on": 1, "auto": 2})
        self.assertFalse(result.is_extensible)

    def test_parse_extensible(self) -> None:
        """Parse extensible ENUMERATED."""
        result = EnumeratedType.from_asn1("ENUMERATED { a (0), b (1), ... }")
        self.assertIsNotNone(result)
        assert result is not None
        self.assertTrue(result.is_extensible)


class TestEnumeratedImplicitValues(TestCase):
    """Tests for ENUMERATED with implicit (auto-assigned) values.

    Bug: ASN.1 allows ENUMERATED without explicit value assignments.
    Values are implicitly assigned starting from 0.

    Example from J2735:
        AsphaltOrTarType ::= ENUMERATED { newSharp, traveled, trafficPolished, excessTar, ... }

    This should be equivalent to:
        { newSharp (0), traveled (1), trafficPolished (2), excessTar (3), ... }
    """

    def test_implicit_values_simple(self) -> None:
        """Parse ENUMERATED with implicit sequential values."""
        result = EnumeratedType.from_asn1(
            "ENUMERATED { newSharp, traveled, trafficPolished, excessTar, ... }"
        )
        # Bug: Currently returns None because values like "newSharp" have no (n)
        self.assertIsNotNone(result, "Should parse ENUMERATED with implicit values")
        assert result is not None
        self.assertEqual(result.values["newSharp"], 0)
        self.assertEqual(result.values["traveled"], 1)
        self.assertEqual(result.values["trafficPolished"], 2)
        self.assertEqual(result.values["excessTar"], 3)
        self.assertTrue(result.is_extensible)

    def test_implicit_values_non_extensible(self) -> None:
        """Parse non-extensible ENUMERATED with implicit values."""
        result = EnumeratedType.from_asn1("ENUMERATED { low, medium, high }")
        self.assertIsNotNone(result, "Should parse ENUMERATED with implicit values")
        assert result is not None
        self.assertEqual(result.values, {"low": 0, "medium": 1, "high": 2})
        self.assertFalse(result.is_extensible)

    def test_mixed_explicit_implicit_values(self) -> None:
        """Parse ENUMERATED with mixed explicit and implicit values."""
        # ASN.1 allows: implicit values continue from last explicit
        result = EnumeratedType.from_asn1("ENUMERATED { a (0), b, c (5), d }")
        self.assertIsNotNone(result, "Should parse mixed ENUMERATED")
        assert result is not None
        # 'a' is explicit 0, 'b' implicit 1, 'c' explicit 5, 'd' implicit 6
        self.assertEqual(result.values["a"], 0)
        self.assertEqual(result.values["b"], 1)
        self.assertEqual(result.values["c"], 5)
        self.assertEqual(result.values["d"], 6)
