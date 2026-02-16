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
"""Tests for SEQUENCE optional field width macro generation.

Tests cover sequence_internal_width.j2 which generates
J2735_INTERNAL_WIDTH_{TYPE}_{FIELD}(buf) macros for OPTIONAL fields.
"""

from unittest import TestCase

from tools.j2735_c_generator_jinja import (
    create_jinja_env,
    get_template,
)
from tools.j2735_spec_constraints import SequenceType
from tools.j2735_spec_parser import J2735Specification
from tools.tests.conftest import (
    SpecLoadingTestBase,
    get_sequence_typedef,
    make_extensible_mock_spec,
    make_nested_mock_spec,
    make_optional_mock_spec,
)

_TEMPLATE_NAME = "sequence/sequence_internal_width.j2"


def generate_sequence_width(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define macros for optional field widths of a SEQUENCE.

    Args:
        type_name: Name of the SEQUENCE type.
        spec: The parsed J2735 specification.

    Returns:
        C code with #define macros, or empty string if no optionals.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.
    """
    typedef = get_sequence_typedef(type_name, spec)
    assert isinstance(typedef.constraint, SequenceType)
    return get_template(create_jinja_env(), _TEMPLATE_NAME).render(typedef=typedef)


class TestWidthGeneration(TestCase):
    """Tests for generate_sequence_width function."""

    def test_sequence_with_optional_field(self) -> None:
        """SEQUENCE with 1 optional field generates width macro."""
        spec = make_optional_mock_spec()
        code = generate_sequence_width("IntersectionReferenceID", spec)

        self.assertIn("J2735_INTERNAL_WIDTH_INTERSECTION_REFERENCE_ID_REGION", code)
        self.assertIn("J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf)", code)
        self.assertIn("J2735_BW_ROAD_REGULATOR_ID", code)
        self.assertIn(": 0U)", code)

    def test_sequence_without_optional_returns_empty(self) -> None:
        """SEQUENCE with no optional fields returns empty string."""
        spec = make_nested_mock_spec()
        code = generate_sequence_width("PositionalAccuracy", spec)

        self.assertEqual(code, "")

    def test_extensible_without_optional_returns_empty(self) -> None:
        """Extensible SEQUENCE with no optional fields returns empty string."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_width("PathPrediction", spec)

        self.assertEqual(code, "")

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        spec = make_optional_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_width("UnknownType", spec)

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        spec = make_optional_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_width("RoadRegulatorID", spec)

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestWidthWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_intersection_reference_id(self) -> None:
        """Real IntersectionReferenceID has width macro for region."""
        code = generate_sequence_width("IntersectionReferenceID", self.spec)

        self.assertIn("J2735_INTERNAL_WIDTH_INTERSECTION_REFERENCE_ID_REGION", code)
        self.assertIn("J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf)", code)
        self.assertIn("J2735_BW_ROAD_REGULATOR_ID", code)

    def test_real_bsm_core_data_returns_empty(self) -> None:
        """Real BSMcoreData (no optionals) returns empty string."""
        code = generate_sequence_width("BSMcoreData", self.spec)

        self.assertEqual(code, "")

    def test_real_path_prediction_returns_empty(self) -> None:
        """Real PathPrediction (no optionals) returns empty string."""
        code = generate_sequence_width("PathPrediction", self.spec)

        self.assertEqual(code, "")
