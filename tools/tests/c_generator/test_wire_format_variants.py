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
"""Tests for wire format variant generation.

Tests cover:
    - _pluralize_bits(): singular/plural grammar for bit counts
    - get_sequence_variants(): all 4 SEQUENCE cases + edge cases
    - SequenceWireVariant: dataclass field correctness
"""

from unittest import TestCase

from tools.j2735_c_generator_wire_format import (
    SequenceWireVariant,
    _pluralize_bits,  # pyright: ignore[reportPrivateUsage]
    get_sequence_variants,
)
from tools.j2735_spec_constraints import SequenceType
from tools.j2735_spec_parser import J2735Specification
from tools.tests.conftest import (
    get_sequence_typedef,
    make_bitstring_field,
    make_extensible_mock_spec,
    make_integer_field,
    make_nested_mock_spec,
    make_optional_mock_spec,
    make_sequence,
)


def _get_variants(type_name: str, spec: J2735Specification) -> list[SequenceWireVariant]:
    """Look up a SEQUENCE type and return its wire format variants.

    Args:
        type_name: Name of the SEQUENCE type.
        spec: The parsed J2735 specification.

    Returns:
        List of SequenceWireVariant objects for the type.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.
    """
    typedef = get_sequence_typedef(type_name, spec)
    assert isinstance(typedef.constraint, SequenceType)
    return get_sequence_variants(typedef.constraint)


# =============================================================================
# Tests — _pluralize_bits()
# =============================================================================


class TestPluralizeBits(TestCase):
    """Tests for _pluralize_bits() singular/plural grammar."""

    def test_zero_bits_plural(self) -> None:
        """Zero uses plural form."""
        self.assertEqual(_pluralize_bits(0), "0 bits")

    def test_one_bit_singular(self) -> None:
        """One uses singular form."""
        self.assertEqual(_pluralize_bits(1), "1 bit")

    def test_two_bits_plural(self) -> None:
        """Two uses plural form."""
        self.assertEqual(_pluralize_bits(2), "2 bits")

    def test_large_number_plural(self) -> None:
        """Large numbers use plural form."""
        self.assertEqual(_pluralize_bits(290), "290 bits")


# =============================================================================
# Tests — get_sequence_variants()
# =============================================================================


class TestFixedSequence(TestCase):
    """Case 1: Fixed SEQUENCE (no OPTIONAL, not extensible)."""

    def test_single_variant_returned(self) -> None:
        """Fixed SEQUENCE produces exactly one variant."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "TypeA", 0, 255),  # 8 bits
                make_integer_field("b", "TypeB", 0, 15),  # 4 bits
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(len(variants), 1)

    def test_total_bits_is_sum_of_fields(self) -> None:
        """Total bits equals sum of all field bit-widths."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "TypeA", 0, 255),  # 8 bits
                make_integer_field("b", "TypeB", 0, 15),  # 4 bits
                make_integer_field("c", "TypeC", 0, 1),  # 1 bit
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].total_bits, 13)

    def test_no_extension_bit(self) -> None:
        """Fixed SEQUENCE has no extension bit."""
        seq = make_sequence(fields=(make_integer_field("a", "TypeA", 0, 127),))
        variants = get_sequence_variants(seq)

        self.assertIsNone(variants[0].ext_bit)

    def test_no_optional_bitmap(self) -> None:
        """Fixed SEQUENCE has no optional bitmap."""
        seq = make_sequence(fields=(make_integer_field("a", "TypeA", 0, 127),))
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].opt_bitmap, "")

    def test_all_fields_included(self) -> None:
        """Variant includes all original fields."""
        fields = (
            make_integer_field("a", "TypeA", 0, 127),
            make_integer_field("b", "TypeB", 0, 127),
            make_integer_field("c", "TypeC", 0, 127),
        )
        seq = make_sequence(fields=fields)
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].fields, fields)

    def test_name_includes_bit_count(self) -> None:
        """Variant name contains the total bit count."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "TypeA", 0, 255),  # 8 bits
                make_integer_field("b", "TypeB", 0, 255),  # 8 bits
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].name, "16 bits")

    def test_single_field(self) -> None:
        """Single-field SEQUENCE works correctly."""
        seq = make_sequence(fields=(make_integer_field("x", "TypeX", 0, 1),))  # 1 bit
        variants = get_sequence_variants(seq)

        self.assertEqual(len(variants), 1)
        self.assertEqual(variants[0].total_bits, 1)
        self.assertEqual(variants[0].name, "1 bit")


class TestExtensibleSequence(TestCase):
    """Case 2: Extensible SEQUENCE with no OPTIONAL fields."""

    def test_two_variants_returned(self) -> None:
        """Extensible SEQUENCE produces exactly two variants."""
        seq = make_sequence(
            fields=(make_integer_field("a", "TypeA", 0, 255),),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(len(variants), 2)

    def test_first_variant_no_extensions(self) -> None:
        """First variant is 'no extensions' with ext_bit=0."""
        seq = make_sequence(
            fields=(make_integer_field("a", "TypeA", 0, 255),),  # 8 bits
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].ext_bit, 0)
        self.assertIn("no extensions", variants[0].name)

    def test_first_variant_total_bits_includes_ext_bit(self) -> None:
        """First variant total_bits = 1 (ext bit) + field bits."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "TypeA", 0, 255),  # 8 bits
                make_integer_field("b", "TypeB", 0, 15),  # 4 bits
            ),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        # 1 ext bit + 8 + 4 = 13
        self.assertEqual(variants[0].total_bits, 13)

    def test_second_variant_with_extensions(self) -> None:
        """Second variant is 'with extensions' with ext_bit=1."""
        seq = make_sequence(
            fields=(make_integer_field("a", "TypeA", 0, 255),),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[1].ext_bit, 1)
        self.assertIn("with extensions", variants[1].name)

    def test_second_variant_variable_bits(self) -> None:
        """Second variant has total_bits='variable'."""
        seq = make_sequence(
            fields=(make_integer_field("a", "TypeA", 0, 255),),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[1].total_bits, "variable")

    def test_both_variants_have_all_fields(self) -> None:
        """Both variants contain all fields."""
        fields = (
            make_integer_field("a", "TypeA", 0, 127),
            make_integer_field("b", "TypeB", 0, 127),
        )
        seq = make_sequence(fields=fields, is_extensible=True)
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].fields, fields)
        self.assertEqual(variants[1].fields, fields)

    def test_no_optional_bitmap(self) -> None:
        """Extensible SEQUENCE without OPTIONAL has empty opt_bitmap."""
        seq = make_sequence(
            fields=(make_integer_field("a", "TypeA", 0, 127),),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].opt_bitmap, "")
        self.assertEqual(variants[1].opt_bitmap, "")


class TestOptionalSequence(TestCase):
    """Case 3: SEQUENCE with OPTIONAL fields (not extensible)."""

    def test_two_variants_returned(self) -> None:
        """OPTIONAL SEQUENCE produces exactly two variants."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "TypeA", 0, 127, is_optional=True),
                make_integer_field("b", "TypeB", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(len(variants), 2)

    def test_absent_variant_excludes_optional(self) -> None:
        """ABSENT variant contains only required fields."""
        opt = make_integer_field("opt", "OptType", 0, 127, is_optional=True)
        req = make_integer_field("req", "ReqType", 0, 127)
        seq = make_sequence(fields=(opt, req))
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].fields, (req,))

    def test_present_variant_includes_all(self) -> None:
        """PRESENT variant contains all fields."""
        opt = make_integer_field("opt", "OptType", 0, 127, is_optional=True)
        req = make_integer_field("req", "ReqType", 0, 127)
        seq = make_sequence(fields=(opt, req))
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[1].fields, (opt, req))

    def test_absent_bitmap_single_optional(self) -> None:
        """Single OPTIONAL: bitmap is "0" for absent."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 127, is_optional=True),
                make_integer_field("req", "ReqType", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].opt_bitmap, "0")

    def test_present_bitmap_single_optional(self) -> None:
        """Single OPTIONAL: bitmap is "1" for present."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 127, is_optional=True),
                make_integer_field("req", "ReqType", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[1].opt_bitmap, "1")

    def test_absent_bitmap_multiple_optionals(self) -> None:
        """Multiple OPTIONALs: bitmap is "0..0 (N)" for absent."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt1", "OptType1", 0, 127, is_optional=True),
                make_integer_field("opt2", "OptType2", 0, 127, is_optional=True),
                make_integer_field("opt3", "OptType3", 0, 127, is_optional=True),
                make_integer_field("req", "ReqType", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].opt_bitmap, "0..0 (3)")

    def test_present_bitmap_multiple_optionals(self) -> None:
        """Multiple OPTIONALs: bitmap is "1..1 (N)" for present."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt1", "OptType1", 0, 127, is_optional=True),
                make_integer_field("opt2", "OptType2", 0, 127, is_optional=True),
                make_integer_field("req", "ReqType", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[1].opt_bitmap, "1..1 (2)")

    def test_absent_total_bits(self) -> None:
        """ABSENT variant: total = opt_count + sum(required field bits)."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 255, is_optional=True),  # 8 bits
                make_integer_field("req", "ReqType", 0, 15),  # 4 bits
            ),
        )
        variants = get_sequence_variants(seq)

        # 1 opt bit + 4 required bits = 5
        self.assertEqual(variants[0].total_bits, 5)

    def test_present_total_bits(self) -> None:
        """PRESENT variant: total = opt_count + sum(all field bits)."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 255, is_optional=True),  # 8 bits
                make_integer_field("req", "ReqType", 0, 15),  # 4 bits
            ),
        )
        variants = get_sequence_variants(seq)

        # 1 opt bit + 8 + 4 = 13
        self.assertEqual(variants[1].total_bits, 13)

    def test_no_ext_bit(self) -> None:
        """Non-extensible OPTIONAL SEQUENCE has ext_bit=None."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 127, is_optional=True),
                make_integer_field("req", "ReqType", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertIsNone(variants[0].ext_bit)
        self.assertIsNone(variants[1].ext_bit)

    def test_single_optional_name_uses_field_name(self) -> None:
        """Single OPTIONAL: variant name uses the field name, not 'all optional'."""
        seq = make_sequence(
            fields=(
                make_integer_field("region", "RoadRegulatorID", 0, 127, is_optional=True),
                make_integer_field("id", "IntersectionID", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertIn("region ABSENT", variants[0].name)
        self.assertIn("region PRESENT", variants[1].name)

    def test_multiple_optional_name_uses_all_optional(self) -> None:
        """Multiple OPTIONALs: variant name says 'all optional ABSENT/PRESENT'."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt1", "TypeA", 0, 127, is_optional=True),
                make_integer_field("opt2", "TypeB", 0, 127, is_optional=True),
                make_integer_field("req", "TypeC", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertIn("all optional ABSENT", variants[0].name)
        self.assertIn("all optional PRESENT", variants[1].name)


class TestOptionalExtensibleSequence(TestCase):
    """Case 4: SEQUENCE with OPTIONAL + extensible."""

    def test_two_variants_returned(self) -> None:
        """Extensible + OPTIONAL produces exactly two variants."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 127, is_optional=True),
                make_integer_field("req", "ReqType", 0, 127),
            ),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(len(variants), 2)

    def test_ext_bit_is_zero(self) -> None:
        """Both variants have ext_bit=0 (Case 3 treats as optional-dominant)."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 127, is_optional=True),
                make_integer_field("req", "ReqType", 0, 127),
            ),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].ext_bit, 0)
        self.assertEqual(variants[1].ext_bit, 0)

    def test_total_bits_includes_ext_bit(self) -> None:
        """Total bits includes the extension bit prefix."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 255, is_optional=True),  # 8 bits
                make_integer_field("req", "ReqType", 0, 15),  # 4 bits
            ),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        # ABSENT: 1 ext + 1 opt + 4 req = 6
        self.assertEqual(variants[0].total_bits, 6)
        # PRESENT: 1 ext + 1 opt + 8 + 4 = 14
        self.assertEqual(variants[1].total_bits, 14)


class TestBitWidthAccumulation(TestCase):
    """Regression tests for total_bits computation accuracy."""

    def test_many_small_fields(self) -> None:
        """Sum of many 1-bit fields is correct."""
        fields = tuple(make_integer_field(f"f{i}", f"T{i}", 0, 1) for i in range(10))
        seq = make_sequence(fields=fields)
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].total_bits, 10)

    def test_mixed_widths(self) -> None:
        """Mixed field widths sum correctly."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "A", 0, 1),  # 1 bit
                make_integer_field("b", "B", 0, 127),  # 7 bits
                make_integer_field("c", "C", 0, 255),  # 8 bits
                make_integer_field("d", "D", 0, 65535),  # 16 bits
                make_integer_field("e", "E", -32767, 32767),  # 16 bits
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].total_bits, 48)

    def test_signed_fields_dont_affect_bit_width(self) -> None:
        """Signed vs unsigned with same range size → same bit width."""
        seq_unsigned = make_sequence(
            fields=(make_integer_field("u", "U", 0, 65534),),  # 16 bits
        )
        seq_signed = make_sequence(
            fields=(make_integer_field("s", "S", -32767, 32767),),  # 16 bits
        )

        variants_u = get_sequence_variants(seq_unsigned)
        variants_s = get_sequence_variants(seq_signed)

        self.assertEqual(variants_u[0].total_bits, variants_s[0].total_bits)

    def test_bitstring_field_width(self) -> None:
        """BitStringConstraint field width is included correctly."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "TypeA", 0, 255),  # 8 bits
                make_bitstring_field("flags", "Flags", 13),  # 13 bits
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].total_bits, 21)


class TestSequenceWireVariantImmutability(TestCase):
    """SequenceWireVariant is a frozen dataclass — verify immutability."""

    def test_frozen(self) -> None:
        """SequenceWireVariant raises on attribute assignment."""
        variant = SequenceWireVariant(
            name="test",
            fields=(),
            ext_bit=None,
            opt_bitmap="",
            total_bits=0,
        )
        with self.assertRaises(AttributeError):
            variant.name = "changed"  # type: ignore[misc]

    def test_fields_is_tuple(self) -> None:
        """Variant fields are returned as a tuple (not list)."""
        seq = make_sequence(fields=(make_integer_field("a", "TypeA", 0, 127),))
        variants = get_sequence_variants(seq)

        self.assertIsInstance(variants[0].fields, tuple)


# =============================================================================
# Tests — Regression: Variants Match SequenceType Properties
# =============================================================================


class TestVariantsMatchSequenceType(TestCase):
    """Cross-check variant data against SequenceType properties."""

    def test_fixed_variant_bits_matches_uper_bit_width(self) -> None:
        """Fixed SEQUENCE: variant total_bits matches SequenceType.uper_bit_width."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "TypeA", 0, 255),  # 8
                make_integer_field("b", "TypeB", 0, 65535),  # 16
            ),
        )
        variants = get_sequence_variants(seq)

        self.assertEqual(variants[0].total_bits, seq.uper_bit_width)

    def test_extensible_variant_bits_matches_root_uper_bit_width(self) -> None:
        """Extensible SEQUENCE: 'no ext' variant matches root_uper_bit_width."""
        seq = make_sequence(
            fields=(
                make_integer_field("a", "TypeA", 0, 255),  # 8
                make_integer_field("b", "TypeB", 0, 15),  # 4
            ),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)

        # root_uper_bit_width = preamble_bits(1) + 8 + 4 = 13
        self.assertEqual(variants[0].total_bits, seq.root_uper_bit_width)

    def test_optional_present_matches_preamble_plus_all_fields(self) -> None:
        """OPTIONAL PRESENT variant total = preamble_bits + sum(all fields)."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 255, is_optional=True),  # 8
                make_integer_field("req", "ReqType", 0, 15),  # 4
            ),
        )
        variants = get_sequence_variants(seq)

        expected = seq.preamble_bits + sum(f.type.uper_bit_width or 0 for f in seq.fields)
        self.assertEqual(variants[1].total_bits, expected)

    def test_optional_absent_matches_preamble_plus_required_fields(self) -> None:
        """OPTIONAL ABSENT variant total = preamble_bits + sum(required fields)."""
        seq = make_sequence(
            fields=(
                make_integer_field("opt", "OptType", 0, 255, is_optional=True),  # 8
                make_integer_field("req", "ReqType", 0, 15),  # 4
            ),
        )
        variants = get_sequence_variants(seq)

        expected = seq.preamble_bits + sum(
            f.type.uper_bit_width or 0 for f in seq.fields if not f.is_optional
        )
        self.assertEqual(variants[0].total_bits, expected)


# =============================================================================
# Tests — Real Spec Fixtures
# =============================================================================


class TestWithRealFixtures(TestCase):
    """Use the conftest mock specs to verify against known J2735 types."""

    def test_positional_accuracy_fixed(self) -> None:
        """PositionalAccuracy: 32 bits, no preamble, 3 fields."""
        variants = _get_variants("PositionalAccuracy", make_nested_mock_spec())

        self.assertEqual(len(variants), 1)
        self.assertEqual(variants[0].total_bits, 32)
        self.assertEqual(len(variants[0].fields), 3)
        self.assertIsNone(variants[0].ext_bit)
        self.assertEqual(variants[0].opt_bitmap, "")

    def test_intersection_reference_id_optional(self) -> None:
        """IntersectionReferenceID: 2 variants, 17 or 33 bits."""
        variants = _get_variants("IntersectionReferenceID", make_optional_mock_spec())

        self.assertEqual(len(variants), 2)
        # ABSENT: 1 opt bit + 16 id bits = 17
        self.assertEqual(variants[0].total_bits, 17)
        # PRESENT: 1 opt bit + 16 region + 16 id = 33
        self.assertEqual(variants[1].total_bits, 33)
        self.assertIn("region ABSENT", variants[0].name)
        self.assertIn("region PRESENT", variants[1].name)

    def test_path_prediction_extensible(self) -> None:
        """PathPrediction: 2 variants, 25 bits or variable."""
        variants = _get_variants("PathPrediction", make_extensible_mock_spec())

        self.assertEqual(len(variants), 2)
        # No ext: 1 ext bit + 16 radius + 8 confidence = 25
        self.assertEqual(variants[0].total_bits, 25)
        self.assertEqual(variants[1].total_bits, "variable")
        self.assertEqual(variants[0].ext_bit, 0)
        self.assertEqual(variants[1].ext_bit, 1)
