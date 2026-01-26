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
"""Tests for BIT STRING size macro generator.

Tests cover generate_bitstring_size which generates
J2735_<TYPE>_SIZE macros returning bit count.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code

_TEMPLATE_NAME = "bitstring/bitstring_size.j2"


class TestSizeGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_size."""

    def test_extensible_type_has_conditional_size(self) -> None:
        """VehicleEventFlags should have root and extended sizes."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        self.assertIn("J2735_VEHICLE_EVENT_FLAGS_SIZE", code)
        # Root: 1 + 13 = 14 bits -> 14U (or as documented)
        # Extended: 1 + 8 + 13 = 22 bits -> 22U
        self.assertIn("14U", code)
        self.assertIn("22U", code)

    def test_non_extensible_8bit_type(self) -> None:
        """GNSSstatus (8-bit) should return fixed 8."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "GNSSstatus")
        self.assertIn("J2735_GNSS_STATUS_SIZE", code)
        self.assertIn("8U", code)

    def test_non_extensible_12bit_type(self) -> None:
        """AllowedManeuvers (12-bit) should return fixed 12."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "AllowedManeuvers")
        self.assertIn("J2735_ALLOWED_MANEUVERS_SIZE", code)
        self.assertIn("12U", code)

    def test_small_2bit_type(self) -> None:
        """LaneDirection (2-bit) should return fixed 2."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "LaneDirection")
        self.assertIn("J2735_LANE_DIRECTION_SIZE", code)
        self.assertIn("2U", code)
