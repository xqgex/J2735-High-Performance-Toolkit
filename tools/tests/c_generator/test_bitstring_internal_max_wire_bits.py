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
"""Tests for BIT STRING internal max wire bits constants generator.

Tests cover bitstring_internal_max_wire_bits.j2 which generates
J2735_INTERNAL_MAX_WIRE_BITS_{TYPE} constants for maximum wire encoding size.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code

_TEMPLATE_NAME = "bitstring/bitstring_internal_max_wire_bits.j2"


class TestMaxWireBitsGenerator(SpecLoadingTestBase):
    """Tests for bitstring_internal_max_wire_bits template."""

    def test_extensible_type_has_max_wire_bits_22(self) -> None:
        """VehicleEventFlags should have read_bits=22 (1+7+14)."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        self.assertIn("J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS", code)
        self.assertIn("22U", code)

    def test_non_extensible_8bit_type(self) -> None:
        """GNSSstatus (8-bit) should have read_bits=8."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "GNSSstatus")
        self.assertIn("J2735_INTERNAL_MAX_WIRE_BITS_GNSS_STATUS", code)
        self.assertIn("8U", code)

    def test_non_extensible_12bit_type(self) -> None:
        """AllowedManeuvers (12-bit) should have read_bits=12."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "AllowedManeuvers")
        self.assertIn("J2735_INTERNAL_MAX_WIRE_BITS_ALLOWED_MANEUVERS", code)
        self.assertIn("12U", code)

    def test_small_2bit_type(self) -> None:
        """LaneDirection (2-bit) should have read_bits=2."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "LaneDirection")
        self.assertIn("J2735_INTERNAL_MAX_WIRE_BITS_LANE_DIRECTION", code)
        self.assertIn("2U", code)

    def test_invalid_type_raises_value_error(self) -> None:
        """Non-existent type should raise ValueError."""
        with self.assertRaises(ValueError) as ctx:
            _ = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "NonExistentType")
        self.assertIn("not found", str(ctx.exception))

    def test_non_bitstring_type_raises_value_error(self) -> None:
        """INTEGER type should raise ValueError."""
        with self.assertRaises(ValueError) as ctx:
            _ = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "MsgCount")
        self.assertIn("not a BIT STRING", str(ctx.exception))
