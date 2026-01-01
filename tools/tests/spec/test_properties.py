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
"""Property-based tests for UPER constraint calculations.

These tests use Hypothesis to verify mathematical invariants in the
constraint classes. Unlike example-based tests that check specific values,
property tests generate thousands of random inputs and verify that
invariants always hold.

This provides stronger confidence that the ITU-T X.691 formulas are
correctly implemented across the entire input domain.
"""

from math import ceil, log2
from unittest import TestCase

from hypothesis import assume, given, settings
from hypothesis import strategies as st

from tools.j2735_spec_constraints import (
    BitStringConstraint,
    BooleanType,
    EnumeratedType,
    IA5StringConstraint,
    IntegerConstraint,
    OctetStringConstraint,
    SequenceField,
    SequenceType,
    TypeReference,
)

# =============================================================================
# Strategy Definitions
# =============================================================================

# Reasonable bounds to avoid extremely slow tests or overflow
# These cover far more than J2735 needs (typically 32-bit values)
MIN_INT = -(2**31)
MAX_INT = 2**31


# =============================================================================
# IntegerConstraint Property Tests
# =============================================================================


class TestIntegerConstraintProperties(TestCase):
    """Property-based tests for IntegerConstraint."""

    @given(
        min_val=st.integers(min_value=MIN_INT, max_value=MAX_INT),
        max_val=st.integers(min_value=MIN_INT, max_value=MAX_INT),
    )
    @settings(max_examples=1000)
    def test_bit_width_matches_itu_formula(self, min_val: int, max_val: int) -> None:
        """UPER bit-width matches ITU-T X.691 formula: ceil(log2(max - min + 1)).

        Per ITU-T X.691 Section 13.2.2, a constrained whole number is encoded
        using the minimum number of bits needed to represent the range.
        """
        assume(min_val <= max_val)

        constraint = IntegerConstraint(min_value=min_val, max_value=max_val)
        range_size = max_val - min_val + 1

        # Expected bit-width per ITU-T X.691
        if range_size == 1:
            expected_bits = 0  # Single value needs no bits
        else:
            expected_bits = ceil(log2(range_size))

        self.assertEqual(
            constraint.uper_bit_width,
            expected_bits,
            f"Failed for range [{min_val}..{max_val}], range_size={range_size}",
        )

    @given(
        min_val=st.integers(min_value=MIN_INT, max_value=MAX_INT),
        max_val=st.integers(min_value=MIN_INT, max_value=MAX_INT),
    )
    def test_range_size_is_positive(self, min_val: int, max_val: int) -> None:
        """Range size is always >= 1 for valid constraints."""
        assume(min_val <= max_val)

        constraint = IntegerConstraint(min_value=min_val, max_value=max_val)
        self.assertGreaterEqual(constraint.range_size, 1)

    @given(
        min_val=st.integers(min_value=MIN_INT, max_value=MAX_INT),
        max_val=st.integers(min_value=MIN_INT, max_value=MAX_INT),
    )
    def test_bit_width_is_non_negative(self, min_val: int, max_val: int) -> None:
        """Bit-width is never negative."""
        assume(min_val <= max_val)

        constraint = IntegerConstraint(min_value=min_val, max_value=max_val)
        self.assertGreaterEqual(constraint.uper_bit_width, 0)

    @given(value=st.integers(min_value=MIN_INT, max_value=MAX_INT))
    def test_single_value_needs_zero_bits(self, value: int) -> None:
        """A single-value constraint needs 0 bits (value is known at compile time)."""
        constraint = IntegerConstraint(min_value=value, max_value=value)
        self.assertEqual(constraint.uper_bit_width, 0)
        self.assertEqual(constraint.range_size, 1)


# =============================================================================
# EnumeratedType Property Tests
# =============================================================================


class TestEnumeratedTypeProperties(TestCase):
    """Property-based tests for EnumeratedType."""

    @given(
        values=st.dictionaries(
            keys=st.text(
                alphabet=st.characters(whitelist_categories=("Ll", "Lu")),
                min_size=1,
                max_size=20,
            ),
            values=st.integers(min_value=0, max_value=1000),
            min_size=1,
            max_size=100,
        ),
        is_extensible=st.booleans(),
    )
    def test_bit_width_matches_formula(self, values: dict[str, int], is_extensible: bool) -> None:
        """UPER bit-width for enumeration index matches ceil(log2(count)).

        Per ITU-T X.691 Section 14, the enumeration index is encoded as
        a constrained whole number from 0 to (count - 1).
        """
        constraint = EnumeratedType(values=values, is_extensible=is_extensible)
        count = len(values)

        # Expected bit-width per ITU-T X.691
        if count <= 1:
            base_bits = 0
        else:
            base_bits = ceil(log2(count))

        # Extensible enumerations need at least 1 bit for extension flag
        if is_extensible:
            expected_bits = max(1, base_bits)
        else:
            expected_bits = base_bits

        self.assertEqual(
            constraint.uper_bit_width,
            expected_bits,
            f"Failed for count={count}, extensible={is_extensible}",
        )

    @given(
        values=st.dictionaries(
            keys=st.text(
                alphabet=st.characters(whitelist_categories=("Ll", "Lu")),
                min_size=1,
                max_size=10,
            ),
            values=st.integers(min_value=0, max_value=100),
            min_size=1,
            max_size=50,
        ),
    )
    def test_value_count_matches_dict_length(self, values: dict[str, int]) -> None:
        """Value count property matches dictionary length."""
        constraint = EnumeratedType(values=values, is_extensible=False)
        self.assertEqual(constraint.value_count, len(values))


# =============================================================================
# BitStringConstraint Property Tests
# =============================================================================


class TestBitStringConstraintProperties(TestCase):
    """Property-based tests for BitStringConstraint."""

    @given(root_size=st.integers(min_value=1, max_value=1000))
    def test_bit_width_equals_size_for_fixed(self, root_size: int) -> None:
        """Fixed BIT STRING bit-width equals the declared root_size."""
        # Generate complete named_bits for the given root_size
        named_bits = {f"bit{i}": i for i in range(root_size)}
        constraint = BitStringConstraint(
            root_size=root_size,
            is_extensible=False,
            extension_size=None,
            named_bits=named_bits,
        )
        self.assertEqual(constraint.uper_bit_width, root_size)

    @given(root_size=st.integers(min_value=1, max_value=1000))
    def test_bit_width_is_none_for_extensible(self, root_size: int) -> None:
        """Extensible BIT STRING bit-width is None (variable-length)."""
        # Generate complete named_bits for the given root_size
        named_bits = {f"bit{i}": i for i in range(root_size)}
        constraint = BitStringConstraint(
            root_size=root_size,
            is_extensible=True,
            extension_size=None,
            named_bits=named_bits,
        )
        self.assertIsNone(constraint.uper_bit_width)


# =============================================================================
# OctetStringConstraint Property Tests
# =============================================================================


class TestOctetStringConstraintProperties(TestCase):
    """Property-based tests for OctetStringConstraint."""

    @given(size=st.integers(min_value=0, max_value=1000))
    def test_fixed_size_bit_width_is_8x(self, size: int) -> None:
        """Fixed-size OCTET STRING bit-width is 8 * size."""
        constraint = OctetStringConstraint(min_size=size, max_size=size)
        self.assertTrue(constraint.is_fixed_size)
        self.assertEqual(constraint.uper_bit_width, size * 8)

    @given(
        min_size=st.integers(min_value=0, max_value=500),
        max_size=st.integers(min_value=0, max_value=500),
    )
    def test_variable_size_has_none_width(self, min_size: int, max_size: int) -> None:
        """Variable-size OCTET STRING has None bit-width."""
        assume(min_size < max_size)  # Must be different for variable size
        constraint = OctetStringConstraint(min_size=min_size, max_size=max_size)
        self.assertFalse(constraint.is_fixed_size)
        self.assertIsNone(constraint.uper_bit_width)


# =============================================================================
# IA5StringConstraint Property Tests
# =============================================================================


class TestIA5StringConstraintProperties(TestCase):
    """Property-based tests for IA5StringConstraint."""

    @given(size=st.integers(min_value=0, max_value=1000))
    def test_fixed_size_bit_width_is_7x(self, size: int) -> None:
        """Fixed-size IA5String bit-width is 7 * size (7 bits per ASCII char)."""
        constraint = IA5StringConstraint(min_size=size, max_size=size)
        self.assertTrue(constraint.is_fixed_size)
        self.assertEqual(constraint.uper_bit_width, size * 7)

    @given(
        min_size=st.integers(min_value=0, max_value=500),
        max_size=st.integers(min_value=0, max_value=500),
    )
    def test_variable_size_has_none_width(self, min_size: int, max_size: int) -> None:
        """Variable-size IA5String has None bit-width."""
        assume(min_size < max_size)
        constraint = IA5StringConstraint(min_size=min_size, max_size=max_size)
        self.assertFalse(constraint.is_fixed_size)
        self.assertIsNone(constraint.uper_bit_width)


# =============================================================================
# BooleanType Property Tests
# =============================================================================


class TestBooleanTypeProperties(TestCase):
    """Property-based tests for BooleanType."""

    def test_boolean_always_one_bit(self) -> None:
        """BOOLEAN is always encoded as 1 bit."""
        constraint = BooleanType()
        self.assertEqual(constraint.uper_bit_width, 1)


# =============================================================================
# SequenceType Property Tests
# =============================================================================


class TestSequenceTypeProperties(TestCase):
    """Property-based tests for SequenceType."""

    @given(
        widths=st.lists(
            st.integers(min_value=1, max_value=64),
            min_size=1,
            max_size=20,
        )
    )
    def test_sequence_width_is_sum_of_fields(self, widths: list[int]) -> None:
        """SEQUENCE bit-width is sum of all field bit-widths."""
        fields = tuple(
            SequenceField(
                name=f"field{i}",
                type_name=f"Type{i}",
                type=IntegerConstraint(min_value=0, max_value=(2**w) - 1),
                is_optional=False,
                section_comment="",
                inline_comment="",
            )
            for i, w in enumerate(widths)
        )
        constraint = SequenceType(fields=fields, is_extensible=False)

        # Each field's bit-width should be w (since range is 0..2^w-1 = 2^w values = w bits)
        expected_total = sum(widths)
        self.assertEqual(constraint.uper_bit_width, expected_total)

    @given(
        widths=st.lists(
            st.integers(min_value=1, max_value=32),
            min_size=1,
            max_size=10,
        ),
        optional_index=st.integers(min_value=0, max_value=9),
    )
    def test_optional_field_means_none_width(self, widths: list[int], optional_index: int) -> None:
        """SEQUENCE with any OPTIONAL field has None bit-width."""
        assume(optional_index < len(widths))

        fields = tuple(
            SequenceField(
                name=f"field{i}",
                type_name=f"Type{i}",
                type=IntegerConstraint(min_value=0, max_value=(2**w) - 1),
                is_optional=(i == optional_index),
                section_comment="",
                inline_comment="",
            )
            for i, w in enumerate(widths)
        )
        constraint = SequenceType(fields=fields, is_extensible=False)

        # Any optional field means we can't determine fixed bit-width
        self.assertIsNone(constraint.uper_bit_width)

    @given(
        widths=st.lists(
            st.integers(min_value=1, max_value=32),
            min_size=1,
            max_size=10,
        ),
    )
    def test_extensible_sequence_has_none_width(self, widths: list[int]) -> None:
        """Extensible SEQUENCE always has None bit-width."""
        fields = tuple(
            SequenceField(
                name=f"field{i}",
                type_name=f"Type{i}",
                type=IntegerConstraint(min_value=0, max_value=(2**w) - 1),
                is_optional=False,
                section_comment="",
                inline_comment="",
            )
            for i, w in enumerate(widths)
        )
        constraint = SequenceType(fields=fields, is_extensible=True)
        self.assertIsNone(constraint.uper_bit_width)

    @given(
        optional_mask=st.lists(
            st.booleans(),
            min_size=1,
            max_size=20,
        )
    )
    def test_optional_count_matches_mask(self, optional_mask: list[bool]) -> None:
        """optional_count equals number of True values in optional flags."""
        fields = tuple(
            SequenceField(
                name=f"field{i}",
                type_name=f"Type{i}",
                type=IntegerConstraint(min_value=0, max_value=7),
                is_optional=is_opt,
                section_comment="",
                inline_comment="",
            )
            for i, is_opt in enumerate(optional_mask)
        )
        constraint = SequenceType(fields=fields, is_extensible=False)
        self.assertEqual(constraint.optional_count, sum(optional_mask))

    def test_extension_bit_zero_when_not_extensible(self) -> None:
        """extension_bit is 0 for non-extensible SEQUENCE."""
        constraint = SequenceType(fields=(), is_extensible=False)
        self.assertEqual(constraint.extension_bit, 0)

    def test_extension_bit_one_when_extensible(self) -> None:
        """extension_bit is 1 for extensible SEQUENCE."""
        constraint = SequenceType(fields=(), is_extensible=True)
        self.assertEqual(constraint.extension_bit, 1)

    @given(
        optional_count=st.integers(min_value=0, max_value=10),
        is_extensible=st.booleans(),
    )
    def test_preamble_bits_formula(self, optional_count: int, is_extensible: bool) -> None:
        """preamble_bits = extension_bit + optional_count."""
        fields = tuple(
            SequenceField(
                name=f"field{i}",
                type_name=f"Type{i}",
                type=IntegerConstraint(min_value=0, max_value=7),
                is_optional=True,
                section_comment="",
                inline_comment="",
            )
            for i in range(optional_count)
        )
        constraint = SequenceType(fields=fields, is_extensible=is_extensible)
        expected = (1 if is_extensible else 0) + optional_count
        self.assertEqual(constraint.preamble_bits, expected)

    def test_nested_unresolved_reference_none_width(self) -> None:
        """SEQUENCE with unresolved TypeReference has None bit-width."""
        fields = (
            SequenceField(
                name="unresolved",
                type_name="UnknownType",
                type=TypeReference(name="UnknownType"),
                is_optional=False,
                section_comment="",
                inline_comment="",
            ),
        )
        seq = SequenceType(fields=fields, is_extensible=False)
        self.assertIsNone(seq.uper_bit_width)

    def test_empty_sequence_zero_width(self) -> None:
        """Empty SEQUENCE has 0 bit-width."""
        seq = SequenceType(fields=(), is_extensible=False)
        self.assertEqual(seq.uper_bit_width, 0)

    def test_empty_extensible_sequence_none_width(self) -> None:
        """Empty extensible SEQUENCE has None bit-width."""
        seq = SequenceType(fields=(), is_extensible=True)
        self.assertIsNone(seq.uper_bit_width)

    def test_preamble_bits_many_optionals(self) -> None:
        """Preamble with many OPTIONAL fields."""
        fields = tuple(
            SequenceField(
                name=f"opt{i}",
                type_name=f"Type{i}",
                type=IntegerConstraint(min_value=0, max_value=7),
                is_optional=True,
                section_comment="",
                inline_comment="",
            )
            for i in range(100)
        )
        seq = SequenceType(fields=fields, is_extensible=False)
        self.assertEqual(seq.optional_count, 100)
        self.assertEqual(seq.preamble_bits, 100)


# =============================================================================
# TypeReference Property Tests
# =============================================================================


class TestTypeReferenceProperties(TestCase):
    """Property-based tests for TypeReference."""

    @given(
        name=st.text(
            alphabet=st.characters(whitelist_categories=("Ll", "Lu")),
            min_size=1,
            max_size=50,
        )
    )
    def test_unresolved_reference_has_none_width(self, name: str) -> None:
        """Unresolved TypeReference always has None bit-width."""
        ref = TypeReference(name=name)
        self.assertIsNone(ref.uper_bit_width)
