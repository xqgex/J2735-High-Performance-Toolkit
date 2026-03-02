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
        self.assertIn("J2735_READ_BITS", code)

    def test_non_extensible_12bit_type(self) -> None:
        """AllowedManeuvers (12-bit) should read 12 bits."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "AllowedManeuvers")
        self.assertIn("J2735_INTERNAL_RAW_READ_ALLOWED_MANEUVERS", code)
        self.assertIn("J2735_READ_BITS", code)

    def test_non_extensible_must_not_reference_max_wire_bits(self) -> None:
        """Non-extensible RAW_READ must not reference MAX_WIRE_BITS.

        Non-extensible BIT STRING types have no extension bit and no nsnnwn field.
        MAX_WIRE_BITS is only defined for extensible types (by
        bitstring_internal_max_wire_bits.j2, conditionally included). Referencing it
        from a non-extensible header produces an undefined symbol at compile time.
        """
        for type_name, prefix in [
            ("LaneDirection", "LANE_DIRECTION"),
            ("GNSSstatus", "GNSS_STATUS"),
            ("AllowedManeuvers", "ALLOWED_MANEUVERS"),
            ("BrakeAppliedStatus", "BRAKE_APPLIED_STATUS"),
            ("TransitStatus", "TRANSIT_STATUS"),
            ("LaneSharing", "LANE_SHARING"),
            ("VerticalAccelerationThreshold", "VERTICAL_ACCELERATION_THRESHOLD"),
        ]:
            with self.subTest(type_name=type_name):
                code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, type_name)
                self.assertNotIn(
                    f"J2735_INTERNAL_MAX_WIRE_BITS_{prefix}",
                    code,
                    f"{type_name} RAW_READ must not reference undefined MAX_WIRE_BITS",
                )

    def test_non_extensible_raw_read_uses_root_size(self) -> None:
        """Non-extensible RAW_READ should read ROOT_SIZE bits, not MAX_WIRE_BITS."""
        for type_name, prefix in [
            ("LaneDirection", "LANE_DIRECTION"),
            ("GNSSstatus", "GNSS_STATUS"),
            ("AllowedManeuvers", "ALLOWED_MANEUVERS"),
        ]:
            with self.subTest(type_name=type_name):
                code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, type_name)
                self.assertIn(
                    f"J2735_INTERNAL_ROOT_SIZE_{prefix}",
                    code,
                    f"{type_name} RAW_READ should use ROOT_SIZE as bit count",
                )

    def test_extensible_raw_read_uses_max_wire_bits(self) -> None:
        """Extensible RAW_READ should still use MAX_WIRE_BITS."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        self.assertIn(
            "J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS",
            code,
            "Extensible RAW_READ must use MAX_WIRE_BITS",
        )
