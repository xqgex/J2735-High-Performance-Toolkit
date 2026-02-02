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
"""Tests for BIT STRING internal bit position constants generator.

Tests cover generate_bitstring_internal_bit_pos which generates
J2735_INTERNAL_BIT_<TYPE>_<FLAG> constants for named bits.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code

_TEMPLATE_NAME = "bitstring/bitstring_internal_bit_pos.j2"


class TestBitPosGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_internal_bit_pos."""

    def test_extensible_type_includes_all_named_bits(self) -> None:
        """VehicleEventFlags should have 14 bit position constants."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        # Root bits (0-12)
        self.assertIn("J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_HAZARD_LIGHTS", code)
        self.assertIn("0U", code)
        # Extension bit (13)
        self.assertIn("J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_JACK_KNIFE", code)
        self.assertIn("13U", code)

    def test_non_extensible_type_gnss_status(self) -> None:
        """GNSSstatus (8-bit non-extensible) should generate 8 constants."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "GNSSstatus")
        self.assertIn("J2735_INTERNAL_BIT_GNSS_STATUS_", code)
        # Check macro structure
        self.assertIn("#define J2735_INTERNAL_BIT_GNSS_STATUS_", code)

    def test_small_type_lane_direction(self) -> None:
        """LaneDirection (2-bit) should generate 2 constants."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "LaneDirection")
        self.assertIn("J2735_INTERNAL_BIT_LANE_DIRECTION_", code)
        # Should have bits 0 and 1
        self.assertIn("0U", code)
        self.assertIn("1U", code)
