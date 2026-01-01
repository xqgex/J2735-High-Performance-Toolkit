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
"""Tests for root size constant generation.

Tests cover generate_sequence_root_size function for extensible SEQUENCE types.
"""

from unittest import TestCase

from tools.j2735_c_generator_sequence import generate_sequence_root_size
from tools.tests.conftest import (
    SpecLoadingTestBase,
    make_extensible_mock_spec,
    make_nested_mock_spec,
)


class TestRootSizeGeneration(TestCase):
    """Tests for generate_sequence_root_size function."""

    def test_extensible_sequence_exact_output(self) -> None:
        """Extensible SEQUENCE generates expected macro."""
        spec = make_extensible_mock_spec()
        code = generate_sequence_root_size("PathPrediction", spec)

        # PathPrediction: 1 preamble + 16 radius + 8 confidence = 25 bits
        # Uses symbolic expression for clarity (clang-format handles line breaking)
        self.assertIn("#define J2735_ROOT_SIZE_BITS_PATH_PREDICTION", code)
        self.assertIn("J2735_PREFIX_BITS_PATH_PREDICTION", code)
        self.assertIn("J2735_BW_RADIUS_OF_CURVATURE", code)
        self.assertIn("J2735_BW_CONFIDENCE", code)
        self.assertIn("25 bits", code)

    def test_non_extensible_returns_empty(self) -> None:
        """Non-extensible SEQUENCE returns empty string."""
        spec = make_nested_mock_spec()
        code = generate_sequence_root_size("PositionalAccuracy", spec)

        self.assertEqual(code, "")

    def test_not_found_raises(self) -> None:
        """Unknown type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_root_size("UnknownType", spec)

        self.assertIn("not found", str(ctx.exception))

    def test_non_sequence_raises(self) -> None:
        """Non-SEQUENCE type raises ValueError."""
        spec = make_extensible_mock_spec()

        with self.assertRaises(ValueError) as ctx:
            generate_sequence_root_size("RadiusOfCurvature", spec)

        self.assertIn("not a SEQUENCE", str(ctx.exception))


class TestRootSizeWithRealSpec(SpecLoadingTestBase):
    """Tests using the real J2735 specification file."""

    def test_real_path_prediction(self) -> None:
        """Real PathPrediction from spec."""
        code = generate_sequence_root_size("PathPrediction", self.spec)

        # Uses symbolic expression for clarity (clang-format handles line breaking)
        self.assertIn("#define J2735_ROOT_SIZE_BITS_PATH_PREDICTION", code)
        self.assertIn("J2735_PREFIX_BITS_PATH_PREDICTION", code)
        self.assertIn("J2735_BW_RADIUS_OF_CURVATURE", code)
        self.assertIn("J2735_BW_CONFIDENCE", code)
        self.assertIn("25 bits", code)

    def test_real_bsm_core_data_returns_empty(self) -> None:
        """Real BSMcoreData (non-extensible) returns empty string."""
        code = generate_sequence_root_size("BSMcoreData", self.spec)

        self.assertEqual(code, "")
