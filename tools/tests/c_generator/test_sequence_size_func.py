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
"""Tests for size function generation.

Tests cover generate_sequence_size_func function for extensible SEQUENCE types.
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
)

_TEMPLATE_NAME = "sequence/sequence_size.j2"


def generate_sequence_size_func(type_name: str, spec: J2735Specification) -> str:
    """Generate C inline function for calculating total size of a SEQUENCE.

    Only generates output for extensible SEQUENCE types with fixed-width
    root components (no OPTIONAL fields).

    Args:
        type_name: Name of the SEQUENCE type (e.g., "PathPrediction").
        spec: The parsed J2735 specification.

    Returns:
        C code with inline function, or empty string if not applicable.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.
    """
    typedef = get_sequence_typedef(type_name, spec)
    assert isinstance(typedef.constraint, SequenceType)  # Guaranteed by getter, required by Mypy
    if not typedef.constraint.is_extensible or typedef.constraint.root_uper_bit_width is None:
        return ""
    return get_template(create_jinja_env(), _TEMPLATE_NAME).render(typedef=typedef)


class TestSizeFuncGeneration(TestCase):
    """Tests for generate_sequence_size_func function."""

    def test_extensible_sequence_function_signature(self) -> None:
        """Extensible SEQUENCE generates correct function signature."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_size_func("PathPrediction", spec)

        self.assertIn("static inline int j2735_inline_path_prediction_size(", code)
        self.assertIn("uint8_t const *const buf", code)
        self.assertIn("uint32_t *const out_size_bits", code)

    def test_extensible_sequence_uses_root_size_constant(self) -> None:
        """Generated function uses ROOT_SIZE_BITS constant."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_size_func("PathPrediction", spec)

        self.assertIn("J2735_INTERNAL_ROOT_SIZE_BITS_PATH_PREDICTION", code)

    def test_extensible_sequence_uses_has_extension_macro(self) -> None:
        """Generated function uses HAS_EXTENSION macro."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_size_func("PathPrediction", spec)

        self.assertIn("J2735_PATH_PREDICTION_HAS_EXTENSION(buf)", code)

    def test_extensible_sequence_calls_skip_extensions(self) -> None:
        """Generated function calls j2735_internal_inline_skip_extensions."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_size_func("PathPrediction", spec)

        self.assertIn("j2735_internal_inline_skip_extensions(", code)

    def test_extensible_sequence_has_doxygen_comment(self) -> None:
        """Generated function has Doxygen documentation."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_size_func("PathPrediction", spec)

        self.assertIn("@brief", code)
        self.assertIn("@param buf", code)
        self.assertIn("@param out_size_bits", code)
        self.assertIn("@return", code)

    def test_non_extensible_returns_empty(self) -> None:
        """Non-extensible SEQUENCE returns empty string."""
        spec = make_nested_mock_spec()
        code = generate_sequence_size_func("PositionalAccuracy", spec)

        self.assertEqual(code, "")

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_size_func("UnknownType", spec)

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_size_func("RadiusOfCurvature", spec)

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestSizeFuncWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_path_prediction_function_name(self) -> None:
        """Real PathPrediction generates correct function name."""
        code = generate_sequence_size_func("PathPrediction", self.spec)

        self.assertIn("j2735_inline_path_prediction_size", code)

    def test_real_path_prediction_root_size(self) -> None:
        """Real PathPrediction uses correct root size constant."""
        code = generate_sequence_size_func("PathPrediction", self.spec)

        # Should reference 25-bit root size
        self.assertIn("J2735_INTERNAL_ROOT_SIZE_BITS_PATH_PREDICTION", code)

    def test_real_bsm_core_data_returns_empty(self) -> None:
        """Real BSMcoreData (non-extensible) returns empty string."""
        code = generate_sequence_size_func("BSMcoreData", self.spec)

        self.assertEqual(code, "")
