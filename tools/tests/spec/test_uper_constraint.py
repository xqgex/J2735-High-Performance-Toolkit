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
"""Tests for UPERConstraint abstract base class.

Tests verify that UPERConstraint properly enforces required class members
on all subclasses.
"""

from dataclasses import dataclass
from unittest import TestCase

from tools.j2735_spec_constraints import (
    BitStringConstraint,
    EnumeratedType,
    IntegerConstraint,
    OctetStringConstraint,
    UPERConstraint,
)


class TestUPERConstraintEnforcement(TestCase):
    """Tests that UPERConstraint ABC enforces required members."""

    def test_missing_pattern_raises(self) -> None:
        """Subclass without PATTERN raises NotImplementedError."""
        with self.assertRaises(NotImplementedError) as ctx:
            # Attempt to define a subclass without PATTERN
            @dataclass(frozen=True, slots=True)
            class _BadConstraint(UPERConstraint):  # pyright: ignore[reportUnusedClass]
                """Bad Constraint"""

                value: int

                @classmethod
                def from_asn1(cls, raw_def: str):  # type: ignore  # pylint: disable=unused-argument
                    """Example"""
                    return None

                @property
                def uper_bit_width(self) -> int:
                    """Example"""
                    return 0

        self.assertIn("PATTERN", str(ctx.exception))

    def test_all_constraints_have_pattern(self) -> None:
        """All constraint classes have PATTERN defined."""
        constraint_classes: list[type[UPERConstraint]] = [
            IntegerConstraint,
            OctetStringConstraint,
            BitStringConstraint,
            EnumeratedType,
        ]
        for cls in constraint_classes:
            self.assertTrue(hasattr(cls, "PATTERN"), f"{cls.__name__} missing PATTERN")
            self.assertIsNotNone(cls.PATTERN, f"{cls.__name__}.PATTERN is None")

    def test_all_constraints_have_from_asn1(self) -> None:
        """All constraint classes have from_asn1 classmethod."""
        constraint_classes: list[type[UPERConstraint]] = [
            IntegerConstraint,
            OctetStringConstraint,
            BitStringConstraint,
            EnumeratedType,
        ]
        for cls in constraint_classes:
            self.assertTrue(hasattr(cls, "from_asn1"), f"{cls.__name__} missing from_asn1")
            self.assertTrue(callable(cls.from_asn1), f"{cls.__name__}.from_asn1 not callable")
