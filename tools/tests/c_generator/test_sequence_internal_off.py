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
"""Tests for SEQUENCE field offset macro generation.

Tests cover sequence_internal_off.j2 which generates
J2735_INTERNAL_OFF_{TYPE}_{FIELD}(buf) macros for field bit offsets.
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

_TEMPLATE_NAME = "sequence/sequence_internal_off.j2"


def generate_sequence_off(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define macros for field offsets of a SEQUENCE.

    Args:
        type_name: Name of the SEQUENCE type.
        spec: The parsed J2735 specification.

    Returns:
        C code with #define macros for each field.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.
    """
    typedef = get_sequence_typedef(type_name, spec)
    assert isinstance(typedef.constraint, SequenceType)
    return get_template(create_jinja_env(), _TEMPLATE_NAME).render(typedef=typedef)


class TestOffGeneration(TestCase):
    """Tests for generate_sequence_off function."""

    def test_fixed_layout_sequence(self) -> None:
        """SEQUENCE with all required fields uses BW constants."""
        spec = make_nested_mock_spec()
        code = generate_sequence_off("PositionalAccuracy", spec)

        # First field uses PREFIX_BITS
        self.assertIn("J2735_INTERNAL_OFF_POSITIONAL_ACCURACY_SEMI_MAJOR(buf)", code)
        self.assertIn("J2735_INTERNAL_PREFIX_BITS_POSITIONAL_ACCURACY", code)
        # Subsequent fields chain with BW constants
        self.assertIn("J2735_INTERNAL_OFF_POSITIONAL_ACCURACY_SEMI_MINOR(buf)", code)
        self.assertIn("J2735_BW_SEMI_MAJOR_AXIS_ACCURACY", code)

    def test_extensible_sequence(self) -> None:
        """Extensible SEQUENCE uses PREFIX_BITS and BW constants."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_off("PathPrediction", spec)

        self.assertIn("J2735_INTERNAL_OFF_PATH_PREDICTION_RADIUS_OF_CURVE(buf)", code)
        self.assertIn("J2735_INTERNAL_PREFIX_BITS_PATH_PREDICTION", code)
        self.assertIn("J2735_INTERNAL_OFF_PATH_PREDICTION_CONFIDENCE(buf)", code)
        self.assertIn("J2735_BW_RADIUS_OF_CURVATURE", code)

    def test_sequence_with_optional_field(self) -> None:
        """SEQUENCE with optional field uses WIDTH macro for offset chaining."""
        spec = make_optional_mock_spec()
        code = generate_sequence_off("IntersectionReferenceID", spec)

        # First field (optional) uses PREFIX_BITS
        self.assertIn("J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_REGION(buf)", code)
        self.assertIn("J2735_INTERNAL_PREFIX_BITS_INTERSECTION_REFERENCE_ID", code)
        # Second field chains using WIDTH (not BW) because previous is optional
        self.assertIn("J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_ID(buf)", code)
        self.assertIn("J2735_INTERNAL_WIDTH_INTERSECTION_REFERENCE_ID_REGION(buf)", code)

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        spec = make_nested_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_off("UnknownType", spec)

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_off("RadiusOfCurvature", spec)

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestOffWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_bsm_core_data(self) -> None:
        """Real BSMcoreData has offset macros for all 14 fields."""
        code = generate_sequence_off("BSMcoreData", self.spec)

        self.assertIn("J2735_INTERNAL_OFF_BSM_CORE_DATA_MSG_CNT(buf)", code)
        self.assertIn("J2735_INTERNAL_OFF_BSM_CORE_DATA_ID(buf)", code)
        self.assertIn("J2735_INTERNAL_OFF_BSM_CORE_DATA_LAT(buf)", code)
        self.assertIn("J2735_BW_MSG_COUNT", code)

    def test_real_intersection_reference_id(self) -> None:
        """Real IntersectionReferenceID uses WIDTH for optional chaining."""
        code = generate_sequence_off("IntersectionReferenceID", self.spec)

        self.assertIn("J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_REGION(buf)", code)
        self.assertIn("J2735_INTERNAL_OFF_INTERSECTION_REFERENCE_ID_ID(buf)", code)
        self.assertIn("J2735_INTERNAL_WIDTH_INTERSECTION_REFERENCE_ID_REGION(buf)", code)
