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
"""Tests for BIT STRING internal root size constants generator.

Tests cover bitstring_internal_root_size.j2 which generates
J2735_INTERNAL_ROOT_SIZE_{TYPE} constants for root bit count.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code

_TEMPLATE_NAME = "bitstring/bitstring_internal_root_size.j2"


class TestRootSizeGenerator(SpecLoadingTestBase):
    """Tests for bitstring_internal_root_size template."""

    def test_extensible_type_has_root_size_13(self) -> None:
        """VehicleEventFlags should have root_size=13."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        self.assertIn("J2735_INTERNAL_ROOT_SIZE_VEHICLE_EVENT_FLAGS", code)
        self.assertIn("13U", code)

    def test_non_extensible_8bit_type(self) -> None:
        """GNSSstatus (8-bit) should have root_size=8."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "GNSSstatus")
        self.assertIn("J2735_INTERNAL_ROOT_SIZE_GNSS_STATUS", code)
        self.assertIn("8U", code)

    def test_non_extensible_12bit_type(self) -> None:
        """AllowedManeuvers (12-bit) should have root_size=12."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "AllowedManeuvers")
        self.assertIn("J2735_INTERNAL_ROOT_SIZE_ALLOWED_MANEUVERS", code)
        self.assertIn("12U", code)

    def test_small_2bit_type(self) -> None:
        """LaneDirection (2-bit) should have root_size=2."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "LaneDirection")
        self.assertIn("J2735_INTERNAL_ROOT_SIZE_LANE_DIRECTION", code)
        self.assertIn("2U", code)
