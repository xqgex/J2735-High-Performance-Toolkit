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
"""Tests for BIT STRING generators - cross-cutting concerns.

Tests cover error handling and naming consistency across all BIT STRING
generators to ensure uniform behavior and API.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code


class TestConsistency(SpecLoadingTestBase):
    """Tests for consistency across generator outputs."""

    def test_naming_consistency_extensible(self) -> None:
        """All VehicleEventFlags macros should use consistent naming."""
        type_name = "VehicleEventFlags"
        prefix = "VEHICLE_EVENT_FLAGS"

        # Generate all outputs
        bit_pos = generate_bitstring_code(
            "bitstring/bitstring_internal_bit_pos.j2", self.spec, type_name
        )
        raw_read = generate_bitstring_code(
            "bitstring/bitstring_internal_raw_read.j2", self.spec, type_name
        )
        is_ext = generate_bitstring_code(
            "bitstring/bitstring_internal_is_extension.j2", self.spec, type_name
        )
        get_all = generate_bitstring_code(
            "bitstring/bitstring_internal_get_all.j2", self.spec, type_name
        )
        get_one_int = generate_bitstring_code(
            "bitstring/bitstring_internal_get_one.j2", self.spec, type_name
        )
        is_extended = generate_bitstring_code(
            "bitstring/bitstring_is_extended.j2", self.spec, type_name
        )
        size = generate_bitstring_code("bitstring/bitstring_size.j2", self.spec, type_name)
        get_pub = generate_bitstring_code("bitstring/bitstring_get.j2", self.spec, type_name)
        get_one_pub = generate_bitstring_code(
            "bitstring/bitstring_get_one.j2", self.spec, type_name
        )

        # All should use the same prefix
        self.assertIn(f"J2735_INTERNAL_BIT_{prefix}_", bit_pos)
        self.assertIn(f"J2735_INTERNAL_RAW_READ_{prefix}", raw_read)
        self.assertIn(f"J2735_INTERNAL_IS_EXTENSION_{prefix}", is_ext)
        self.assertIn(f"J2735_INTERNAL_GET_ALL_{prefix}", get_all)
        self.assertIn(f"J2735_INTERNAL_GET_ONE_{prefix}", get_one_int)
        self.assertIn(f"J2735_{prefix}_IS_EXTENDED", is_extended)
        self.assertIn(f"J2735_{prefix}_SIZE", size)
        self.assertIn(f"J2735_{prefix}_GET", get_pub)
        self.assertIn(f"J2735_{prefix}_GET_", get_one_pub)

    def test_naming_consistency_non_extensible(self) -> None:
        """All GNSSstatus macros should use consistent naming."""
        type_name = "GNSSstatus"
        prefix = "GNSS_STATUS"

        bit_pos = generate_bitstring_code(
            "bitstring/bitstring_internal_bit_pos.j2", self.spec, type_name
        )
        size = generate_bitstring_code("bitstring/bitstring_size.j2", self.spec, type_name)
        get_one_pub = generate_bitstring_code(
            "bitstring/bitstring_get_one.j2", self.spec, type_name
        )

        self.assertIn(f"J2735_INTERNAL_BIT_{prefix}_", bit_pos)
        self.assertIn(f"J2735_{prefix}_SIZE", size)
        self.assertIn(f"J2735_{prefix}_GET_", get_one_pub)
