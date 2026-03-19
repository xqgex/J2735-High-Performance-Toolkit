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
"""Tests for SpecEntry.from_*_block class methods.

Tests cover the three block-parsing classmethods that convert raw
specification text blocks into SpecEntry instances:
    - from_data_element_block (section 7, multiline ASN.1 strategy)
    - from_data_frame_block (section 6, DOTALL ASN.1 strategy)
    - from_message_block (section 5, DOTALL ASN.1 strategy + abbreviation)
"""

from unittest import TestCase

from tools.j2735_spec_parser import (
    ASN1TypeClass,
    J2735EntryKind,
    SpecEntry,
)

# =============================================================================
# Synthetic Blocks - Data Elements (Section 7)
# =============================================================================

_DE_FULL_BLOCK = """\
7.99 Data Element: DE_MsgCount

Use: A counter that increments each time a new message is generated.

ASN.1 Representation:
MsgCount ::= INTEGER (0..127)

Remarks: This is a simple counter.
"""

_DE_MULTILINE_BLOCK = """\
7.50 Data Element: DE_AllowedManeuvers

Use: A BIT STRING defining allowed maneuvers at an intersection.

ASN.1 Representation:
AllowedManeuvers ::= BIT STRING {
    maneuverStraightAllowed     (0),
    maneuverLeftAllowed         (1),
    maneuverRightAllowed        (2),
    maneuverUTurnAllowed        (3),
    maneuverLeftTurnOnRedAllowed (4),
    maneuverRightTurnOnRedAllowed (5),
    maneuverLaneChangeAllowed   (6),
    maneuverNoStoppingAllowed   (7),
    yieldAllwaysRequired        (8),
    goWithHalt                  (9),
    caution                     (10),
    reserved1                   (11)
} (SIZE(12))

Remarks: Bits map to maneuver permissions.
"""

_DE_WITH_ENCODING_COMMENT_BLOCK = """\
7.10 Data Element: DE_Elevation

Use: The geographic elevation.

ASN.1 Representation:
Elevation ::= INTEGER (-4096..61439)
-- In units of 10 cm steps above WGS-84 reference
-- size: 16 bits

Remarks: Uses WGS-84 reference.
"""

_DE_NO_USE_NO_REMARKS_BLOCK = """\
7.77 Data Element: DE_SpeedAdvice

ASN.1 Representation:
SpeedAdvice ::= INTEGER (0..500)
"""

_DE_NO_ASN1_BLOCK = """\
7.88 Data Element: DE_Mystery

Use: This element has no ASN.1 definition yet.

Remarks: Pending standardization.
"""

_DE_INVALID_HEADER = "This is not a valid block at all."


# =============================================================================
# Synthetic Blocks - Data Frames (Section 6)
# =============================================================================

_DF_FULL_BLOCK = """\
6.10 Data Frame: DF_PathPrediction

Use: Conveys a predicted path with a confidence value.

ASN.1 Representation:
PathPrediction ::= SEQUENCE {
    radiusOfCurve  RadiusOfCurvature,
    confidence     Confidence
}

Remarks: Used for path prediction in intersection safety.
"""

_DF_NO_USE_NO_REMARKS_BLOCK = """\
6.20 Data Frame: DF_SimpleFrame

ASN.1 Representation:
SimpleFrame ::= SEQUENCE {
    value  INTEGER (0..255)
}
"""

_DF_NO_ASN1_BLOCK = """\
6.30 Data Frame: DF_EmptyFrame

Use: A frame without ASN.1 definition.

Remarks: Not yet defined.
"""

_DF_INVALID_HEADER = "No data frame header here."


# =============================================================================
# Synthetic Blocks - Messages (Section 5)
# =============================================================================

_MSG_WITH_ABBREVIATION_BLOCK = """\
5.2 Message: MSG_BasicSafetyMessage (BSM)

Use: A basic safety message broadcast by vehicles.

ASN.1 Representation:
BasicSafetyMessage ::= SEQUENCE {
    coreData  BSMcoreData
}

Remarks: Transmitted at 10 Hz.
"""

_MSG_NO_ABBREVIATION_BLOCK = """\
5.8 Message: MSG_ProbeDataManagement

Use: Controls how probe data is collected.

ASN.1 Representation:
ProbeDataManagement ::= SEQUENCE {
    sample  INTEGER (0..255)
}

Remarks: Used by TMC.
"""

_MSG_NO_USE_NO_REMARKS_BLOCK = """\
5.15 Message: MSG_TestOnly

ASN.1 Representation:
TestOnly ::= SEQUENCE {
    value  INTEGER (0..7)
}
"""

_MSG_INVALID_HEADER = "Not a message block."


# =============================================================================
# Test Cases - from_data_element_block
# =============================================================================


class TestFromDataElementBlock(TestCase):
    """Tests for SpecEntry.from_data_element_block."""

    def test_returns_none_on_invalid_header(self) -> None:
        """Returns None when block has no matching header."""
        result = SpecEntry.from_data_element_block(_DE_INVALID_HEADER, 1)
        self.assertIsNone(result)

    def test_full_block_section_number(self) -> None:
        """Extracts correct section number."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 100)
        self.assertIsNotNone(entry)
        assert entry is not None
        self.assertEqual(entry.section_number, "7.99")

    def test_full_block_entry_type(self) -> None:
        """Sets entry_type to DATA_ELEMENTS."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.entry_type, J2735EntryKind.DATA_ELEMENTS)

    def test_full_block_name(self) -> None:
        """Extracts name without DE_ prefix."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.name, "MsgCount")

    def test_full_block_abbreviation_is_empty(self) -> None:
        """Data Elements always have empty abbreviation."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.abbreviation, "")

    def test_full_block_use_description(self) -> None:
        """Extracts Use: description text."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None
        self.assertIn("counter", entry.use_description)

    def test_full_block_remarks(self) -> None:
        """Extracts Remarks: text."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None
        self.assertIn("simple counter", entry.remarks)

    def test_full_block_line_number(self) -> None:
        """Stores the line offset."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 42)
        assert entry is not None
        self.assertEqual(entry.line_number, 42)

    def test_full_block_asn1_parsed(self) -> None:
        """Parses simple INTEGER ASN.1 definition."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None
        self.assertIsNotNone(entry.asn1_definition)
        assert entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.name, "MsgCount")
        self.assertEqual(entry.asn1_definition.type_class, ASN1TypeClass.INTEGER)

    def test_full_block_asn1_bit_width(self) -> None:
        """MsgCount INTEGER (0..127) is 7 bits."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None and entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.uper_bit_width, 7)

    def test_multiline_bit_string_parsed(self) -> None:
        """Multiline BIT STRING with named bits is correctly assembled."""
        entry = SpecEntry.from_data_element_block(_DE_MULTILINE_BLOCK, 1)
        assert entry is not None
        self.assertIsNotNone(entry.asn1_definition)
        assert entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.name, "AllowedManeuvers")
        self.assertEqual(entry.asn1_definition.type_class, ASN1TypeClass.BIT_STRING)

    def test_encoding_comment_preserved(self) -> None:
        """Comments containing 'size' or 'bit' keywords are kept in type body."""
        entry = SpecEntry.from_data_element_block(_DE_WITH_ENCODING_COMMENT_BLOCK, 1)
        assert entry is not None
        self.assertIsNotNone(entry.asn1_definition)
        assert entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.name, "Elevation")
        # The "-- size: 16 bits" comment should be preserved in the raw definition
        self.assertIn("size", entry.asn1_definition.raw_definition.lower())

    def test_missing_use_and_remarks(self) -> None:
        """Block without Use: and Remarks: sets empty strings."""
        entry = SpecEntry.from_data_element_block(_DE_NO_USE_NO_REMARKS_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.use_description, "")
        self.assertEqual(entry.remarks, "")

    def test_missing_asn1(self) -> None:
        """Block without ASN.1 Representation sets asn1_definition to None."""
        entry = SpecEntry.from_data_element_block(_DE_NO_ASN1_BLOCK, 1)
        assert entry is not None
        self.assertIsNone(entry.asn1_definition)
        # Other fields should still be populated
        self.assertEqual(entry.name, "Mystery")

    def test_asn1_spec_section_propagated(self) -> None:
        """The section number is passed through to ASN1TypeDefinition."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None and entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.spec_section, "7.99")

    def test_asn1_description_propagated(self) -> None:
        """The use description is passed through to ASN1TypeDefinition."""
        entry = SpecEntry.from_data_element_block(_DE_FULL_BLOCK, 1)
        assert entry is not None and entry.asn1_definition is not None
        self.assertIn("counter", entry.asn1_definition.description)


# =============================================================================
# Test Cases - from_data_frame_block
# =============================================================================


class TestFromDataFrameBlock(TestCase):
    """Tests for SpecEntry.from_data_frame_block."""

    def test_returns_none_on_invalid_header(self) -> None:
        """Returns None when block has no matching header."""
        result = SpecEntry.from_data_frame_block(_DF_INVALID_HEADER, 1)
        self.assertIsNone(result)

    def test_full_block_section_number(self) -> None:
        """Extracts correct section number."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.section_number, "6.10")

    def test_full_block_entry_type(self) -> None:
        """Sets entry_type to DATA_FRAMES."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.entry_type, J2735EntryKind.DATA_FRAMES)

    def test_full_block_name(self) -> None:
        """Extracts name without DF_ prefix."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.name, "PathPrediction")

    def test_full_block_abbreviation_is_empty(self) -> None:
        """Data Frames always have empty abbreviation."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.abbreviation, "")

    def test_full_block_use_description(self) -> None:
        """Extracts Use: description text."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 1)
        assert entry is not None
        self.assertIn("predicted path", entry.use_description)

    def test_full_block_remarks(self) -> None:
        """Extracts Remarks: text."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 1)
        assert entry is not None
        self.assertIn("path prediction", entry.remarks.lower())

    def test_full_block_asn1_parsed(self) -> None:
        """Parses SEQUENCE ASN.1 definition."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 1)
        assert entry is not None
        self.assertIsNotNone(entry.asn1_definition)
        assert entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.name, "PathPrediction")
        self.assertEqual(entry.asn1_definition.type_class, ASN1TypeClass.SEQUENCE)

    def test_missing_use_and_remarks(self) -> None:
        """Block without Use: and Remarks: sets empty strings."""
        entry = SpecEntry.from_data_frame_block(_DF_NO_USE_NO_REMARKS_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.use_description, "")
        self.assertEqual(entry.remarks, "")

    def test_missing_asn1(self) -> None:
        """Block without ASN.1 Representation sets asn1_definition to None."""
        entry = SpecEntry.from_data_frame_block(_DF_NO_ASN1_BLOCK, 1)
        assert entry is not None
        self.assertIsNone(entry.asn1_definition)
        self.assertEqual(entry.name, "EmptyFrame")

    def test_asn1_spec_section_propagated(self) -> None:
        """The section number is passed through to ASN1TypeDefinition."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 1)
        assert entry is not None and entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.spec_section, "6.10")

    def test_line_number_stored(self) -> None:
        """Stores the line offset."""
        entry = SpecEntry.from_data_frame_block(_DF_FULL_BLOCK, 99)
        assert entry is not None
        self.assertEqual(entry.line_number, 99)


# =============================================================================
# Test Cases - from_message_block
# =============================================================================


class TestFromMessageBlock(TestCase):
    """Tests for SpecEntry.from_message_block."""

    def test_returns_none_on_invalid_header(self) -> None:
        """Returns None when block has no matching header."""
        result = SpecEntry.from_message_block(_MSG_INVALID_HEADER, 1)
        self.assertIsNone(result)

    def test_full_block_section_number(self) -> None:
        """Extracts correct section number."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.section_number, "5.2")

    def test_full_block_entry_type(self) -> None:
        """Sets entry_type to MESSAGES."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.entry_type, J2735EntryKind.MESSAGES)

    def test_full_block_name(self) -> None:
        """Extracts name without MSG_ prefix."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.name, "BasicSafetyMessage")

    def test_abbreviation_extracted(self) -> None:
        """Extracts abbreviation from parentheses in header."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.abbreviation, "BSM")

    def test_no_abbreviation(self) -> None:
        """Sets empty abbreviation when none present in header."""
        entry = SpecEntry.from_message_block(_MSG_NO_ABBREVIATION_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.abbreviation, "")

    def test_full_block_use_description(self) -> None:
        """Extracts Use: description text."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 1)
        assert entry is not None
        self.assertIn("basic safety message", entry.use_description.lower())

    def test_full_block_remarks(self) -> None:
        """Extracts Remarks: text."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 1)
        assert entry is not None
        self.assertIn("10 Hz", entry.remarks)

    def test_full_block_asn1_parsed(self) -> None:
        """Parses SEQUENCE ASN.1 definition."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 1)
        assert entry is not None
        self.assertIsNotNone(entry.asn1_definition)
        assert entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.name, "BasicSafetyMessage")
        self.assertEqual(entry.asn1_definition.type_class, ASN1TypeClass.SEQUENCE)

    def test_missing_use_and_remarks(self) -> None:
        """Block without Use: and Remarks: sets empty strings."""
        entry = SpecEntry.from_message_block(_MSG_NO_USE_NO_REMARKS_BLOCK, 1)
        assert entry is not None
        self.assertEqual(entry.use_description, "")
        self.assertEqual(entry.remarks, "")

    def test_asn1_spec_section_propagated(self) -> None:
        """The section number is passed through to ASN1TypeDefinition."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 1)
        assert entry is not None and entry.asn1_definition is not None
        self.assertEqual(entry.asn1_definition.spec_section, "5.2")

    def test_line_number_stored(self) -> None:
        """Stores the line offset."""
        entry = SpecEntry.from_message_block(_MSG_WITH_ABBREVIATION_BLOCK, 7)
        assert entry is not None
        self.assertEqual(entry.line_number, 7)
