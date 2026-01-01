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

from collections.abc import Callable

from tools.j2735_c_generator_bitstring import generate_bitstring_get
from tools.j2735_c_generator_bitstring import generate_bitstring_get_one
from tools.j2735_c_generator_bitstring import generate_bitstring_internal_bit_pos
from tools.j2735_c_generator_bitstring import generate_bitstring_internal_get_all
from tools.j2735_c_generator_bitstring import generate_bitstring_internal_get_one
from tools.j2735_c_generator_bitstring import generate_bitstring_internal_is_extension
from tools.j2735_c_generator_bitstring import generate_bitstring_internal_raw_read
from tools.j2735_c_generator_bitstring import generate_bitstring_is_extended
from tools.j2735_c_generator_bitstring import generate_bitstring_size
from tools.j2735_spec_parser import J2735Specification
from tools.tests.conftest import SpecLoadingTestBase


class TestErrorHandling(SpecLoadingTestBase):
    """Tests for error handling across all generators."""

    def _all_generators(
        self,
    ) -> list[Callable[[str, J2735Specification], str]]:
        """Return all generator functions."""
        return [
            generate_bitstring_internal_bit_pos,
            generate_bitstring_internal_raw_read,
            generate_bitstring_internal_is_extension,
            generate_bitstring_internal_get_all,
            generate_bitstring_internal_get_one,
            generate_bitstring_is_extended,
            generate_bitstring_size,
            generate_bitstring_get,
            generate_bitstring_get_one,
        ]

    def test_all_generators_reject_unknown_type(self) -> None:
        """All generators should raise ValueError for unknown type."""
        for gen in self._all_generators():
            with self.assertRaises(ValueError, msg=f"{gen.__name__} should reject unknown type"):
                gen("NonExistentType", self.spec)

    def test_all_generators_reject_non_bitstring(self) -> None:
        """All generators should raise ValueError for non-BIT_STRING type."""
        for gen in self._all_generators():
            with self.assertRaises(ValueError, msg=f"{gen.__name__} should reject INTEGER"):
                gen("MsgCount", self.spec)


class TestConsistency(SpecLoadingTestBase):
    """Tests for consistency across generator outputs."""

    def test_naming_consistency_extensible(self) -> None:
        """All VehicleEventFlags macros should use consistent naming."""
        type_name = "VehicleEventFlags"
        prefix = "VEHICLE_EVENT_FLAGS"

        # Generate all outputs
        bit_pos = generate_bitstring_internal_bit_pos(type_name, self.spec)
        raw_read = generate_bitstring_internal_raw_read(type_name, self.spec)
        is_ext = generate_bitstring_internal_is_extension(type_name, self.spec)
        get_all = generate_bitstring_internal_get_all(type_name, self.spec)
        get_one_int = generate_bitstring_internal_get_one(type_name, self.spec)
        is_extended = generate_bitstring_is_extended(type_name, self.spec)
        size = generate_bitstring_size(type_name, self.spec)
        get_pub = generate_bitstring_get(type_name, self.spec)
        get_one_pub = generate_bitstring_get_one(type_name, self.spec)

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

        bit_pos = generate_bitstring_internal_bit_pos(type_name, self.spec)
        size = generate_bitstring_size(type_name, self.spec)
        get_one_pub = generate_bitstring_get_one(type_name, self.spec)

        self.assertIn(f"J2735_INTERNAL_BIT_{prefix}_", bit_pos)
        self.assertIn(f"J2735_{prefix}_SIZE", size)
        self.assertIn(f"J2735_{prefix}_GET_", get_one_pub)
