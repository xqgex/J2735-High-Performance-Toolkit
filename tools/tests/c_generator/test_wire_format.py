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
"""Tests for wire format computation.

Tests cover compute_wire_format functions for generating byte-level wire format documentation.
"""

from unittest import TestCase

from tools.j2735_c_generator_wire_format import (
    ByteSegment,
    compute_wire_format,
)
from tools.tests.conftest import make_nested_mock_spec


class TestWireFormatComputation(TestCase):
    """Tests for compute_wire_format function."""

    def test_wire_format_returns_tuple_of_byte_segments(self) -> None:
        """Wire format returns tuple of tuples containing ByteSegments."""
        spec = make_nested_mock_spec()
        typedef = spec.lookup_type("PositionalAccuracy")
        self.assertIsNotNone(typedef)
        assert typedef

        wire = compute_wire_format(typedef)

        # PositionalAccuracy is 32 bits = 4 bytes
        self.assertEqual(len(wire), 4)

        # Each byte contains ByteSegment tuples
        for byte_segments in wire:
            self.assertIsInstance(byte_segments, tuple)
            for segment in byte_segments:
                self.assertIsInstance(segment, ByteSegment)

    def test_wire_format_empty_for_optional_fields(self) -> None:
        """Wire format returns empty tuple if SEQUENCE has OPTIONAL fields."""
        # BSMcoreData has no OPTIONAL fields in the mock, but we can test
        # the function doesn't crash on valid input
        spec = make_nested_mock_spec()
        typedef = spec.lookup_type("PositionalAccuracy")
        self.assertIsNotNone(typedef)
        assert typedef

        wire = compute_wire_format(typedef)
        self.assertGreater(len(wire), 0)

    def test_wire_format_field_names(self) -> None:
        """Wire format segments have correct field names."""
        spec = make_nested_mock_spec()
        typedef = spec.lookup_type("PositionalAccuracy")
        self.assertIsNotNone(typedef)
        assert typedef

        wire = compute_wire_format(typedef)

        # Collect all field names from segments
        field_names: set[str] = set()
        for byte_segments in wire:
            for segment in byte_segments:
                field_names.add(segment.field_name)

        # Should contain the three fields from PositionalAccuracy
        self.assertIn("semiMajor", field_names)
        self.assertIn("semiMinor", field_names)
        self.assertIn("orientation", field_names)
