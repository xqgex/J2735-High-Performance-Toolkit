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
"""Tests for SEQUENCE optional field index constant generation.

Tests cover sequence_internal_opt.j2 which generates
J2735_INTERNAL_OPT_{TYPE}_{FIELD} constants for OPTIONAL fields.
"""

from unittest import TestCase

from tools.tests.conftest import (
    SpecLoadingTestBase,
    generate_sequence_code,
    make_extensible_mock_spec,
    make_nested_mock_spec,
    make_optional_mock_spec,
)

_SEQUENCE_OPT_TEMPLATE_NAME = "sequence/sequence_internal_opt.j2"


class TestOptGeneration(TestCase):
    """Tests for sequence_internal_opt.j2 template with mock specs."""

    def test_sequence_with_optional_field(self) -> None:
        """SEQUENCE with 1 optional field generates index constant."""
        spec = make_optional_mock_spec()
        code = generate_sequence_code(_SEQUENCE_OPT_TEMPLATE_NAME, spec, "IntersectionReferenceID")

        self.assertIn("J2735_INTERNAL_OPT_INTERSECTION_REFERENCE_ID_REGION", code)
        self.assertIn("0U", code)
        self.assertIn("optional bitmap bit 0", code)

    def test_sequence_without_optional_returns_empty(self) -> None:
        """SEQUENCE with no optional fields returns empty string."""
        spec = make_nested_mock_spec()
        code = generate_sequence_code(_SEQUENCE_OPT_TEMPLATE_NAME, spec, "PositionalAccuracy")

        self.assertEqual(code, "")

    def test_extensible_without_optional_returns_empty(self) -> None:
        """Extensible SEQUENCE with no optional fields returns empty string."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_code(_SEQUENCE_OPT_TEMPLATE_NAME, spec, "PathPrediction")

        self.assertEqual(code, "")

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        with self.assertRaises(ValueError) as ctx:
            generate_sequence_code(
                _SEQUENCE_OPT_TEMPLATE_NAME, make_optional_mock_spec(), "UnknownType"
            )

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        with self.assertRaises(ValueError) as ctx:
            generate_sequence_code(
                _SEQUENCE_OPT_TEMPLATE_NAME, make_optional_mock_spec(), "RoadRegulatorID"
            )

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestOptWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_intersection_reference_id(self) -> None:
        """Real IntersectionReferenceID has 1 optional field (region)."""
        code = generate_sequence_code(
            _SEQUENCE_OPT_TEMPLATE_NAME, self.spec, "IntersectionReferenceID"
        )

        self.assertIn("J2735_INTERNAL_OPT_INTERSECTION_REFERENCE_ID_REGION", code)
        self.assertIn("0U", code)

    def test_real_bsm_core_data_returns_empty(self) -> None:
        """Real BSMcoreData (no optionals) returns empty string."""
        code = generate_sequence_code(_SEQUENCE_OPT_TEMPLATE_NAME, self.spec, "BSMcoreData")

        self.assertEqual(code, "")

    def test_real_path_prediction_returns_empty(self) -> None:
        """Real PathPrediction (no optionals) returns empty string."""
        code = generate_sequence_code(_SEQUENCE_OPT_TEMPLATE_NAME, self.spec, "PathPrediction")

        self.assertEqual(code, "")
