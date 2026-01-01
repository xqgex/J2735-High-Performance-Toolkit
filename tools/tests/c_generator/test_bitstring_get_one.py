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
"""Tests for BIT STRING public get-one macro generator.

Tests cover generate_bitstring_get_one which generates
J2735_<TYPE>_GET_<FLAG> macros for each named bit.
"""

from tools.j2735_c_generator_bitstring import generate_bitstring_get_one
from tools.tests.conftest import SpecLoadingTestBase


class TestGetOneGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_get_one."""

    def test_extensible_type_all_flags(self) -> None:
        """VehicleEventFlags should have macro for each named bit."""
        code = generate_bitstring_get_one("VehicleEventFlags", self.spec)

        # Root flag
        self.assertIn("J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS", code)
        # Extension flag
        self.assertIn("J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_JACK_KNIFE", code)
        # Doxygen for extension flag
        self.assertIn("extended form only", code)

    def test_extension_flag_has_warning(self) -> None:
        """Extension flags should have warning in Doxygen."""
        code = generate_bitstring_get_one("VehicleEventFlags", self.spec)

        # jackKnife is bit 13 (extension)
        jack_knife_idx = code.find("eventJackKnife")
        self.assertGreater(jack_knife_idx, 0)
        # Warning should appear before the macro
        warning_idx = code.find("@warning", jack_knife_idx - 500)
        self.assertGreater(warning_idx, 0)

    def test_non_extensible_type_no_extension_warning(self) -> None:
        """GNSSstatus flags should NOT have extension warnings."""
        code = generate_bitstring_get_one("GNSSstatus", self.spec)

        self.assertIn("J2735_GNSS_STATUS_GET_", code)
        # Non-extensible should not have extension warnings
        self.assertNotIn("extended form only", code)

    def test_small_type_lane_direction(self) -> None:
        """LaneDirection (2-bit) should have 2 GET macros."""
        code = generate_bitstring_get_one("LaneDirection", self.spec)

        self.assertIn("J2735_LANE_DIRECTION_GET_", code)
