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

Tests cover sequence_has_extension.j2 template for extensible SEQUENCE types.
"""

from unittest import TestCase

from tools.tests.conftest import (
    SpecLoadingTestBase,
    generate_sequence_code,
    make_extensible_mock_spec,
    make_nested_mock_spec,
)

_SEQUENCE_HAS_EXTENSION_TEMPLATE_NAME = "sequence/sequence_has_extension.j2"


class TestHasExtensionGeneration(TestCase):
    """Tests for sequence_has_extension.j2 template with mock specs."""

    def test_extensible_sequence_exact_output(self) -> None:
        """Extensible SEQUENCE generates exact expected output."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_code(
            _SEQUENCE_HAS_EXTENSION_TEMPLATE_NAME, spec, "PathPrediction", require_extensible=True
        )

        expected = (
            "/**\n"
            " * @brief Check if PathPrediction has extension additions present.\n"
            " * @param[in] buf Pointer to the PathPrediction encoding.\n"
            " * @return 1 if extensions are present, 0 otherwise.\n"
            " */\n"
            "#define J2735_PATH_PREDICTION_HAS_EXTENSION(buf) J2735_INTERNAL_HAS_EXTENSION(buf)\n"
        )
        self.assertEqual(code, expected)

    def test_non_extensible_returns_empty(self) -> None:
        """Non-extensible SEQUENCE returns empty string."""
        spec = make_nested_mock_spec()
        code = generate_sequence_code(
            _SEQUENCE_HAS_EXTENSION_TEMPLATE_NAME,
            spec,
            "PositionalAccuracy",
            require_extensible=True,
        )

        self.assertEqual(code, "")

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        with self.assertRaises(ValueError) as ctx:
            generate_sequence_code(
                _SEQUENCE_HAS_EXTENSION_TEMPLATE_NAME, make_extensible_mock_spec(), "UnknownType"
            )

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        with self.assertRaises(ValueError) as ctx:
            generate_sequence_code(
                _SEQUENCE_HAS_EXTENSION_TEMPLATE_NAME, make_extensible_mock_spec(), "Confidence"
            )

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestHasExtensionWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_path_prediction(self) -> None:
        """Real PathPrediction from spec."""
        code = generate_sequence_code(
            _SEQUENCE_HAS_EXTENSION_TEMPLATE_NAME,
            self.spec,
            "PathPrediction",
            require_extensible=True,
        )

        expected = (
            "/**\n"
            " * @brief Check if PathPrediction has extension additions present.\n"
            " * @param[in] buf Pointer to the PathPrediction encoding.\n"
            " * @return 1 if extensions are present, 0 otherwise.\n"
            " */\n"
            "#define J2735_PATH_PREDICTION_HAS_EXTENSION(buf) J2735_INTERNAL_HAS_EXTENSION(buf)\n"
        )
        self.assertEqual(code, expected)

    def test_real_bsm_core_data_returns_empty(self) -> None:
        """Real BSMcoreData (non-extensible) returns empty string."""
        code = generate_sequence_code(
            _SEQUENCE_HAS_EXTENSION_TEMPLATE_NAME, self.spec, "BSMcoreData", require_extensible=True
        )

        self.assertEqual(code, "")
