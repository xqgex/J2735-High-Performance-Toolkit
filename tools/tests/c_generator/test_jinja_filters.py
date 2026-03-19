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
"""Tests for Jinja filters.

Tests cover all filters registered in create_jinja_env():
    - filter_format_range: value range formatting for SequenceField
    - filter_is_signed: signed/unsigned detection for SequenceField
    - filter_screaming_snake: CamelCase -> SCREAMING_SNAKE_CASE
    - filter_snake_case: CamelCase -> snake_case
"""

from unittest import TestCase

from tools.j2735_c_generator_jinja import (
    filter_format_range,
    filter_is_signed,
    filter_screaming_snake,
    filter_snake_case,
)
from tools.j2735_spec_constraints import SequenceField, TypeReference
from tools.tests.conftest import (
    make_bitstring_field,
    make_integer_field,
)


class TestScreamingSnakeConversion(TestCase):
    """Tests for filter_screaming_snake filter function."""

    def test_simple_camel_case(self) -> None:
        """Simple CamelCase conversion."""
        self.assertEqual(filter_screaming_snake("MsgCount"), "MSG_COUNT")

    def test_uppercase_suffix(self) -> None:
        """Handle uppercase suffixes like ID."""
        self.assertEqual(filter_screaming_snake("TemporaryID"), "TEMPORARY_ID")

    def test_lowercase_input(self) -> None:
        """Handle all-lowercase input."""
        self.assertEqual(filter_screaming_snake("latitude"), "LATITUDE")

    def test_mixed_case_abbreviation(self) -> None:
        """Handle mixed case with abbreviations."""
        self.assertEqual(filter_screaming_snake("BSMcoreData"), "BSM_CORE_DATA")
        self.assertEqual(filter_screaming_snake("HTTPSconnection"), "HTTPS_CONNECTION")

    def test_digits_in_name(self) -> None:
        """Handle digits - split before and after."""
        self.assertEqual(filter_screaming_snake("AccelerationSet4Way"), "ACCELERATION_SET_4_WAY")
        self.assertEqual(filter_screaming_snake("Type2Value"), "TYPE_2_VALUE")

    def test_hyphenated_asn1_names(self) -> None:
        """Handle ASN.1 names with hyphens (e.g., Offset-B10, Node-LL-24B)."""
        self.assertEqual(filter_screaming_snake("Offset-B10"), "OFFSET_B_10")
        self.assertEqual(filter_screaming_snake("Node-LL-24B"), "NODE_LL_24_B")
        self.assertEqual(filter_screaming_snake("OffsetLL-B12"), "OFFSET_LL_B_12")
        self.assertEqual(filter_screaming_snake("NMEA-MsgType"), "NMEA_MSG_TYPE")
        self.assertEqual(filter_screaming_snake("RTCM-Revision"), "RTCM_REVISION")
        self.assertEqual(filter_screaming_snake("VertOffset-B07"), "VERT_OFFSET_B_07")
        self.assertEqual(filter_screaming_snake("TimeInSecond-B16"), "TIME_IN_SECOND_B_16")

    def test_already_screaming(self) -> None:
        """Already SCREAMING_SNAKE stays the same."""
        self.assertEqual(filter_screaming_snake("MSG_COUNT"), "MSG_COUNT")


class TestSnakeCaseConversion(TestCase):
    """Tests for filter_snake_case filter function."""

    def test_simple_camel_case(self) -> None:
        """Simple CamelCase conversion."""
        self.assertEqual(filter_snake_case("MsgCount"), "msg_count")

    def test_uppercase_suffix(self) -> None:
        """Handle uppercase suffixes like ID."""
        self.assertEqual(filter_snake_case("TemporaryID"), "temporary_id")

    def test_lowercase_input(self) -> None:
        """Handle all-lowercase input."""
        self.assertEqual(filter_snake_case("latitude"), "latitude")

    def test_mixed_case_abbreviation(self) -> None:
        """Handle mixed case with abbreviations."""
        self.assertEqual(filter_snake_case("BSMcoreData"), "bsm_core_data")
        self.assertEqual(filter_snake_case("PathPrediction"), "path_prediction")

    def test_digits_in_name(self) -> None:
        """Handle digits - split before and after."""
        self.assertEqual(filter_snake_case("AccelerationSet4Way"), "acceleration_set_4_way")
        self.assertEqual(filter_snake_case("Type2Value"), "type_2_value")

    def test_hyphenated_asn1_names(self) -> None:
        """Handle ASN.1 names with hyphens."""
        self.assertEqual(filter_snake_case("Offset-B10"), "offset_b_10")
        self.assertEqual(filter_snake_case("Node-LL-24B"), "node_ll_24_b")
        self.assertEqual(filter_snake_case("OffsetLL-B12"), "offset_ll_b_12")
        self.assertEqual(filter_snake_case("NMEA-MsgType"), "nmea_msg_type")

    def test_already_snake(self) -> None:
        """Already snake_case stays the same."""
        self.assertEqual(filter_snake_case("msg_count"), "msg_count")


# =============================================================================
# Tests - filter_is_signed()
# =============================================================================


class TestIsSigned(TestCase):
    """Tests for the filter_is_signed Jinja filter."""

    def test_positive_min_is_unsigned(self) -> None:
        """Field with min_value > 0 is unsigned."""
        field = make_integer_field("x", "X", min_value=1, max_value=100)
        self.assertFalse(filter_is_signed(field))

    def test_zero_min_is_unsigned(self) -> None:
        """Field with min_value == 0 is unsigned (boundary)."""
        field = make_integer_field("x", "X", min_value=0, max_value=255)
        self.assertFalse(filter_is_signed(field))

    def test_negative_min_is_signed(self) -> None:
        """Field with min_value < 0 is signed."""
        field = make_integer_field("x", "X", min_value=-100, max_value=100)
        self.assertTrue(filter_is_signed(field))

    def test_large_negative_min_is_signed(self) -> None:
        """Large negative value like Latitude (-900000000) is signed."""
        field = make_integer_field("lat", "Latitude", min_value=-900000000, max_value=900000001)
        self.assertTrue(filter_is_signed(field))

    def test_bitstring_field_is_unsigned(self) -> None:
        """BIT STRING field (no min_value) is treated as unsigned."""
        field = make_bitstring_field("flags", "Flags", 8)
        self.assertFalse(filter_is_signed(field))

    def test_unresolved_type_reference_is_unsigned(self) -> None:
        """Unresolved TypeReference has no min_value and returns False."""
        field = SequenceField(
            name="x",
            type_name="Unknown",
            type=TypeReference(name="Unknown"),
            is_optional=False,
            section_comment="",
            inline_comment="",
        )
        self.assertFalse(filter_is_signed(field))


# =============================================================================
# Tests - filter_format_range()
# =============================================================================


class TestFormatRange(TestCase):
    """Tests for the filter_format_range Jinja filter."""

    def test_unsigned_range(self) -> None:
        """Unsigned range formats as 'min..max'."""
        field = make_integer_field("x", "X", 0, 255)
        self.assertEqual(filter_format_range(field), "0..255")

    def test_signed_range(self) -> None:
        """Signed range includes negative min."""
        field = make_integer_field("x", "X", -100, 100)
        self.assertEqual(filter_format_range(field), "-100..100")

    def test_large_values(self) -> None:
        """Large values are formatted correctly (no truncation)."""
        field = make_integer_field("lat", "Latitude", -900000000, 900000001)
        self.assertEqual(filter_format_range(field), "-900000000..900000001")

    def test_single_value_range(self) -> None:
        """Range where min==max is still formatted."""
        field = make_integer_field("x", "X", 42, 42)
        self.assertEqual(filter_format_range(field), "42..42")

    def test_bitstring_returns_empty(self) -> None:
        """BIT STRING field has no range -> returns empty string."""
        field = make_bitstring_field("flags", "Flags", 8)
        self.assertEqual(filter_format_range(field), "")

    def test_unresolved_type_reference_returns_empty(self) -> None:
        """Unresolved TypeReference has no range and returns empty string."""
        field = SequenceField(
            name="x",
            type_name="Unknown",
            type=TypeReference(name="Unknown"),
            is_optional=False,
            section_comment="",
            inline_comment="",
        )
        self.assertEqual(filter_format_range(field), "")
