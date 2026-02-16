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
"""Tests for SEQUENCE field getter macro generation.

Tests cover sequence_get.j2 which generates
J2735_{TYPE}_GET_{FIELD}(buf) macros for field access.
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

_TEMPLATE_NAME = "sequence/sequence_get.j2"


def generate_sequence_get(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define macros for field getters of a SEQUENCE.

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


class TestGetGeneration(TestCase):
    """Tests for generate_sequence_get function."""

    def test_unsigned_field(self) -> None:
        """Unsigned field uses cast to uint type."""
        spec = make_nested_mock_spec()
        code = generate_sequence_get("PositionalAccuracy", spec)

        self.assertIn("J2735_POSITIONAL_ACCURACY_GET_SEMI_MAJOR(buf)", code)
        self.assertIn("(uint8_t)", code)
        self.assertIn("J2735_READ_BITS", code)

    def test_signed_field(self) -> None:
        """Signed field uses SIGN_EXTEND macro."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_get("PathPrediction", spec)

        self.assertIn("J2735_PATH_PREDICTION_GET_RADIUS_OF_CURVE(buf)", code)
        self.assertIn("J2735_INTERNAL_SIGN_EXTEND", code)
        self.assertIn("int16_t", code)

    def test_optional_field_has_precondition(self) -> None:
        """Optional field has @pre comment for HAS macro."""
        spec = make_optional_mock_spec()
        code = generate_sequence_get("IntersectionReferenceID", spec)

        self.assertIn("J2735_INTERSECTION_REFERENCE_ID_GET_REGION(buf)", code)
        self.assertIn("@pre J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf)", code)

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        spec = make_nested_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_get("UnknownType", spec)

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_get("RadiusOfCurvature", spec)

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestGetWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_bsm_core_data_unsigned(self) -> None:
        """Real BSMcoreData msgCnt is unsigned."""
        code = generate_sequence_get("BSMcoreData", self.spec)

        self.assertIn("J2735_BSM_CORE_DATA_GET_MSG_CNT(buf)", code)
        self.assertIn("(uint8_t)", code)

    def test_real_bsm_core_data_signed(self) -> None:
        """Real BSMcoreData lat is signed (uses SIGN_EXTEND)."""
        code = generate_sequence_get("BSMcoreData", self.spec)

        self.assertIn("J2735_BSM_CORE_DATA_GET_LAT(buf)", code)
        self.assertIn("J2735_INTERNAL_SIGN_EXTEND", code)

    def test_real_intersection_reference_id_optional(self) -> None:
        """Real IntersectionReferenceID region has @pre comment."""
        code = generate_sequence_get("IntersectionReferenceID", self.spec)

        self.assertIn("J2735_INTERSECTION_REFERENCE_ID_GET_REGION(buf)", code)
        self.assertIn("@pre J2735_INTERSECTION_REFERENCE_ID_HAS_REGION(buf)", code)

    def test_real_path_prediction_signed(self) -> None:
        """Real PathPrediction radiusOfCurve is signed."""
        code = generate_sequence_get("PathPrediction", self.spec)

        self.assertIn("J2735_PATH_PREDICTION_GET_RADIUS_OF_CURVE(buf)", code)
        self.assertIn("J2735_INTERNAL_SIGN_EXTEND", code)
