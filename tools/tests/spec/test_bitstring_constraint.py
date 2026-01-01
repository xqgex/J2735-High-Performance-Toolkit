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
"""Tests for BitStringConstraint class.

Tests cover UPER bit-width calculation for BIT STRING types
per ITU-T X.691.
"""

from unittest import TestCase

from tools.j2735_spec_constraints import BitStringConstraint


def _make_named_bits(count: int) -> dict[str, int]:
    """Create a complete named_bits dict with positions 0 to count-1."""
    return {f"bit{i}": i for i in range(count)}


class TestBitStringConstraint(TestCase):
    """Tests for BitStringConstraint."""

    def test_fixed_size(self) -> None:
        """Fixed BIT STRING sizes."""
        self.assertEqual(
            BitStringConstraint(
                root_size=1,
                is_extensible=False,
                extension_size=None,
                named_bits=_make_named_bits(1),
            ).uper_bit_width,
            1,
        )
        self.assertEqual(
            BitStringConstraint(
                root_size=12,
                is_extensible=False,
                extension_size=None,
                named_bits=_make_named_bits(12),
            ).uper_bit_width,
            12,
        )
        self.assertEqual(
            BitStringConstraint(
                root_size=16,
                is_extensible=False,
                extension_size=None,
                named_bits=_make_named_bits(16),
            ).uper_bit_width,
            16,
        )

    def test_extensible_returns_none_for_uper_bit_width(self) -> None:
        """Extensible BIT STRING has variable bit-width."""
        c = BitStringConstraint(
            root_size=13,
            is_extensible=True,
            extension_size=None,
            named_bits=_make_named_bits(13),
        )
        self.assertIsNone(c.uper_bit_width)

    def test_extensible_with_extension_size(self) -> None:
        """Extensible BIT STRING with known extension size."""
        c = BitStringConstraint(
            root_size=13,
            is_extensible=True,
            extension_size=14,
            named_bits=_make_named_bits(14),
        )
        self.assertEqual(c.root_size, 13)
        self.assertTrue(c.is_extensible)
        self.assertEqual(c.extension_size, 14)
        self.assertIsNone(c.uper_bit_width)

    def test_validation_root_size_positive(self) -> None:
        """root_size must be >= 1."""
        with self.assertRaises(ValueError):
            BitStringConstraint(
                root_size=0,
                is_extensible=False,
                extension_size=None,
                named_bits={"a": 0},
            )

    def test_validation_extension_size_requires_extensible(self) -> None:
        """extension_size requires is_extensible=True."""
        with self.assertRaises(ValueError):
            BitStringConstraint(
                root_size=12,
                is_extensible=False,
                extension_size=14,
                named_bits=_make_named_bits(14),
            )

    def test_validation_extension_size_greater_than_root(self) -> None:
        """extension_size must be > root_size."""
        with self.assertRaises(ValueError):
            BitStringConstraint(
                root_size=13,
                is_extensible=True,
                extension_size=13,
                named_bits=_make_named_bits(13),
            )
        with self.assertRaises(ValueError):
            BitStringConstraint(
                root_size=13,
                is_extensible=True,
                extension_size=10,
                named_bits=_make_named_bits(10),
            )


class TestBitStringParsing(TestCase):
    """Tests for BitStringConstraint.from_asn1() parsing."""

    def test_parse_fixed_size(self) -> None:
        """Parse BIT STRING with fixed SIZE constraint."""
        result = BitStringConstraint.from_asn1("BIT STRING { a (0), b (1), c (2) } (SIZE(3))")
        self.assertIsNotNone(result)
        assert result is not None
        self.assertEqual(result.root_size, 3)
        self.assertFalse(result.is_extensible)
        self.assertIsNone(result.extension_size)
        self.assertEqual(result.named_bits, {"a": 0, "b": 1, "c": 2})

    def test_parse_with_spaces(self) -> None:
        """Parse BIT STRING with spaces in SIZE constraint."""
        result = BitStringConstraint.from_asn1("BIT STRING { x (0) } ( SIZE ( 1 ) )")
        self.assertIsNotNone(result)
        assert result is not None
        self.assertEqual(result.root_size, 1)
        self.assertFalse(result.is_extensible)
        self.assertEqual(result.named_bits, {"x": 0})

    def test_parse_no_named_bits_returns_none(self) -> None:
        """BIT STRING without named bits fails to parse."""
        # This tests that we require named bits
        result = BitStringConstraint.from_asn1("BIT STRING {} (SIZE(8))")
        self.assertIsNone(result)

    def test_parse_mismatched_count_returns_none(self) -> None:
        """BIT STRING with mismatched named_bits count fails validation."""
        # 3 named bits but SIZE(12) - should fail
        result = BitStringConstraint.from_asn1("BIT STRING { a (0), b (1), c (2) } (SIZE(12))")
        self.assertIsNone(result)


class TestBitStringExtensibleSize(TestCase):
    """Tests for BIT STRING with extensible SIZE constraint.

    J2735 uses extensible SIZE like (SIZE (9, ...)) for types like ExteriorLights
    and (SIZE (13, ..., 14)) for VehicleEventFlags.

    The ", ..." indicates the SIZE is extensible (can grow in future versions).
    The optional third number specifies the known extension size.
    """

    def test_extensible_size_with_extension_marker(self) -> None:
        """Parse BIT STRING with extensible SIZE containing extension marker."""
        result = BitStringConstraint.from_asn1(
            "BIT STRING { lowBeam (0), highBeam (1) } (SIZE (2, ...))"
        )
        self.assertIsNotNone(result, "Should parse BIT STRING with extensible SIZE")
        assert result is not None
        self.assertEqual(result.root_size, 2)
        self.assertTrue(result.is_extensible)
        self.assertIsNone(result.extension_size)
        self.assertEqual(result.named_bits, {"lowBeam": 0, "highBeam": 1})

    def test_extensible_size_with_known_extension(self) -> None:
        """Parse BIT STRING with extensible SIZE and known extension size."""
        result = BitStringConstraint.from_asn1(
            "BIT STRING { a (0), b (1), c (2) } (SIZE (2, ..., 3))"
        )
        self.assertIsNotNone(result)
        assert result is not None
        self.assertEqual(result.root_size, 2)
        self.assertTrue(result.is_extensible)
        self.assertEqual(result.extension_size, 3)
        self.assertEqual(result.named_bits, {"a": 0, "b": 1, "c": 2})

    def test_extensible_size_spaces_variations(self) -> None:
        """Parse extensible SIZE with various spacing."""
        # Test with different whitespace patterns - all have 1 named bit and SIZE(1, ...)
        test_cases = [
            "BIT STRING { a (0) } (SIZE(1, ...))",
            "BIT STRING { a (0) } (SIZE (1, ...))",
            "BIT STRING { a (0) } ( SIZE ( 1 , ... ) )",
        ]
        for raw_def in test_cases:
            with self.subTest(raw_def=raw_def):
                result = BitStringConstraint.from_asn1(raw_def)
                self.assertIsNotNone(result, f"Should parse: {raw_def}")
                assert result is not None
                self.assertEqual(result.root_size, 1)
                self.assertTrue(result.is_extensible)
                self.assertEqual(result.named_bits, {"a": 0})

    def test_real_world_vehicle_event_flags(self) -> None:
        """Parse real VehicleEventFlags definition from J2735."""
        raw_def = """BIT STRING {
            eventHazardLights (0),
            eventStopLineViolation (1),
            eventABSactivated (2),
            eventTractionControlLoss (3),
            eventStabilityControlactivated (4),
            eventHazardousMaterials (5),
            eventReserved1 (6),
            eventHardBraking (7),
            eventLightsChanged (8),
            eventWipersChanged (9),
            eventFlatTire (10),
            eventDisabledVehicle (11),
            eventAirBagDeployment (12),
            eventJackKnife (13)
        } (SIZE (13, ..., 14))"""
        result = BitStringConstraint.from_asn1(raw_def)
        self.assertIsNotNone(result, "Should parse VehicleEventFlags definition")
        assert result is not None
        self.assertEqual(result.root_size, 13)
        self.assertTrue(result.is_extensible)
        self.assertEqual(result.extension_size, 14)
        # Verify all 14 named bits
        self.assertEqual(len(result.named_bits), 14)
        self.assertEqual(result.named_bits["eventHazardLights"], 0)
        self.assertEqual(result.named_bits["eventJackKnife"], 13)

    def test_real_world_exterior_lights(self) -> None:
        """Parse real ExteriorLights definition from J2735."""
        raw_def = """BIT STRING {
            lowBeamHeadlightsOn (0),
            highBeamHeadlightsOn (1),
            leftTurnSignalOn (2),
            rightTurnSignalOn (3),
            hazardSignalOn (4),
            automaticLightControlOn (5),
            daytimeRunningLightsOn (6),
            fogLightOn (7),
            parkingLightsOn (8)
        } (SIZE (9, ...))"""
        result = BitStringConstraint.from_asn1(raw_def)
        self.assertIsNotNone(result, "Should parse ExteriorLights definition")
        assert result is not None
        self.assertEqual(result.root_size, 9)
        self.assertTrue(result.is_extensible)
        self.assertIsNone(result.extension_size)
        # Verify all 9 named bits
        self.assertEqual(len(result.named_bits), 9)
        self.assertEqual(result.named_bits["lowBeamHeadlightsOn"], 0)
        self.assertEqual(result.named_bits["parkingLightsOn"], 8)


class TestBitStringNamedBitsValidation(TestCase):
    """Tests for named_bits validation in BitStringConstraint.

    Validates that named_bits:
        - Must not be empty
        - Count must equal max_size (all positions must be named)
        - All positions must be >= 0
        - No duplicate positions allowed
        - All positions must be < max_size (extension_size or root_size)
    """

    def test_empty_named_bits_raises(self) -> None:
        """Empty named_bits must raise ValueError."""
        with self.assertRaises(ValueError) as ctx:
            BitStringConstraint(
                root_size=8, is_extensible=False, extension_size=None, named_bits={}
            )
        self.assertIn("must not be empty", str(ctx.exception))

    def test_count_mismatch_raises(self) -> None:
        """named_bits count must equal max_size."""
        with self.assertRaises(ValueError) as ctx:
            BitStringConstraint(
                root_size=8,
                is_extensible=False,
                extension_size=None,
                named_bits={"a": 0, "b": 1},  # Only 2, but root_size is 8
            )
        self.assertIn("named_bits count (2) must equal max size (8)", str(ctx.exception))

    def test_negative_position_raises(self) -> None:
        """Negative bit positions must raise ValueError."""
        with self.assertRaises(ValueError) as ctx:
            BitStringConstraint(
                root_size=1, is_extensible=False, extension_size=None, named_bits={"bad": -1}
            )
        self.assertIn("must be >= 0", str(ctx.exception))

    def test_duplicate_position_raises(self) -> None:
        """Duplicate bit positions must raise ValueError."""
        with self.assertRaises(ValueError) as ctx:
            BitStringConstraint(
                root_size=2,
                is_extensible=False,
                extension_size=None,
                named_bits={"a": 0, "b": 0},  # 2 entries but duplicate position
            )
        self.assertIn("Duplicate bit position", str(ctx.exception))

    def test_position_exceeds_root_size_raises(self) -> None:
        """Bit position >= root_size must raise ValueError."""
        with self.assertRaises(ValueError) as ctx:
            BitStringConstraint(
                root_size=8,
                is_extensible=False,
                extension_size=None,
                named_bits=_make_named_bits(7) | {"overflow": 8},  # 8 entries, but pos 8 invalid
            )
        self.assertIn("must be < max size", str(ctx.exception))

    def test_position_at_boundary_succeeds(self) -> None:
        """Bit position at root_size - 1 should succeed."""
        c = BitStringConstraint(
            root_size=8,
            is_extensible=False,
            extension_size=None,
            named_bits=_make_named_bits(8),
        )
        self.assertEqual(c.named_bits["bit7"], 7)

    def test_extended_position_within_extension_size(self) -> None:
        """Extensible with extension_size allows positions up to extension_size - 1."""
        c = BitStringConstraint(
            root_size=13,
            is_extensible=True,
            extension_size=14,
            named_bits=_make_named_bits(14),
        )
        self.assertEqual(c.named_bits["bit12"], 12)
        self.assertEqual(c.named_bits["bit13"], 13)

    def test_extended_position_exceeds_extension_size_raises(self) -> None:
        """Bit position >= extension_size must raise ValueError."""
        with self.assertRaises(ValueError) as ctx:
            BitStringConstraint(
                root_size=13,
                is_extensible=True,
                extension_size=14,
                named_bits=_make_named_bits(13) | {"overflow": 14},  # pos 14 invalid
            )
        self.assertIn("must be < max size", str(ctx.exception))

    def test_extensible_without_extension_size_uses_root(self) -> None:
        """Extensible without extension_size validates against root_size."""
        # This should fail - position 13 is not valid without explicit extension_size
        with self.assertRaises(ValueError):
            BitStringConstraint(
                root_size=13,
                is_extensible=True,
                extension_size=None,
                named_bits=_make_named_bits(13) | {"overflow": 13},
            )
