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
"""Tests for SEQUENCE prefix bits constant generation.

Tests cover sequence_internal_prefix_bits.j2 which generates
J2735_INTERNAL_PREFIX_BITS_{TYPE} constants (extension bit + optional count).
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

_TEMPLATE_NAME = "sequence/sequence_internal_prefix_bits.j2"


def generate_sequence_prefix_bits(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define constant for prefix bits of a SEQUENCE.

    Args:
        type_name: Name of the SEQUENCE type.
        spec: The parsed J2735 specification.

    Returns:
        C code with #define constant.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.
    """
    typedef = get_sequence_typedef(type_name, spec)
    assert isinstance(typedef.constraint, SequenceType)
    return get_template(create_jinja_env(), _TEMPLATE_NAME).render(typedef=typedef)


class TestPrefixBitsGeneration(TestCase):
    """Tests for generate_sequence_prefix_bits function."""

    def test_non_extensible_no_optional(self) -> None:
        """Non-extensible SEQUENCE with no optionals has 0 prefix bits."""
        spec = make_nested_mock_spec()
        code = generate_sequence_prefix_bits("PositionalAccuracy", spec)

        self.assertIn("J2735_INTERNAL_PREFIX_BITS_POSITIONAL_ACCURACY", code)
        self.assertIn("0U + J2735_INTERNAL_PREAMBLE_BITS(0U)", code)
        self.assertIn("0 ext + 0 opt = 0 bits", code)

    def test_extensible_no_optional(self) -> None:
        """Extensible SEQUENCE with no optionals has 1 prefix bit."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_prefix_bits("PathPrediction", spec)

        self.assertIn("J2735_INTERNAL_PREFIX_BITS_PATH_PREDICTION", code)
        self.assertIn("1U + J2735_INTERNAL_PREAMBLE_BITS(0U)", code)
        self.assertIn("1 ext + 0 opt = 1 bit", code)

    def test_non_extensible_with_optional(self) -> None:
        """Non-extensible SEQUENCE with 1 optional has 1 prefix bit."""
        spec = make_optional_mock_spec()
        code = generate_sequence_prefix_bits("IntersectionReferenceID", spec)

        self.assertIn("J2735_INTERNAL_PREFIX_BITS_INTERSECTION_REFERENCE_ID", code)
        self.assertIn("0U + J2735_INTERNAL_PREAMBLE_BITS(1U)", code)
        self.assertIn("0 ext + 1 opt = 1 bit", code)

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        spec = make_nested_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_prefix_bits("UnknownType", spec)

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_prefix_bits("RadiusOfCurvature", spec)

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestPrefixBitsWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_bsm_core_data(self) -> None:
        """Real BSMcoreData (non-extensible, no optionals) has 0 prefix bits."""
        code = generate_sequence_prefix_bits("BSMcoreData", self.spec)

        self.assertIn("J2735_INTERNAL_PREFIX_BITS_BSM_CORE_DATA", code)
        self.assertIn("0 ext + 0 opt = 0 bits", code)

    def test_real_path_prediction(self) -> None:
        """Real PathPrediction (extensible, no optionals) has 1 prefix bit."""
        code = generate_sequence_prefix_bits("PathPrediction", self.spec)

        self.assertIn("J2735_INTERNAL_PREFIX_BITS_PATH_PREDICTION", code)
        self.assertIn("1 ext + 0 opt = 1 bit", code)

    def test_real_intersection_reference_id(self) -> None:
        """Real IntersectionReferenceID (non-extensible, 1 optional) has 1 prefix bit."""
        code = generate_sequence_prefix_bits("IntersectionReferenceID", self.spec)

        self.assertIn("J2735_INTERNAL_PREFIX_BITS_INTERSECTION_REFERENCE_ID", code)
        self.assertIn("0 ext + 1 opt = 1 bit", code)
