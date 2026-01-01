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
"""Tests for BIT STRING public is-extended macro generator.

Tests cover generate_bitstring_is_extended which generates
J2735_<TYPE>_IS_EXTENDED public API macros.
"""

from tools.j2735_c_generator_bitstring import generate_bitstring_is_extended
from tools.tests.conftest import SpecLoadingTestBase


class TestIsExtendedGenerator(SpecLoadingTestBase):
    """Tests for generate_bitstring_is_extended."""

    def test_extensible_type_has_public_api(self) -> None:
        """VehicleEventFlags should have IS_EXTENDED macro."""
        code = generate_bitstring_is_extended("VehicleEventFlags", self.spec)

        self.assertIn("J2735_VEHICLE_EVENT_FLAGS_IS_EXTENDED", code)
        # Should call internal macro
        self.assertIn("J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS", code)

    def test_non_extensible_type_always_false(self) -> None:
        """GNSSstatus should always return 0 (never extended)."""
        code = generate_bitstring_is_extended("GNSSstatus", self.spec)

        self.assertIn("J2735_GNSS_STATUS_IS_EXTENDED", code)
