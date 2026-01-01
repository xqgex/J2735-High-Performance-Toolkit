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
"""Tests for constraint validation at construction time.

These tests verify that invalid constraint values are rejected immediately
when objects are constructed, providing fail-fast behavior for safety.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import (
    BitStringConstraint,
    IA5StringConstraint,
    IntegerConstraint,
    OctetStringConstraint,
    SequenceOfType,
    TypeReference,
)


class TestBitStringConstraintValidation(TestCase):
    """Tests for BitStringConstraint.__post_init__ validation."""

    def test_valid_size(self) -> None:
        """Valid sizes are accepted."""
        self.assertEqual(
            BitStringConstraint(
                root_size=1, is_extensible=False, extension_size=None, named_bits={"a": 0}
            ).root_size,
            1,
        )
        # For larger sizes, provide complete named_bits
        named_bits = {f"bit{i}": i for i in range(100)}
        self.assertEqual(
            BitStringConstraint(
                root_size=100, is_extensible=False, extension_size=None, named_bits=named_bits
            ).root_size,
            100,
        )

    def test_zero_size_rejected(self) -> None:
        """Size of 0 is rejected."""
        with self.assertRaises(ValueError) as ctx:
            BitStringConstraint(
                root_size=0, is_extensible=False, extension_size=None, named_bits={"a": 0}
            )
        self.assertIn("must be >= 1", str(ctx.exception))
        self.assertIn("got 0", str(ctx.exception))

    def test_negative_size_rejected(self) -> None:
        """Negative sizes are rejected."""
        with self.assertRaises(ValueError) as ctx:
            BitStringConstraint(
                root_size=-5, is_extensible=False, extension_size=None, named_bits={"a": 0}
            )
        self.assertIn("must be >= 1", str(ctx.exception))
        self.assertIn("got -5", str(ctx.exception))


class TestIA5StringConstraintValidation(TestCase):
    """Tests for IA5StringConstraint.__post_init__ validation."""

    def test_valid_sizes(self) -> None:
        """Valid size combinations are accepted."""
        c = IA5StringConstraint(min_size=0, max_size=0)
        self.assertEqual(c.min_size, 0)
        c = IA5StringConstraint(min_size=1, max_size=63)
        self.assertEqual(c.max_size, 63)

    def test_negative_min_size_rejected(self) -> None:
        """Negative min_size is rejected."""
        with self.assertRaises(ValueError) as ctx:
            IA5StringConstraint(min_size=-1, max_size=10)
        self.assertIn("min_size must be >= 0", str(ctx.exception))

    def test_max_less_than_min_rejected(self) -> None:
        """max_size < min_size is rejected."""
        with self.assertRaises(ValueError) as ctx:
            IA5StringConstraint(min_size=10, max_size=5)
        self.assertIn("max_size must be >= min_size", str(ctx.exception))
        self.assertIn("max_size=5", str(ctx.exception))
        self.assertIn("min_size=10", str(ctx.exception))


class TestIntegerConstraintValidation(TestCase):
    """Tests for IntegerConstraint.__post_init__ validation."""

    def test_valid_ranges(self) -> None:
        """Valid ranges are accepted."""
        c = IntegerConstraint(min_value=0, max_value=0)  # Single value
        self.assertEqual(c.range_size, 1)
        c = IntegerConstraint(min_value=-100, max_value=100)
        self.assertEqual(c.range_size, 201)

    def test_max_less_than_min_rejected(self) -> None:
        """max_value < min_value is rejected."""
        with self.assertRaises(ValueError) as ctx:
            IntegerConstraint(min_value=100, max_value=50)
        self.assertIn("max_value must be >= min_value", str(ctx.exception))
        self.assertIn("max_value=50", str(ctx.exception))
        self.assertIn("min_value=100", str(ctx.exception))


class TestOctetStringConstraintValidation(TestCase):
    """Tests for OctetStringConstraint.__post_init__ validation."""

    def test_valid_sizes(self) -> None:
        """Valid size combinations are accepted."""
        c = OctetStringConstraint(min_size=0, max_size=0)
        self.assertEqual(c.min_size, 0)
        c = OctetStringConstraint(min_size=1, max_size=None)  # Unbounded
        self.assertIsNone(c.max_size)

    def test_negative_min_size_rejected(self) -> None:
        """Negative min_size is rejected."""
        with self.assertRaises(ValueError) as ctx:
            OctetStringConstraint(min_size=-1, max_size=10)
        self.assertIn("min_size must be >= 0", str(ctx.exception))

    def test_max_less_than_min_rejected(self) -> None:
        """max_size < min_size is rejected (when max_size is not None)."""
        with self.assertRaises(ValueError) as ctx:
            OctetStringConstraint(min_size=10, max_size=5)
        self.assertIn("max_size must be >= min_size", str(ctx.exception))

    def test_none_max_size_always_valid(self) -> None:
        """None max_size (unbounded) is always valid."""
        c = OctetStringConstraint(min_size=100, max_size=None)
        self.assertIsNone(c.max_size)


class TestSequenceOfTypeValidation(TestCase):
    """Tests for SequenceOfType.__post_init__ validation."""

    def test_valid_sizes(self) -> None:
        """Valid size combinations are accepted."""
        element = TypeReference(name="SomeType")
        c = SequenceOfType(element_type=element, min_size=0, max_size=0)
        self.assertEqual(c.min_size, 0)
        c = SequenceOfType(element_type=element, min_size=1, max_size=10)
        self.assertEqual(c.max_size, 10)

    def test_negative_min_size_rejected(self) -> None:
        """Negative min_size is rejected."""
        element = TypeReference(name="SomeType")
        with self.assertRaises(ValueError) as ctx:
            SequenceOfType(element_type=element, min_size=-1, max_size=10)
        self.assertIn("min_size must be >= 0", str(ctx.exception))

    def test_max_less_than_min_rejected(self) -> None:
        """max_size < min_size is rejected."""
        element = TypeReference(name="SomeType")
        with self.assertRaises(ValueError) as ctx:
            SequenceOfType(element_type=element, min_size=10, max_size=5)
        self.assertIn("max_size must be >= min_size", str(ctx.exception))
