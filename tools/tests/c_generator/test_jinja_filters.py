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

Tests cover the screaming_snake and snake_case filters used in Jinja templates.
"""

from unittest import TestCase

from tools.j2735_c_generator_jinja import screaming_snake, snake_case


class TestScreamingSnakeConversion(TestCase):
    """Tests for screaming_snake filter function."""

    def test_simple_camel_case(self) -> None:
        """Simple CamelCase conversion."""
        self.assertEqual(screaming_snake("MsgCount"), "MSG_COUNT")

    def test_uppercase_suffix(self) -> None:
        """Handle uppercase suffixes like ID."""
        self.assertEqual(screaming_snake("TemporaryID"), "TEMPORARY_ID")

    def test_lowercase_input(self) -> None:
        """Handle all-lowercase input."""
        self.assertEqual(screaming_snake("latitude"), "LATITUDE")

    def test_mixed_case_abbreviation(self) -> None:
        """Handle mixed case with abbreviations."""
        self.assertEqual(screaming_snake("BSMcoreData"), "BSM_CORE_DATA")
        self.assertEqual(screaming_snake("HTTPSconnection"), "HTTPS_CONNECTION")

    def test_digits_in_name(self) -> None:
        """Handle digits - split before and after."""
        self.assertEqual(screaming_snake("AccelerationSet4Way"), "ACCELERATION_SET_4_WAY")
        self.assertEqual(screaming_snake("Type2Value"), "TYPE_2_VALUE")

    def test_already_screaming(self) -> None:
        """Already SCREAMING_SNAKE stays the same."""
        self.assertEqual(screaming_snake("MSG_COUNT"), "MSG_COUNT")


class TestSnakeCaseConversion(TestCase):
    """Tests for snake_case filter function."""

    def test_simple_camel_case(self) -> None:
        """Simple CamelCase conversion."""
        self.assertEqual(snake_case("MsgCount"), "msg_count")

    def test_uppercase_suffix(self) -> None:
        """Handle uppercase suffixes like ID."""
        self.assertEqual(snake_case("TemporaryID"), "temporary_id")

    def test_lowercase_input(self) -> None:
        """Handle all-lowercase input."""
        self.assertEqual(snake_case("latitude"), "latitude")

    def test_mixed_case_abbreviation(self) -> None:
        """Handle mixed case with abbreviations."""
        self.assertEqual(snake_case("BSMcoreData"), "bsm_core_data")
        self.assertEqual(snake_case("PathPrediction"), "path_prediction")

    def test_digits_in_name(self) -> None:
        """Handle digits - split before and after."""
        self.assertEqual(snake_case("AccelerationSet4Way"), "acceleration_set_4_way")
        self.assertEqual(snake_case("Type2Value"), "type_2_value")

    def test_already_snake(self) -> None:
        """Already snake_case stays the same."""
        self.assertEqual(snake_case("msg_count"), "msg_count")
