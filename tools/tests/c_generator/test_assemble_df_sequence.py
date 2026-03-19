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
"""Tests for SEQUENCE type assembly via generate_data_frame()."""

from typing import ClassVar
from unittest import TestCase

from tools.j2735_c_generator_data_frame import generate_data_frame
from tools.j2735_spec_constraints import SequenceType
from tools.j2735_spec_parser import J2735Specification, parse_spec_file
from tools.tests.conftest import SPEC_FILE_PATH


class TestAssembleDfSequenceStaticAsserts(TestCase):
    """Tests for _Static_assert generation in assembled SEQUENCE headers."""

    spec: ClassVar[J2735Specification]

    @classmethod
    def setUpClass(cls) -> None:
        """Load spec once for all tests."""
        cls.spec = parse_spec_file(SPEC_FILE_PATH)

    def test_extensible_sequence_has_root_size_assert(self) -> None:
        """Extensible SEQUENCE validates ROOT_SIZE_BITS == PREFIX + BW_*."""
        code = generate_data_frame("PathPrediction", self.spec)
        self.assertIn(
            "_Static_assert(J2735_INTERNAL_ROOT_SIZE_BITS_PATH_PREDICTION",
            code,
        )
        self.assertIn("J2735_BW_PATH_PREDICTION", code)

    def test_non_extensible_no_optional_has_offset_chain_assert(self) -> None:
        """Non-extensible SEQUENCE with no OPTIONAL validates offset chain."""
        code = generate_data_frame("BSMcoreData", self.spec)
        self.assertIn("_Static_assert(", code)
        self.assertIn("J2735_BW_BSM_CORE_DATA", code)
        self.assertIn("J2735_BW_VEHICLE_SIZE", code)

    def test_non_extensible_with_optional_has_no_offset_assert(self) -> None:
        """Non-extensible SEQUENCE with OPTIONAL has no offset chain assert."""
        code = generate_data_frame("IntersectionReferenceID", self.spec)
        self.assertNotIn("_Static_assert(", code)

    def test_variable_width_sequence_excluded_from_offset_assert(self) -> None:
        """Non-extensible, no-optional SEQUENCE with variable-width fields skips assert.

        The template guard requires ``uper_bit_width is not none`` so that
        SEQUENCE types containing SEQUENCE OF or unresolved inline types
        (e.g. RestrictionClassAssignment, Sample) do not emit a
        _Static_assert referencing a nonexistent J2735_BW_* constant.
        """
        # These are non-extensible, no-optional but have variable-width fields
        variable_width_types = ["Circle", "PrivilegedEvents"]
        for type_name in variable_width_types:
            with self.subTest(type_name=type_name):
                typedef = self.spec.lookup_type(type_name)
                self.assertIsNotNone(typedef, f"{type_name} not in spec")
                assert typedef is not None  # narrow for type checkers
                constraint = typedef.constraint
                assert isinstance(constraint, SequenceType)
                self.assertFalse(constraint.is_extensible)
                self.assertFalse(
                    any(f.is_optional for f in constraint.fields),
                )
                self.assertIsNone(
                    constraint.uper_bit_width,
                    f"{type_name} should have variable width (None)",
                )
