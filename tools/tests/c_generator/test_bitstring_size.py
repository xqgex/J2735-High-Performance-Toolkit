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

from tools.tests.conftest import (
    NON_EXTENSIBLE_BITSTRING_TYPES,
    SpecLoadingTestBase,
    generate_bitstring_code,
)

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

    def test_non_extensible_size_must_not_include_extension_marker_bits(self) -> None:
        """Non-extensible SIZE must not add J2735_INTERNAL_EXTENSION_MARKER_BITS.

        Non-extensible BIT STRING has no extension marker on the wire.
        SIZE should be exactly ROOT_SIZE_BITS, not EXTENSION_MARKER_BITS + ROOT_SIZE_BITS.
        Example: LaneDirection SIZE should be 2, not 1+2=3.
        """
        for type_name, prefix in NON_EXTENSIBLE_BITSTRING_TYPES:
            with self.subTest(type_name=type_name):
                code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, type_name)
                # Extract the #define line for SIZE
                size_macro_name = f"J2735_{prefix}_SIZE"
                self.assertIn(size_macro_name, code)
                self.assertNotIn(
                    "J2735_INTERNAL_EXTENSION_MARKER_BITS",
                    code,
                    f"{type_name} SIZE must not reference EXTENSION_MARKER_BITS "
                    f"(non-extensible types have no extension marker on wire)",
                )

    def test_non_extensible_size_evaluates_to_bw_constant(self) -> None:
        """Non-extensible SIZE #define should expand to J2735_BW_*.

        The #define line for SIZE must reference J2735_BW_* and nothing else.
        """
        for type_name, prefix in NON_EXTENSIBLE_BITSTRING_TYPES:
            with self.subTest(type_name=type_name):
                code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, type_name)
                # Find the #define SIZE line specifically
                size_defines = [
                    line
                    for line in code.splitlines()
                    if line.strip().startswith("#define") and f"J2735_{prefix}_SIZE" in line
                ]
                self.assertTrue(
                    size_defines,
                    f"{type_name}: SIZE #define not found",
                )
                for line in size_defines:
                    self.assertIn(
                        f"J2735_BW_{prefix}",
                        line,
                        f"{type_name} SIZE #define should reference J2735_BW_*: {line}",
                    )
                    self.assertNotIn(
                        "EXTENSION_MARKER_BITS",
                        line,
                        f"{type_name} SIZE #define must not reference EXTENSION_MARKER_BITS: "
                        f"{line}",
                    )
