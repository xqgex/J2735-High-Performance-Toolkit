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
"""Tests for SequenceField parsing.

Tests cover SEQUENCE field extraction including:
    - Basic field parsing
    - OPTIONAL field handling
    - Section and inline comment handling
    - Extension marker (...) handling
"""

from unittest import TestCase

from tools.j2735_spec_constraints import SequenceField


class TestSequenceFieldParsing(TestCase):
    """Tests for SEQUENCE field parsing."""

    def test_simple_sequence(self) -> None:
        """Parse simple SEQUENCE with two fields."""
        fields = SequenceField.from_asn1("SEQUENCE { msgCnt MsgCount, id TemporaryID }")
        self.assertEqual(len(fields), 2)
        self.assertEqual(fields[0].name, "msgCnt")
        self.assertEqual(fields[0].type_name, "MsgCount")
        self.assertFalse(fields[0].is_optional)

    def test_sequence_with_optional(self) -> None:
        """Parse SEQUENCE with OPTIONAL field."""
        fields = SequenceField.from_asn1("SEQUENCE { x TypeA, y TypeB OPTIONAL }")
        self.assertEqual(len(fields), 2)
        self.assertFalse(fields[0].is_optional)
        self.assertTrue(fields[1].is_optional)

    def test_empty_braces(self) -> None:
        """Empty braces returns empty tuple."""
        fields = SequenceField.from_asn1("SEQUENCE { }")
        self.assertEqual(len(fields), 0)


class TestSequenceCommentParsing(TestCase):
    """Comprehensive tests for SEQUENCE comment handling.

    Covers all combinations of:
    - Section comments (standalone `-- comment` lines)
    - Inline comments (`field Type, -- comment`)
    - Extension markers (`...`) with and without comments
    - Multiple fields per line
    """

    def test_no_comments(self) -> None:
        """Fields with no comments have empty comment fields."""
        fields = SequenceField.from_asn1("SEQUENCE { a TypeA, b TypeB }")
        self.assertEqual(len(fields), 2)
        self.assertEqual(fields[0].section_comment, "")
        self.assertEqual(fields[0].inline_comment, "")
        self.assertEqual(fields[1].section_comment, "")
        self.assertEqual(fields[1].inline_comment, "")

    def test_section_comment_only(self) -> None:
        """Standalone comment line becomes section_comment for next field."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            -- Section header
            fieldA TypeA
        }"""
        )
        self.assertEqual(len(fields), 1)
        self.assertEqual(fields[0].name, "fieldA")
        self.assertEqual(fields[0].section_comment, "Section header")
        self.assertEqual(fields[0].inline_comment, "")

    def test_inline_comment_only(self) -> None:
        """Comment after field on same line becomes inline_comment."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            fieldA TypeA, -- This is inline
            fieldB TypeB
        }"""
        )
        self.assertEqual(len(fields), 2)
        self.assertEqual(fields[0].inline_comment, "This is inline")
        self.assertEqual(fields[0].section_comment, "")
        self.assertEqual(fields[1].inline_comment, "")

    def test_both_comment_types(self) -> None:
        """Field can have both section and inline comments."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            -- Group header
            value Count, -- The count value
        }"""
        )
        self.assertEqual(len(fields), 1)
        self.assertEqual(fields[0].section_comment, "Group header")
        self.assertEqual(fields[0].inline_comment, "The count value")

    def test_section_comment_applies_to_first_field_only(self) -> None:
        """Section comment attaches only to the first field after it."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            -- This is a section
            fieldA TypeA,
            fieldB TypeB,
            fieldC TypeC
        }"""
        )
        self.assertEqual(len(fields), 3)
        self.assertEqual(fields[0].section_comment, "This is a section")
        self.assertEqual(fields[1].section_comment, "")
        self.assertEqual(fields[2].section_comment, "")

    def test_multiple_section_comments(self) -> None:
        """Multiple section comments attach to their following fields."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            -- First group
            fieldA TypeA,
            -- Second group
            fieldB TypeB
        }"""
        )
        self.assertEqual(len(fields), 2)
        self.assertEqual(fields[0].section_comment, "First group")
        self.assertEqual(fields[1].section_comment, "Second group")

    def test_inline_comment_on_last_field_of_line(self) -> None:
        """Inline comment applies only to last field on multi-field line."""
        fields = SequenceField.from_asn1("SEQUENCE { a TypeA, b TypeB, c TypeC -- comment on c }")
        self.assertEqual(len(fields), 3)
        self.assertEqual(fields[0].inline_comment, "")
        self.assertEqual(fields[1].inline_comment, "")
        self.assertEqual(fields[2].inline_comment, "comment on c")

    def test_extension_marker_skipped(self) -> None:
        """Extension marker (...) is skipped, not parsed as field."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            fieldA TypeA,
            ...,
            fieldB TypeB
        }"""
        )
        self.assertEqual(len(fields), 2)
        self.assertEqual(fields[0].name, "fieldA")
        self.assertEqual(fields[1].name, "fieldB")

    def test_extension_marker_in_middle_with_comments(self) -> None:
        """Extension marker in middle preserves comments on surrounding fields."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            -- Before extension
            fieldA TypeA, -- inline A
            ...,
            -- After extension
            fieldB TypeB, -- inline B
        }"""
        )
        self.assertEqual(len(fields), 2)
        self.assertEqual(fields[0].section_comment, "Before extension")
        self.assertEqual(fields[0].inline_comment, "inline A")
        self.assertEqual(fields[1].section_comment, "After extension")
        self.assertEqual(fields[1].inline_comment, "inline B")

    def test_extension_marker_with_comment(self) -> None:
        """Comment on extension marker line is discarded."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            fieldA TypeA,
            ... -- LOCAL_CONTENT
        }"""
        )
        self.assertEqual(len(fields), 1)
        self.assertEqual(fields[0].name, "fieldA")

    def test_section_comment_before_extension_carries_over(self) -> None:
        """Section comment before extension marker carries to next real field."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            fieldA TypeA,
            -- Extension section
            ...,
            fieldB TypeB
        }"""
        )
        self.assertEqual(len(fields), 2)
        # The section comment carries over because ... is skipped without
        # consuming the pending section comment
        self.assertEqual(fields[1].section_comment, "Extension section")

    def test_multiple_fields_per_line_with_section_comment(self) -> None:
        """Section comment on line with multiple fields attaches to first."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            -- Multi-field line
            a TypeA, b TypeB, c TypeC
        }"""
        )
        self.assertEqual(len(fields), 3)
        self.assertEqual(fields[0].section_comment, "Multi-field line")
        self.assertEqual(fields[1].section_comment, "")
        self.assertEqual(fields[2].section_comment, "")

    def test_real_world_vehicle_data(self) -> None:
        """Real VehicleData example from J2735 spec."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            -- Values for width and length are sent in BSM part I
            height VehicleHeight OPTIONAL,
            bumpers BumperHeights OPTIONAL,
            mass VehicleMass OPTIONAL,
            doNotUse TrailerWeight OPTIONAL, -- Do not use
            ...,
            trailerPresent BOOLEAN OPTIONAL,
            pivotPoint PivotPointDescription OPTIONAL, -- Angle ignored
            axles Axles OPTIONAL,
            leanAngle INTEGER OPTIONAL -- For motorcycles only
        }"""
        )
        self.assertEqual(len(fields), 8)
        self.assertEqual(
            fields[0].section_comment, "Values for width and length are sent in BSM part I"
        )
        self.assertEqual(fields[0].inline_comment, "")
        self.assertEqual(fields[3].name, "doNotUse")
        self.assertEqual(fields[3].inline_comment, "Do not use")
        self.assertEqual(fields[4].name, "trailerPresent")
        self.assertEqual(fields[5].inline_comment, "Angle ignored")
        self.assertEqual(fields[7].inline_comment, "For motorcycles only")

    def test_real_world_school_bus(self) -> None:
        """Real SchoolBus example with multiple section comments."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            flashingAmberLights BOOLEAN,
            flashingRedLights BOOLEAN,
            -- School bus safety indicators
            studentsCrossingFront BOOLEAN OPTIONAL,
            studentsCrossingBehind BOOLEAN OPTIONAL,
            doorOpen BOOLEAN OPTIONAL,
            stopArmExtended BOOLEAN OPTIONAL,
            stopArmRetracted BOOLEAN OPTIONAL,
            -- Emergency indicators
            emergencyExitOpen BOOLEAN OPTIONAL,
            ...
        }"""
        )
        self.assertEqual(len(fields), 8)
        self.assertEqual(fields[0].section_comment, "")
        self.assertEqual(fields[1].section_comment, "")
        self.assertEqual(fields[2].section_comment, "School bus safety indicators")
        self.assertEqual(fields[2].name, "studentsCrossingFront")
        self.assertEqual(fields[7].section_comment, "Emergency indicators")
        self.assertEqual(fields[7].name, "emergencyExitOpen")


class TestSequenceNestedBraces(TestCase):
    """Tests for SEQUENCE parsing with nested braces.

    Bug: The current regex [^{}]+ fails when SEQUENCE contains nested
    braces like {{Reg-Position3D}} in regional extension fields.
    """

    def test_nested_braces_regional_extension(self) -> None:
        """Parse SEQUENCE with regional extension containing nested braces."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            lat Latitude,
            long Longitude,
            elevation Elevation OPTIONAL,
            regional SEQUENCE (SIZE(1..4)) OF RegionalExtension {{Reg-Position3D}} OPTIONAL,
            ...
        }"""
        )
        # Bug: Currently returns 0 fields because {{...}} breaks the regex
        self.assertGreaterEqual(len(fields), 3, "Should parse at least lat, long, elevation")
        self.assertEqual(fields[0].name, "lat")
        self.assertEqual(fields[0].type_name, "Latitude")
        self.assertEqual(fields[1].name, "long")
        self.assertEqual(fields[2].name, "elevation")
        self.assertTrue(fields[2].is_optional)

    def test_nested_braces_multiple_levels(self) -> None:
        """Parse SEQUENCE with deeply nested braces."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            coreData BSMcoreData,
            partII SEQUENCE (SIZE(1..8)) OF PartIIcontent {{ BSMpartIIExtension }} OPTIONAL,
            regional SEQUENCE (SIZE(1..4)) OF RegionalExtension {{Reg-BasicSafetyMessage}} OPTIONAL,
            ...
        }"""
        )
        self.assertGreaterEqual(len(fields), 1, "Should parse at least coreData")
        self.assertEqual(fields[0].name, "coreData")
        self.assertEqual(fields[0].type_name, "BSMcoreData")

    def test_simple_nested_braces(self) -> None:
        """Simple case with nested braces but no regional extension."""
        fields = SequenceField.from_asn1(
            """SEQUENCE {
            id VehicleID,
            value MESSAGE-ID-AND-TYPE.&Type({MessageTypes}{@.messageId})
        }"""
        )
        self.assertGreaterEqual(len(fields), 1, "Should parse at least id field")
        self.assertEqual(fields[0].name, "id")
