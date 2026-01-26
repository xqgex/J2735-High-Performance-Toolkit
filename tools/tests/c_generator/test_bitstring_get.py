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
"""Tests for BIT STRING public get-all macro generator.

Tests cover generate_bitstring_get which generates
J2735_<TYPE>_GET public API macros that return all flags.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code

_TEMPLATE_NAME = "bitstring/bitstring_get.j2"


class TestGetGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_get."""

    def test_extensible_type_has_public_get(self) -> None:
        """VehicleEventFlags should have GET macro."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        self.assertIn("J2735_VEHICLE_EVENT_FLAGS_GET", code)
        # Should call internal macros
        self.assertIn("J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS", code)
        self.assertIn("J2735_INTERNAL_GET_ALL_VEHICLE_EVENT_FLAGS", code)

    def test_non_extensible_type_simpler(self) -> None:
        """GNSSstatus should have simpler GET macro."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "GNSSstatus")
        self.assertIn("J2735_GNSS_STATUS_GET", code)
