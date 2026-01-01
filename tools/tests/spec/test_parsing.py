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
"""Tests for from_asn1 parsing methods.

Tests cover the parsing of ASN.1 type definition strings into
constraint objects.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import (
    BooleanType,
    ChoiceType,
    EnumeratedType,
    IA5StringConstraint,
    IntegerConstraint,
    OctetStringConstraint,
    SequenceOfType,
    SequenceType,
    TypeReference,
)
from tools.j2735_spec_parser import ASN1TypeClass


class TestParsingFunctions(TestCase):
    """Tests for constraint from_asn1 class methods."""

    def test_parse_integer_no_constraint(self) -> None:
        """INTEGER without constraint returns None."""
        self.assertIsNone(IntegerConstraint.from_asn1("INTEGER"))

    def test_parse_integer_with_constraint(self) -> None:
        """INTEGER with constraint parses correctly."""
        c = IntegerConstraint.from_asn1("INTEGER (0..127)")
        self.assertIsNotNone(c)
        assert c
        self.assertEqual(c.min_value, 0)
        self.assertEqual(c.max_value, 127)

    def test_parse_octet_string_fixed(self) -> None:
        """OCTET STRING (SIZE(4)) parses correctly."""
        c = OctetStringConstraint.from_asn1("OCTET STRING (SIZE(4))")
        self.assertIsNotNone(c)
        assert c
        self.assertEqual(c.min_size, 4)
        self.assertEqual(c.max_size, 4)

    def test_parse_octet_string_variable(self) -> None:
        """OCTET STRING (SIZE(1..8)) parses correctly."""
        c = OctetStringConstraint.from_asn1("OCTET STRING (SIZE(1..8))")
        self.assertIsNotNone(c)
        assert c
        self.assertEqual(c.min_size, 1)
        self.assertEqual(c.max_size, 8)

    def test_parse_enumerated_extensible(self) -> None:
        """ENUMERATED with ... is extensible."""
        e = EnumeratedType.from_asn1("ENUMERATED { a (0), b (1), ... }")
        self.assertIsNotNone(e)
        assert e
        self.assertTrue(e.is_extensible)

    def test_parse_enumerated_not_extensible(self) -> None:
        """ENUMERATED without ... is not extensible."""
        e = EnumeratedType.from_asn1("ENUMERATED { a (0), b (1) }")
        self.assertIsNotNone(e)
        assert e
        self.assertFalse(e.is_extensible)

    def test_classify_types(self) -> None:
        """Type classification is correct."""
        self.assertEqual(ASN1TypeClass.from_definition("INTEGER (0..127)"), ASN1TypeClass.INTEGER)
        self.assertEqual(ASN1TypeClass.from_definition("BOOLEAN"), ASN1TypeClass.BOOLEAN)
        self.assertEqual(
            ASN1TypeClass.from_definition("ENUMERATED { a (0) }"), ASN1TypeClass.ENUMERATED
        )
        self.assertEqual(ASN1TypeClass.from_definition("SEQUENCE { x Y }"), ASN1TypeClass.SEQUENCE)
        self.assertEqual(
            ASN1TypeClass.from_definition("BIT STRING { a (0) } (SIZE(8))"),
            ASN1TypeClass.BIT_STRING,
        )
        self.assertEqual(ASN1TypeClass.from_definition("CHOICE { a A, b B }"), ASN1TypeClass.CHOICE)
        # Type references (like SomeOtherType) are classified as TYPE_REFERENCE
        self.assertEqual(
            ASN1TypeClass.from_definition("SomeUnknownType"), ASN1TypeClass.TYPE_REFERENCE
        )


class TestBooleanTypeParsing(TestCase):
    """Edge case tests for BooleanType.from_asn1()."""

    def test_non_matching_returns_none(self) -> None:
        """Non-BOOLEAN input returns None."""
        self.assertIsNone(BooleanType.from_asn1("INTEGER"))
        self.assertIsNone(BooleanType.from_asn1("SEQUENCE { }"))
        self.assertIsNone(BooleanType.from_asn1(""))

    def test_case_sensitive(self) -> None:
        """BOOLEAN is case-sensitive per ASN.1 spec."""
        self.assertIsNone(BooleanType.from_asn1("boolean"))
        self.assertIsNone(BooleanType.from_asn1("Boolean"))
        self.assertIsNone(BooleanType.from_asn1("BOOL"))

    def test_boolean_with_suffix_rejected(self) -> None:
        """BOOLEAN followed by other text is rejected."""
        self.assertIsNone(BooleanType.from_asn1("BOOLEANX"))

    def test_boolean_valid(self) -> None:
        """Valid BOOLEAN is accepted."""
        result = BooleanType.from_asn1("BOOLEAN")
        self.assertIsNotNone(result)
        assert result
        self.assertEqual(result.uper_bit_width, 1)


class TestChoiceTypeParsing(TestCase):
    """Edge case tests for ChoiceType.from_asn1()."""

    def test_from_asn1_empty_body_returns_none(self) -> None:
        """CHOICE with empty body returns None."""
        self.assertIsNone(ChoiceType.from_asn1("CHOICE { }"))
        self.assertIsNone(ChoiceType.from_asn1("CHOICE {}"))

    def test_from_asn1_non_choice_returns_none(self) -> None:
        """Non-CHOICE input returns None."""
        self.assertIsNone(ChoiceType.from_asn1("SEQUENCE { a A }"))
        self.assertIsNone(ChoiceType.from_asn1("INTEGER"))

    def test_from_asn1_malformed_alternatives(self) -> None:
        """Malformed alternatives are skipped."""
        result = ChoiceType.from_asn1("CHOICE { , , }")
        self.assertIsNone(result)


class TestSequenceTypeParsing(TestCase):
    """Edge case tests for SequenceType.from_asn1()."""

    def test_from_asn1_empty_returns_none(self) -> None:
        """SEQUENCE with empty body returns None."""
        self.assertIsNone(SequenceType.from_asn1("SEQUENCE { }"))
        self.assertIsNone(SequenceType.from_asn1("SEQUENCE {}"))

    def test_from_asn1_non_sequence_returns_none(self) -> None:
        """Non-SEQUENCE input returns None (security: type confusion prevention)."""
        self.assertIsNone(SequenceType.from_asn1("CHOICE { a A }"))
        self.assertIsNone(SequenceType.from_asn1("ENUMERATED { a }"))

    def test_from_asn1_sequence_of_returns_none(self) -> None:
        """SEQUENCE OF input returns None (handled separately)."""
        self.assertIsNone(SequenceType.from_asn1("SEQUENCE OF Byte"))
        self.assertIsNone(SequenceType.from_asn1("SEQUENCE (SIZE(1..10)) OF Element"))


class TestIA5StringParsing(TestCase):
    """Edge case tests for IA5StringConstraint.from_asn1()."""

    def test_from_asn1_no_size_returns_none(self) -> None:
        """IA5String without SIZE returns None."""
        self.assertIsNone(IA5StringConstraint.from_asn1("IA5String"))
        self.assertIsNone(IA5StringConstraint.from_asn1("IA5String -- comment"))

    def test_from_asn1_non_ia5string(self) -> None:
        """Non-IA5String returns None."""
        self.assertIsNone(IA5StringConstraint.from_asn1("UTF8String (SIZE(10))"))


class TestOctetStringParsing(TestCase):
    """Edge case tests for OctetStringConstraint.from_asn1()."""

    def test_from_asn1_no_size_returns_none(self) -> None:
        """OCTET STRING without SIZE returns None."""
        self.assertIsNone(OctetStringConstraint.from_asn1("OCTET STRING"))

    def test_from_asn1_non_octet_string(self) -> None:
        """Non-OCTET STRING returns None."""
        self.assertIsNone(OctetStringConstraint.from_asn1("BIT STRING (SIZE(8))"))


class TestEnumeratedTypeParsing(TestCase):
    """Edge case tests for EnumeratedType.from_asn1()."""

    def test_from_asn1_empty_body_returns_none(self) -> None:
        """ENUMERATED with empty body returns None."""
        self.assertIsNone(EnumeratedType.from_asn1("ENUMERATED { }"))
        self.assertIsNone(EnumeratedType.from_asn1("ENUMERATED {}"))

    def test_from_asn1_only_extension_marker(self) -> None:
        """ENUMERATED with only extension marker returns None."""
        self.assertIsNone(EnumeratedType.from_asn1("ENUMERATED { ... }"))

    def test_from_asn1_non_enumerated(self) -> None:
        """Non-ENUMERATED input returns None."""
        self.assertIsNone(EnumeratedType.from_asn1("SEQUENCE { a A }"))

    def test_negative_explicit_values_rejected(self) -> None:
        """Negative explicit values cause parse failure (J2735 doesn't use them)."""
        # Should return None, not silently corrupt data
        e = EnumeratedType.from_asn1("ENUMERATED { neg(-1), zero(0), pos(1) }")
        self.assertIsNone(e)

    def test_unrecognized_tokens_rejected(self) -> None:
        """Unrecognized tokens cause parse failure, not silent ignore.

        Safety-critical principle: we must fail loudly on malformed input,
        never silently skip parts we don't understand.
        """
        # Hex values - not valid in ASN.1 ENUMERATED
        self.assertIsNone(EnumeratedType.from_asn1("ENUMERATED { hex(0xFF) }"))
        # Float values
        self.assertIsNone(EnumeratedType.from_asn1("ENUMERATED { flt(1.5) }"))
        # Non-numeric in parens
        self.assertIsNone(EnumeratedType.from_asn1("ENUMERATED { bad(abc) }"))
        # Random garbage
        self.assertIsNone(EnumeratedType.from_asn1("ENUMERATED { a(0) @#$ b(1) }"))
        # Unclosed paren
        self.assertIsNone(EnumeratedType.from_asn1("ENUMERATED { bad(1 }"))

    def test_valid_with_comments_accepted(self) -> None:
        """Valid ENUMERATED with ASN.1 comments parses correctly."""
        e = EnumeratedType.from_asn1("ENUMERATED { a(0), -- comment\n b(1) -- another }")
        self.assertIsNotNone(e)
        assert e
        self.assertEqual(e.values, {"a": 0, "b": 1})

    def test_large_explicit_values(self) -> None:
        """Large explicit values don't overflow."""
        e = EnumeratedType.from_asn1("ENUMERATED { big(2147483647) }")
        self.assertIsNotNone(e)
        assert e
        self.assertEqual(e.values["big"], 2147483647)


class TestSequenceOfTypeParsing(TestCase):
    """Edge case tests for SequenceOfType."""

    def test_from_asn1_always_none(self) -> None:
        """SEQUENCE OF from_asn1 always returns None (handled specially)."""
        self.assertIsNone(SequenceOfType.from_asn1("SEQUENCE (SIZE(1..10)) OF Byte"))
        self.assertIsNone(SequenceOfType.from_asn1(""))


class TestTypeReferenceParsing(TestCase):
    """Edge case tests for TypeReference."""

    def test_from_asn1_always_none(self) -> None:
        """TypeReference.from_asn1 always returns None."""
        self.assertIsNone(TypeReference.from_asn1("SomeType"))
        self.assertIsNone(TypeReference.from_asn1(""))
