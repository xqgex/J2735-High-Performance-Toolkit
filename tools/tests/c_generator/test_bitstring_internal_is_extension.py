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
"""Tests for BIT STRING internal is-extension check macro generator.

Tests cover generate_bitstring_internal_is_extension which generates
J2735_INTERNAL_IS_EXTENSION_<TYPE> macros to check extension marker.
"""

from tools.tests.conftest import SpecLoadingTestBase, generate_bitstring_code

_TEMPLATE_NAME = "bitstring/bitstring_internal_is_extension.j2"


class TestIsExtensionGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_internal_is_extension."""

    def test_extensible_type_generates_extension_check(self) -> None:
        """VehicleEventFlags should have extension bit check."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "VehicleEventFlags")
        self.assertIn("J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS", code)
        # Should use named constant for shift amount
        self.assertIn("J2735_INTERNAL_MAX_WIRE_BITS_VEHICLE_EVENT_FLAGS - 1U", code)

    def test_non_extensible_type_generates_always_false(self) -> None:
        """GNSSstatus (non-extensible) should return 0."""
        code = generate_bitstring_code(_TEMPLATE_NAME, self.spec, "GNSSstatus")
        self.assertIn("J2735_INTERNAL_IS_EXTENSION_GNSS_STATUS", code)
        # Non-extensible always returns 0
        self.assertIn("(0U)", code)
