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
"""Integration tests for J2735 specification file parsing.

These tests run against the actual J2735 specification file to verify
end-to-end parsing and code generation workflows.
"""

from unittest import TestCase, skipIf

from tools.j2735_spec_constraints import EnumeratedType, SequenceType
from tools.j2735_spec_parser import ASN1TypeClass, J2735Specification, parse_spec_file
from tools.tests.conftest import SPEC_FILE_PATH


@skipIf(not SPEC_FILE_PATH.exists(), f"Spec file not found: {SPEC_FILE_PATH}")
class TestSpecFileParsing(TestCase):
    """Integration tests against the actual J2735 specification file."""

    spec: J2735Specification

    @classmethod
    def setUpClass(cls) -> None:
        """Parse the spec file once for all tests."""
        cls.spec = parse_spec_file(SPEC_FILE_PATH)

    def test_parsing_succeeded(self) -> None:
        """Basic parsing produces non-empty results."""
        self.assertTrue(self.spec.messages, "No messages parsed")
        self.assertTrue(self.spec.data_frames, "No data frames parsed")
        self.assertTrue(self.spec.data_elements, "No data elements parsed")

    def test_version_extracted(self) -> None:
        """Specification version is extracted."""
        self.assertTrue(self.spec.version, "No version extracted")

    def test_msg_count_bit_width(self) -> None:
        """MsgCount is 7 bits (0..127)."""
        typedef = self.spec.lookup_type("MsgCount")
        self.assertIsNotNone(typedef, "MsgCount not found")
        assert typedef is not None, "MsgCount not found"
        self.assertEqual(typedef.uper_bit_width, 7)

    def test_dsecond_bit_width(self) -> None:
        """DSecond is 16 bits."""
        typedef = self.spec.lookup_type("DSecond")
        self.assertIsNotNone(typedef, "DSecond not found")
        assert typedef is not None, "DSecond not found"
        self.assertEqual(typedef.uper_bit_width, 16)

    def test_latitude_bit_width(self) -> None:
        """Latitude is 31 bits (-900000000..900000001)."""
        typedef = self.spec.lookup_type("Latitude")
        self.assertIsNotNone(typedef, "Latitude not found")
        assert typedef is not None, "Latitude not found"
        self.assertEqual(typedef.uper_bit_width, 31)

    def test_longitude_bit_width(self) -> None:
        """Longitude is 32 bits."""
        typedef = self.spec.lookup_type("Longitude")
        self.assertIsNotNone(typedef, "Longitude not found")
        assert typedef is not None, "Longitude not found"
        self.assertEqual(typedef.uper_bit_width, 32)

    def test_temporary_id_bit_width(self) -> None:
        """TemporaryID is 32 bits (4 octets)."""
        typedef = self.spec.lookup_type("TemporaryID")
        self.assertIsNotNone(typedef, "TemporaryID not found")
        assert typedef is not None, "TemporaryID not found"
        self.assertEqual(typedef.uper_bit_width, 32)

    def test_bsm_core_data_fields(self) -> None:
        """BSMcoreData SEQUENCE has expected fields."""
        bsm_core = self.spec.lookup_type("BSMcoreData")
        self.assertIsNotNone(bsm_core, "BSMcoreData not found")
        assert bsm_core is not None, "BSMcoreData not found"
        self.assertIsInstance(bsm_core.constraint, SequenceType)
        assert isinstance(bsm_core.constraint, SequenceType)

        expected_fields = ["msgCnt", "id", "secMark", "lat", "long", "elev"]
        actual_fields = [f.name for f in bsm_core.constraint.fields]

        for expected in expected_fields:
            self.assertIn(expected, actual_fields, f"Missing field: {expected}")

    def test_hyphenated_type_names_parsed(self) -> None:
        """Type names with hyphens (e.g., Offset-B10) are correctly parsed.

        The J2735 spec defines types like Offset-B10, OffsetLL-B12, TimeInSecond-B8.
        These must be in the type_registry for SEQUENCE field references to resolve.
        """
        # These types have hyphens in their ASN.1 names
        hyphenated_types = [
            ("Offset-B10", 10),  # INTEGER (-512..511) = 10 bits
            ("Offset-B12", 12),  # INTEGER (-2048..2047) = 12 bits
            ("OffsetLL-B12", 12),  # INTEGER (-2048..2047) = 12 bits
        ]

        for type_name, expected_bits in hyphenated_types:
            typedef = self.spec.lookup_type(type_name)
            self.assertIsNotNone(
                typedef,
                f"Hyphenated type '{type_name}' not found in type_registry. "
                f"Parser may not handle hyphens in ASN.1 type names.",
            )
            assert typedef is not None
            self.assertEqual(
                typedef.uper_bit_width,
                expected_bits,
                f"{type_name} should be {expected_bits} bits",
            )

    def test_enumerated_types_parsed(self) -> None:
        """At least one ENUMERATED type is correctly parsed."""
        enum_types = [
            td
            for td in self.spec.type_registry.values()
            if td.type_class == ASN1TypeClass.ENUMERATED
        ]
        self.assertTrue(enum_types, "No ENUMERATED types found")

        typedef = enum_types[0]
        self.assertIsInstance(typedef.constraint, EnumeratedType)
        assert isinstance(typedef.constraint, EnumeratedType)
        self.assertGreater(typedef.constraint.value_count, 0)
