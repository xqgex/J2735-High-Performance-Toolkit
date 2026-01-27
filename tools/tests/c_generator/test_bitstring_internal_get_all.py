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
"""Tests for BIT STRING internal get-all macro generator.

Tests cover generate_bitstring_internal_get_all which generates
J2735_INTERNAL_GET_ALL_<TYPE> macros that extract all flag bits.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code

_TEMPLATE_NAME = "bitstring/bitstring_internal_get_all.j2"


class TestInternalGetAllGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_internal_get_all."""

    def test_extensible_type_has_shift_logic(self) -> None:
        """VehicleEventFlags should mask and shift correctly."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        self.assertIn("J2735_INTERNAL_GET_ALL_VEHICLE_EVENT_FLAGS", code)
        # Should have shift for root flags
        self.assertIn(">>", code)

    def test_non_extensible_type_no_extension_handling(self) -> None:
        """GNSSstatus should have simple mask without extension logic."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "GNSSstatus")
        self.assertIn("J2735_INTERNAL_GET_ALL_GNSS_STATUS", code)
