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
"""Tests for has-extension macro generation.

Tests cover generate_sequence_has_extension function for extensible SEQUENCE types.
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

_TEMPLATE_NAME = "sequence/sequence_has_extension.j2"


def generate_sequence_has_extension(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define macro for checking extension bit in a SEQUENCE.

    Only generates output for extensible SEQUENCE types.

    Args:
        type_name: Name of the SEQUENCE type (e.g., "PathPrediction").
        spec: The parsed J2735 specification.

    Returns:
        C code with #define macro, or empty string if not extensible.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.
    """
    typedef = get_sequence_typedef(type_name, spec)
    assert isinstance(typedef.constraint, SequenceType)  # Guaranteed by getter, required by Mypy
    if not typedef.constraint.is_extensible:
        return ""
    return get_template(create_jinja_env(), _TEMPLATE_NAME).render(typedef=typedef)


class TestHasExtensionGeneration(TestCase):
    """Tests for generate_sequence_has_extension function."""

    def test_extensible_sequence_exact_output(self) -> None:
        """Extensible SEQUENCE generates exact expected output."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_has_extension("PathPrediction", spec)

        expected = (
            "/**\n"
            " * @brief Check if PathPrediction has extension additions present.\n"
            " * @param buf Pointer to the PathPrediction encoding.\n"
            " * @return 1 if extensions are present, 0 otherwise.\n"
            " */\n"
            "#define J2735_PATH_PREDICTION_HAS_EXTENSION(buf) J2735_INTERNAL_HAS_EXTENSION(buf)\n"
        )
        self.assertEqual(code, expected)

    def test_non_extensible_returns_empty(self) -> None:
        """Non-extensible SEQUENCE returns empty string."""
        spec = make_nested_mock_spec()
        code = generate_sequence_has_extension("PositionalAccuracy", spec)

        self.assertEqual(code, "")

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_has_extension("UnknownType", spec)

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_has_extension("RadiusOfCurvature", spec)

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestHasExtensionWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_path_prediction(self) -> None:
        """Real PathPrediction from spec."""
        code = generate_sequence_has_extension("PathPrediction", self.spec)

        expected = (
            "/**\n"
            " * @brief Check if PathPrediction has extension additions present.\n"
            " * @param buf Pointer to the PathPrediction encoding.\n"
            " * @return 1 if extensions are present, 0 otherwise.\n"
            " */\n"
            "#define J2735_PATH_PREDICTION_HAS_EXTENSION(buf) J2735_INTERNAL_HAS_EXTENSION(buf)\n"
        )
        self.assertEqual(code, expected)

    def test_real_bsm_core_data_returns_empty(self) -> None:
        """Real BSMcoreData (non-extensible) returns empty string."""
        code = generate_sequence_has_extension("BSMcoreData", self.spec)

        self.assertEqual(code, "")
