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
"""Tests for ASN1TypeDefinition construction.

Tests cover the from_asn1 factory method for creating type definitions
from ASN.1 strings.
"""

from unittest import TestCase

from tools.j2735_spec_parser import ASN1TypeClass, ASN1TypeDefinition


class TestTypeDefinitionConstruction(TestCase):
    """Tests for ASN1TypeDefinition.from_asn1 classmethod."""

    def test_integer_type(self) -> None:
        """Parse INTEGER type definition."""
        td = ASN1TypeDefinition.from_asn1("Latitude", "INTEGER (-900000000..900000001)")
        self.assertEqual(td.name, "Latitude")
        self.assertEqual(td.type_class, ASN1TypeClass.INTEGER)
        self.assertEqual(td.uper_bit_width, 31)

    def test_boolean_type(self) -> None:
        """BOOLEAN is always 1 bit."""
        td = ASN1TypeDefinition.from_asn1("IsActive", "BOOLEAN")
        self.assertEqual(td.type_class, ASN1TypeClass.BOOLEAN)
        self.assertEqual(td.uper_bit_width, 1)
