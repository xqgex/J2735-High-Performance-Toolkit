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

Tests cover sequence_size.j2 template for extensible SEQUENCE types.
"""

from unittest import TestCase

from tools.tests.conftest import (
    SpecLoadingTestBase,
    generate_sequence_code,
    make_extensible_mock_spec,
    make_nested_mock_spec,
    make_optional_mock_spec,
)

_SEQUENCE_SIZE_TEMPLATE_NAME = "sequence/sequence_size.j2"


class TestSizeFuncGeneration(TestCase):
    """Tests for sequence_size.j2 template with mock specs."""

    def test_extensible_sequence_function_signature(self) -> None:
        """Extensible SEQUENCE generates correct function signature."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            spec,
            "PathPrediction",
            require_extensible=True,
            require_fixed_root=True,
        )

        self.assertIn("static inline int j2735_inline_path_prediction_size(", code)
        self.assertIn("uint8_t const *const buf", code)
        self.assertIn("uint32_t *const out_size_bits", code)

    def test_extensible_sequence_uses_root_size_constant(self) -> None:
        """Generated function uses ROOT_SIZE_BITS constant."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            spec,
            "PathPrediction",
            require_extensible=True,
            require_fixed_root=True,
        )

        self.assertIn("J2735_INTERNAL_ROOT_SIZE_BITS_PATH_PREDICTION", code)

    def test_extensible_sequence_uses_has_extension_macro(self) -> None:
        """Generated function uses HAS_EXTENSION macro."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            spec,
            "PathPrediction",
            require_extensible=True,
            require_fixed_root=True,
        )

        self.assertIn("J2735_PATH_PREDICTION_HAS_EXTENSION(buf)", code)

    def test_extensible_sequence_calls_skip_extensions(self) -> None:
        """Generated function calls j2735_internal_inline_skip_extensions."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            spec,
            "PathPrediction",
            require_extensible=True,
            require_fixed_root=True,
        )

        self.assertIn("j2735_internal_inline_skip_extensions(", code)

    def test_extensible_sequence_has_doxygen_comment(self) -> None:
        """Generated function has Doxygen documentation."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            spec,
            "PathPrediction",
            require_extensible=True,
            require_fixed_root=True,
        )

        self.assertIn("@brief", code)
        self.assertIn("@param[in]  buf", code)
        self.assertIn("@param[out] out_size_bits", code)
        self.assertIn("@return", code)

    def test_non_extensible_returns_empty(self) -> None:
        """Non-extensible SEQUENCE returns empty string."""
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            make_optional_mock_spec(),
            "IntersectionReferenceID",
            require_extensible=True,
            require_fixed_root=True,
        )

        self.assertEqual(code, "")

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        with self.assertRaises(ValueError) as ctx:
            generate_sequence_code(
                _SEQUENCE_SIZE_TEMPLATE_NAME, make_extensible_mock_spec(), "UnknownType"
            )

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        with self.assertRaises(ValueError) as ctx:
            generate_sequence_code(
                _SEQUENCE_SIZE_TEMPLATE_NAME, make_nested_mock_spec(), "SemiMinorAxisAccuracy"
            )

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestSizeFuncWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_pivot_point_description_function_name(self) -> None:
        """Real PivotPointDescription generates correct function name."""
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            self.spec,
            "PivotPointDescription",
            require_extensible=True,
            require_fixed_root=True,
        )

        self.assertIn("j2735_inline_pivot_point_description_size", code)

    def test_real_pivot_point_description_root_size(self) -> None:
        """Real PivotPointDescription uses correct root size constant."""
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            self.spec,
            "PivotPointDescription",
            require_extensible=True,
            require_fixed_root=True,
        )

        # Should reference 28-bit root size
        self.assertIn("J2735_INTERNAL_ROOT_SIZE_BITS_PIVOT_POINT_DESCRIPTION", code)

    def test_real_positional_accuracy_returns_empty(self) -> None:
        """Real PositionalAccuracy (non-extensible) returns empty string."""
        code = generate_sequence_code(
            _SEQUENCE_SIZE_TEMPLATE_NAME,
            self.spec,
            "PositionalAccuracy",
            require_extensible=True,
            require_fixed_root=True,
        )

        self.assertEqual(code, "")
