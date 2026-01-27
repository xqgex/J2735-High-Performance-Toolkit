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
"""Tests for BIT STRING internal raw read macro generator.

Tests cover generate_bitstring_internal_raw_read which generates
J2735_INTERNAL_RAW_READ_<TYPE> macros that read all bits from buffer.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code

_TEMPLATE_NAME = "bitstring/bitstring_internal_raw_read.j2"


class TestRawReadGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_internal_raw_read."""

    def test_extensible_type_reads_correct_bits(self) -> None:
        """VehicleEventFlags (22 bits total) should read from correct position."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        self.assertIn("J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS", code)
        self.assertIn("J2735_READ_BITS", code)

    def test_non_extensible_8bit_type(self) -> None:
        """GNSSstatus (8-bit) should have simple read."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "GNSSstatus")
        self.assertIn("J2735_INTERNAL_RAW_READ_GNSS_STATUS", code)
        # Uses symbolic constant for bit count
        self.assertIn("J2735_INTERNAL_MAX_WIRE_BITS_GNSS_STATUS", code)

    def test_non_extensible_12bit_type(self) -> None:
        """AllowedManeuvers (12-bit) should read 12 bits."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "AllowedManeuvers")
        self.assertIn("J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS", code)
        # Uses symbolic constant for bit count
        self.assertIn("J2735_INTERNAL_MAX_WIRE_BITS_ALLOWED_MANEUVERS", code)
