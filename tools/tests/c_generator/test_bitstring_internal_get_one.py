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
"""Tests for BIT STRING internal get-one macro generator.

Tests cover generate_bitstring_internal_get_one which generates
J2735_INTERNAL_GET_ONE_<TYPE> macros that extract a single bit.
"""

from tools.j2735_c_generator_bitstring import generate_bitstring_internal_get_one
from tools.tests.conftest import SpecLoadingTestBase


class TestInternalGetOneGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_internal_get_one."""

    def test_extensible_type_bit_extraction(self) -> None:
        """VehicleEventFlags should extract single bit."""
        code = generate_bitstring_internal_get_one("VehicleEventFlags", self.spec)

        self.assertIn("J2735_INTERNAL_GET_ONE_VEHICLE_EVENT_FLAGS", code)
        # Should have bit extraction logic
        self.assertIn(">>", code)
        self.assertIn("& 1U", code)

    def test_non_extensible_type_simpler_logic(self) -> None:
        """GNSSstatus should also extract single bit."""
        code = generate_bitstring_internal_get_one("GNSSstatus", self.spec)

        self.assertIn("J2735_INTERNAL_GET_ONE_GNSS_STATUS", code)
        self.assertIn("& 1U", code)
