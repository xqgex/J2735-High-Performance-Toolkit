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
"""Tests for wire format Jinja2 template rendering.

Tests cover:
    - asn1_definition.j2: ASN.1 definition block rendering
    - wire_format_table.j2: column-based Unicode box table
    - wire_format_compact.j2: row-based compact table
    - Structural invariants: box-drawing symmetry, bit continuity, field presence
"""

import re
from unittest import TestCase

from tools.j2735_c_generator_jinja import create_jinja_env, get_template
from tools.j2735_c_generator_wire_format import (
    SequenceWireVariant,
    get_sequence_variants,
)
from tools.j2735_spec_constraints import (
    SequenceField,
    SequenceType,
)
from tools.j2735_spec_parser import (
    ASN1TypeClass,
    ASN1TypeDefinition,
    J2735Specification,
)
from tools.tests.conftest import (
    get_sequence_typedef,
    make_extensible_mock_spec,
    make_integer_field,
    make_nested_mock_spec,
    make_optional_mock_spec,
    make_sequence,
)

# =============================================================================
# Template Names
# =============================================================================

_ASN1_TEMPLATE = "asn1_definition.j2"
_WIRE_FORMAT_TABLE_TEMPLATE = "wire_format_table.j2"
_WIRE_FORMAT_COMPACT_TEMPLATE = "wire_format_compact.j2"


# =============================================================================
# Template Rendering Helpers
# =============================================================================


def _render_asn1(typedef: ASN1TypeDefinition) -> str:
    """Render the ASN.1 definition template.

    Args:
        typedef: The type definition to render.

    Returns:
        Rendered ASN.1 definition string.
    """
    return get_template(create_jinja_env(), _ASN1_TEMPLATE).render(typedef=typedef)


def _render_wire_format(
    variant: SequenceWireVariant,
    opt_count: int,
    *,
    compact: bool = False,
) -> str:
    """Render a wire format table template.

    Args:
        variant: The SequenceWireVariant to render.
        opt_count: Number of optional fields.
        compact: If True, use compact (row-based) template.

    Returns:
        Rendered wire format table string.
    """
    name = _WIRE_FORMAT_COMPACT_TEMPLATE if compact else _WIRE_FORMAT_TABLE_TEMPLATE
    return get_template(create_jinja_env(), name).render(variant=variant, opt_count=opt_count)


def _render_real_wire_format(
    type_name: str,
    spec: J2735Specification,
    variant_index: int,
) -> str:
    """Look up a SEQUENCE type and render its wire format table.

    Args:
        type_name: Name of the SEQUENCE type.
        spec: The parsed J2735 specification.
        variant_index: Which variant to render (0 or 1).

    Returns:
        Rendered wire format table string.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.
    """
    typedef = get_sequence_typedef(type_name, spec)
    assert isinstance(typedef.constraint, SequenceType)
    variants = get_sequence_variants(typedef.constraint)
    return _render_wire_format(variants[variant_index], typedef.constraint.optional_count)


def _render_real_asn1(type_name: str, spec: J2735Specification) -> str:
    """Look up a SEQUENCE type and render its ASN.1 definition.

    Args:
        type_name: Name of the SEQUENCE type.
        spec: The parsed J2735 specification.

    Returns:
        Rendered ASN.1 definition string.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.
    """
    typedef = get_sequence_typedef(type_name, spec)
    return _render_asn1(typedef)


# =============================================================================
# Test Helpers — Synthetic Type Builders
# =============================================================================


def _make_typedef(
    name: str,
    fields: tuple[SequenceField, ...],
    *,
    is_extensible: bool = False,
) -> ASN1TypeDefinition:
    """Create an ASN1TypeDefinition with a SequenceType constraint.

    Args:
        name: Type name.
        fields: Ordered fields.
        is_extensible: Whether the SEQUENCE has "...".

    Returns:
        An ASN1TypeDefinition.
    """
    return ASN1TypeDefinition(
        name=name,
        type_class=ASN1TypeClass.SEQUENCE,
        raw_definition="SEQUENCE { ... }",
        constraint=make_sequence(fields=fields, is_extensible=is_extensible),
        spec_section="",
        description="",
    )


# =============================================================================
# Box-Drawing Structural Validators
# =============================================================================


def _get_content_lines(text: str) -> list[str]:
    """Extract non-empty lines from rendered output.

    Args:
        text: The rendered template output.

    Returns:
        List of non-empty stripped lines.
    """
    return [line for line in text.splitlines() if line.strip()]


def _extract_row_widths(line: str) -> list[int]:
    """Extract column widths from a box-drawing border line.

    Parses a line like " * ┌────┬────────┐" and returns the width
    of each cell (count of '─' characters between connectors).

    Args:
        line: A box-drawing border line containing '─'.

    Returns:
        List of cell widths (number of '─' chars per cell).
    """
    # Find all runs of ─ characters
    return [len(m.group()) for m in re.finditer(r"─+", line)]


# =============================================================================
# Tests — ASN.1 Definition Template
# =============================================================================


class TestASN1DefinitionTemplate(TestCase):
    """Tests for asn1_definition.j2 rendering."""

    def test_contains_type_name(self) -> None:
        """Output contains the type name in the header line."""
        typedef = _make_typedef(
            "PositionalAccuracy",
            fields=(
                make_integer_field("semiMajor", "SemiMajorAxisAccuracy", 0, 255),
                make_integer_field("semiMinor", "SemiMinorAxisAccuracy", 0, 255),
            ),
        )
        output = _render_asn1(typedef)

        self.assertIn("PositionalAccuracy ::= SEQUENCE {", output)

    def test_contains_all_field_names(self) -> None:
        """Output contains every field name."""
        typedef = _make_typedef(
            "TestType",
            fields=(
                make_integer_field("alpha", "TypeA", 0, 127),
                make_integer_field("beta", "TypeB", 0, 127),
                make_integer_field("gamma", "TypeC", 0, 127),
            ),
        )
        output = _render_asn1(typedef)

        self.assertIn("alpha", output)
        self.assertIn("beta", output)
        self.assertIn("gamma", output)

    def test_contains_type_names(self) -> None:
        """Output contains every field's ASN.1 type name."""
        typedef = _make_typedef(
            "TestType",
            fields=(
                make_integer_field("a", "MsgCount", 0, 127),
                make_integer_field("b", "Latitude", 0, 127),
            ),
        )
        output = _render_asn1(typedef)

        self.assertIn("MsgCount", output)
        self.assertIn("Latitude", output)

    def test_closing_brace(self) -> None:
        """Output ends with closing brace."""
        typedef = _make_typedef(
            "TestType",
            fields=(make_integer_field("a", "TypeA", 0, 127),),
        )
        output = _render_asn1(typedef)

        self.assertIn(" * }", output)

    def test_extensible_has_ellipsis(self) -> None:
        """Extensible SEQUENCE includes '...' marker."""
        typedef = _make_typedef(
            "TestType",
            fields=(make_integer_field("a", "TypeA", 0, 127),),
            is_extensible=True,
        )
        output = _render_asn1(typedef)

        self.assertIn("...", output)

    def test_non_extensible_no_ellipsis(self) -> None:
        """Non-extensible SEQUENCE does not include '...' marker."""
        typedef = _make_typedef(
            "TestType",
            fields=(make_integer_field("a", "TypeA", 0, 127),),
        )
        output = _render_asn1(typedef)

        # "..." should not appear (except in template comments)
        lines = output.splitlines()
        ellipsis_lines = [ln for ln in lines if "..." in ln and "*" in ln]
        self.assertEqual(len(ellipsis_lines), 0)

    def test_optional_field_marked(self) -> None:
        """OPTIONAL field includes 'OPTIONAL' keyword."""
        typedef = _make_typedef(
            "TestType",
            fields=(
                make_integer_field("opt", "OptType", 0, 127, is_optional=True),
                make_integer_field("req", "ReqType", 0, 127),
            ),
        )
        output = _render_asn1(typedef)

        self.assertIn("OPTIONAL", output)

    def test_bit_width_in_comment(self) -> None:
        """Bit width appears in the inline comment."""
        typedef = _make_typedef(
            "TestType",
            fields=(make_integer_field("x", "TypeX", 0, 255),),  # 8 bits
        )
        output = _render_asn1(typedef)

        self.assertIn("8 bits", output)

    def test_signed_field_comment(self) -> None:
        """Signed field has 'signed' in comment."""
        typedef = _make_typedef(
            "TestType",
            fields=(make_integer_field("x", "TypeX", -100, 100),),
        )
        output = _render_asn1(typedef)

        self.assertIn("signed", output)

    def test_unsigned_field_comment(self) -> None:
        """Unsigned field has 'unsigned' in comment."""
        typedef = _make_typedef(
            "TestType",
            fields=(make_integer_field("x", "TypeX", 0, 255),),
        )
        output = _render_asn1(typedef)

        self.assertIn("unsigned", output)

    def test_range_in_comment(self) -> None:
        """Value range appears in the inline comment."""
        typedef = _make_typedef(
            "TestType",
            fields=(make_integer_field("x", "TypeX", 0, 255),),
        )
        output = _render_asn1(typedef)

        self.assertIn("0..255", output)

    def test_commas_between_fields(self) -> None:
        """Fields (except last in non-extensible) have trailing commas."""
        typedef = _make_typedef(
            "TestType",
            fields=(
                make_integer_field("a", "TypeA", 0, 127),
                make_integer_field("b", "TypeB", 0, 127),
                make_integer_field("c", "TypeC", 0, 127),
            ),
        )
        output = _render_asn1(typedef)

        # Check the type portion only (before "--" comment which may contain commas)
        for line in output.splitlines():
            type_part = line.split("--")[0] if "--" in line else line
            if "TypeA" in type_part or "TypeB" in type_part:
                self.assertIn(",", type_part)
            elif "TypeC" in type_part:
                self.assertNotIn(",", type_part)

    def test_doxygen_prefix(self) -> None:
        """Every content line starts with ' * '."""
        typedef = _make_typedef(
            "TestType",
            fields=(make_integer_field("a", "TypeA", 0, 127),),
        )
        output = _render_asn1(typedef)

        for line in output.splitlines():
            if line.strip():
                self.assertTrue(
                    line.startswith(" * "),
                    f"Line missing ' * ' prefix: {line!r}",
                )


# =============================================================================
# Tests — Wire Format Table Template (Column-Based)
# =============================================================================


class TestWireFormatTableTemplate(TestCase):
    """Tests for wire_format_table.j2 rendering."""

    def _make_fixed_variant(self) -> tuple[SequenceWireVariant, int]:
        """Create a simple 2-field fixed variant for testing.

        Returns:
            Tuple of (SequenceWireVariant, opt_count).
        """
        seq = make_sequence(
            fields=(
                make_integer_field("alpha", "TypeA", 0, 255),  # 8 bits
                make_integer_field("beta", "TypeB", 0, 15),  # 4 bits
            ),
        )
        variants = get_sequence_variants(seq)
        return variants[0], seq.optional_count

    def test_contains_box_drawing_top(self) -> None:
        """Output contains top border with ┌ and ┐."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)

        self.assertIn("┌", output)
        self.assertIn("┐", output)

    def test_contains_box_drawing_bottom(self) -> None:
        """Output contains bottom border with └ and ┘."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)

        self.assertIn("└", output)
        self.assertIn("┘", output)

    def test_contains_field_names(self) -> None:
        """Output contains all field names."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)

        self.assertIn("alpha", output)
        self.assertIn("beta", output)

    def test_contains_bit_widths(self) -> None:
        """Output contains field bit widths in parentheses."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)

        self.assertIn("alpha (8)", output)
        self.assertIn("beta (4)", output)

    def test_contains_bit_range_headers(self) -> None:
        """Output contains bit range headers (e.g., 'Bits 0-7')."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)

        self.assertIn("Bits 0-7", output)
        self.assertIn("Bits 8-11", output)

    def test_top_and_bottom_widths_match(self) -> None:
        """Top border and bottom border have identical column widths."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)
        lines = _get_content_lines(output)

        top_line = next(ln for ln in lines if "┌" in ln)
        bottom_line = next(ln for ln in lines if "└" in ln)

        self.assertEqual(_extract_row_widths(top_line), _extract_row_widths(bottom_line))

    def test_middle_border_widths_match(self) -> None:
        """Middle border (├/┤) has same column widths as top/bottom."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)
        lines = _get_content_lines(output)

        top_line = next(ln for ln in lines if "┌" in ln)
        mid_line = next(ln for ln in lines if "├" in ln)

        self.assertEqual(_extract_row_widths(top_line), _extract_row_widths(mid_line))

    def test_column_count_matches_field_count(self) -> None:
        """Number of columns equals number of fields (no preamble)."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)
        lines = _get_content_lines(output)

        top_line = next(ln for ln in lines if "┌" in ln)
        col_count = len(_extract_row_widths(top_line))

        self.assertEqual(col_count, len(variant.fields))

    def test_ext_bit_adds_column(self) -> None:
        """Extension bit adds an extra column."""
        seq = make_sequence(
            fields=(make_integer_field("a", "TypeA", 0, 255),),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)
        variant = variants[0]  # no extensions
        output = _render_wire_format(variant, seq.optional_count)
        lines = _get_content_lines(output)

        top_line = next(ln for ln in lines if "┌" in ln)
        col_count = len(_extract_row_widths(top_line))

        # 1 ext bit column + 1 field column = 2
        self.assertEqual(col_count, 2)
        self.assertIn("Ext=0", output)

    def test_opt_bitmap_adds_column(self) -> None:
        """Optional bitmap adds an extra column."""
        seq = make_sequence(
            fields=(
                make_integer_field("flag", "FlagType", 0, 127, is_optional=True),
                make_integer_field("count", "CountType", 0, 127),
            ),
        )
        variants = get_sequence_variants(seq)
        variant = variants[0]  # ABSENT
        output = _render_wire_format(variant, seq.optional_count)
        lines = _get_content_lines(output)

        top_line = next(ln for ln in lines if "┌" in ln)
        col_count = len(_extract_row_widths(top_line))

        # 1 opt bitmap column + 1 required field column = 2
        self.assertEqual(col_count, 2)
        self.assertIn("Opt=0", output)

    def test_doxygen_prefix(self) -> None:
        """Every content line starts with ' * '."""
        variant, opt_count = self._make_fixed_variant()
        output = _render_wire_format(variant, opt_count)

        for line in output.splitlines():
            if line.strip():
                self.assertTrue(
                    line.startswith(" * "),
                    f"Line missing ' * ' prefix: {line!r}",
                )

    def test_extension_variant_placeholder(self) -> None:
        """'with extensions' variant shows '(extension data)' placeholder."""
        seq = make_sequence(
            fields=(make_integer_field("a", "TypeA", 0, 255),),
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)
        variant = variants[1]  # with extensions
        output = _render_wire_format(variant, seq.optional_count)

        self.assertIn("(extension data)", output)


# =============================================================================
# Tests — Wire Format Compact Template (Row-Based)
# =============================================================================


class TestWireFormatCompactTemplate(TestCase):
    """Tests for wire_format_compact.j2 rendering."""

    def _make_large_variant(self) -> tuple[SequenceWireVariant, int]:
        """Create a 10-field variant (triggers compact mode).

        Returns:
            Tuple of (SequenceWireVariant, opt_count).
        """
        fields = tuple(make_integer_field(f"field{i}", f"Type{i}", 0, 255) for i in range(10))
        seq = make_sequence(fields=fields)
        variants = get_sequence_variants(seq)
        return variants[0], seq.optional_count

    @staticmethod
    def _has_doxygen_prefix(line: str) -> bool:
        """Check if a line has a valid Doxygen comment prefix.

        The compact template has a known whitespace variation where
        some lines start with '* ' instead of ' * '. Both are valid
        within a Doxygen comment block.

        Args:
            line: A line from the rendered output.

        Returns:
            True if the line starts with ' * ' or '* '.
        """
        return line.startswith(" * ") or line.startswith("* ")

    def test_contains_header_row(self) -> None:
        """Output contains 'Bits' and 'Content' header labels."""
        variant, opt_count = self._make_large_variant()
        output = _render_wire_format(variant, opt_count, compact=True)

        self.assertIn("Bits", output)
        self.assertIn("Content", output)

    def test_contains_all_field_names(self) -> None:
        """Output contains every field name."""
        variant, opt_count = self._make_large_variant()
        output = _render_wire_format(variant, opt_count, compact=True)

        for field in variant.fields:
            self.assertIn(field.name, output)

    def test_contains_box_drawing(self) -> None:
        """Output contains top and bottom box-drawing characters."""
        variant, opt_count = self._make_large_variant()
        output = _render_wire_format(variant, opt_count, compact=True)

        self.assertIn("┌", output)
        self.assertIn("┐", output)
        self.assertIn("└", output)
        self.assertIn("┘", output)

    def test_two_columns_only(self) -> None:
        """Compact format always has exactly 2 columns."""
        variant, opt_count = self._make_large_variant()
        output = _render_wire_format(variant, opt_count, compact=True)
        lines = _get_content_lines(output)

        top_line = next(ln for ln in lines if "┌" in ln)
        col_count = len(_extract_row_widths(top_line))

        self.assertEqual(col_count, 2)

    def test_top_and_bottom_widths_match(self) -> None:
        """Top and bottom borders have identical column widths."""
        variant, opt_count = self._make_large_variant()
        output = _render_wire_format(variant, opt_count, compact=True)
        lines = _get_content_lines(output)

        top_line = next(ln for ln in lines if "┌" in ln)
        bottom_line = next(ln for ln in lines if "└" in ln)

        self.assertEqual(_extract_row_widths(top_line), _extract_row_widths(bottom_line))

    def test_row_count_matches_fields(self) -> None:
        """Number of data rows (│...│ lines after header) matches field count."""
        variant, opt_count = self._make_large_variant()
        output = _render_wire_format(variant, opt_count, compact=True)
        lines = _get_content_lines(output)

        # Count data rows: lines with │ that are NOT the header row and NOT borders
        data_rows = [ln for ln in lines if "│" in ln and "Bits" not in ln and "Content" not in ln]

        self.assertEqual(len(data_rows), len(variant.fields))

    def test_ext_bit_adds_row(self) -> None:
        """Extension bit adds a row in compact format."""
        fields = tuple(make_integer_field(f"f{i}", f"T{i}", 0, 255) for i in range(8))
        seq = make_sequence(fields=fields, is_extensible=True)
        variants = get_sequence_variants(seq)
        variant = variants[0]  # no extensions
        output = _render_wire_format(variant, seq.optional_count, compact=True)

        self.assertIn("Ext=0", output)

        lines = _get_content_lines(output)
        data_rows = [ln for ln in lines if "│" in ln and "Bits" not in ln and "Content" not in ln]

        # 8 fields + 1 ext bit row = 9
        self.assertEqual(len(data_rows), 9)

    def test_opt_bitmap_adds_row(self) -> None:
        """Optional bitmap adds a row in compact format."""
        fields = (
            *tuple(
                make_integer_field(f"opt{i}", f"Opt{i}", 0, 127, is_optional=True) for i in range(3)
            ),
            *tuple(make_integer_field(f"req{i}", f"Req{i}", 0, 255) for i in range(5)),
        )
        seq = make_sequence(fields=fields)
        variants = get_sequence_variants(seq)
        variant = variants[0]  # absent
        output = _render_wire_format(variant, seq.optional_count, compact=True)

        self.assertIn("Opt=", output)

    def test_doxygen_prefix(self) -> None:
        """Every content line has a valid Doxygen comment prefix."""
        variant, opt_count = self._make_large_variant()
        output = _render_wire_format(variant, opt_count, compact=True)

        for line in output.splitlines():
            if line.strip():
                self.assertTrue(
                    self._has_doxygen_prefix(line),
                    f"Line missing Doxygen prefix: {line!r}",
                )

    def test_extension_variant_placeholder(self) -> None:
        """'with extensions' variant shows '(extension data)' row."""
        fields = tuple(make_integer_field(f"f{i}", f"T{i}", 0, 255) for i in range(8))
        seq = make_sequence(fields=fields, is_extensible=True)
        variants = get_sequence_variants(seq)
        variant = variants[1]  # with extensions
        output = _render_wire_format(variant, seq.optional_count, compact=True)

        self.assertIn("(extension data)", output)


# =============================================================================
# Tests — Bit Position Continuity (Regression)
# =============================================================================


class TestBitPositionContinuity(TestCase):
    """Verify bit positions in rendered tables have no gaps or overlaps.

    This is the key regression test: if someone changes the template's
    bit position tracking, these tests catch it.
    """

    @staticmethod
    def _extract_bit_ranges(text: str) -> list[tuple[int, int]]:
        """Extract all (start, end) bit ranges from rendered output.

        Parses both "Bit N" and "Bits N-M" patterns (column table format).

        Args:
            text: Rendered template output.

        Returns:
            List of (start_bit, end_bit) tuples in order of appearance.
        """
        ranges: list[tuple[int, int]] = []
        for m in re.finditer(r"Bits?\s+(\d+)(?:-(\d+))?", text):
            start = int(m.group(1))
            end = int(m.group(2)) if m.group(2) else start
            ranges.append((start, end))
        return ranges

    @staticmethod
    def _extract_compact_bit_ranges(text: str) -> list[tuple[int, int]]:
        """Extract bit ranges from compact table data rows.

        Compact template uses bare "N" or "N-M" in the first column
        (e.g., "│ 0-7          │"). Skips the header row ("Bits").

        Args:
            text: Rendered compact template output.

        Returns:
            List of (start_bit, end_bit) tuples from data rows only.
        """
        ranges: list[tuple[int, int]] = []
        for line in text.splitlines():
            # Data rows have │ and contain field data, skip header/borders
            if "│" not in line or "Bits" in line or "Content" in line:
                continue
            # Extract the first column content between first two │ chars
            parts = line.split("│")
            if len(parts) < 3:
                continue
            cell = parts[1].strip()
            # Parse "N" or "N-M" or "N+" pattern
            m = re.match(r"^(\d+)(?:-(\d+))?(\+)?$", cell)
            if m:
                start = int(m.group(1))
                end = int(m.group(2)) if m.group(2) else start
                ranges.append((start, end))
        return ranges

    def test_column_table_bits_are_contiguous(self) -> None:
        """Column table bit ranges form a contiguous sequence from 0."""
        seq = make_sequence(
            fields=(
                make_integer_field("x", "IntX", 0, 255),  # 8 bits
                make_integer_field("y", "IntY", 0, 15),  # 4 bits
                make_integer_field("z", "IntZ", 0, 1),  # 1 bit
            ),
        )
        variants = get_sequence_variants(seq)
        output = _render_wire_format(variants[0], seq.optional_count)
        ranges = self._extract_bit_ranges(output)

        # Header row has the bit ranges (1 occurrence each)
        # Filter to just the header ranges (first N, where N = field count)
        header_ranges = ranges[: len(variants[0].fields)]

        # First range starts at 0
        self.assertEqual(header_ranges[0][0], 0)

        # Each range starts where previous ended + 1
        for i in range(1, len(header_ranges)):
            prev_end = header_ranges[i - 1][1]
            curr_start = header_ranges[i][0]
            self.assertEqual(
                curr_start,
                prev_end + 1,
                f"Gap between bit {prev_end} and {curr_start}",
            )

    def test_compact_table_bits_are_contiguous(self) -> None:
        """Compact table bit ranges form a contiguous sequence from 0."""
        fields = tuple(make_integer_field(f"f{i}", f"T{i}", 0, 255) for i in range(8))
        seq = make_sequence(fields=fields)
        variants = get_sequence_variants(seq)
        output = _render_wire_format(variants[0], seq.optional_count, compact=True)

        # Compact template uses bare "N" or "N-M" in the first column
        # (no "Bit"/"Bits" prefix). Extract from data rows only.
        data_ranges = self._extract_compact_bit_ranges(output)

        # First range starts at 0
        self.assertEqual(data_ranges[0][0], 0)

        # Each range starts where previous ended + 1
        for i in range(1, len(data_ranges)):
            prev_end = data_ranges[i - 1][1]
            curr_start = data_ranges[i][0]
            self.assertEqual(
                curr_start,
                prev_end + 1,
                f"Gap between bit {prev_end} and {curr_start}",
            )

    def test_ext_bit_at_position_zero(self) -> None:
        """Extension bit occupies position 0, fields start at 1."""
        seq = make_sequence(
            fields=(make_integer_field("a", "TypeA", 0, 255),),  # 8 bits
            is_extensible=True,
        )
        variants = get_sequence_variants(seq)
        output = _render_wire_format(variants[0], seq.optional_count)
        ranges = self._extract_bit_ranges(output)

        # First range is Bit 0 (ext bit)
        self.assertEqual(ranges[0], (0, 0))
        # Second range starts at 1
        self.assertEqual(ranges[1][0], 1)

    def test_opt_bitmap_position(self) -> None:
        """Optional bitmap occupies correct position after ext bit (if any)."""
        seq = make_sequence(
            fields=(
                make_integer_field("marker", "MarkerType", 0, 255, is_optional=True),  # 8 bits
                make_integer_field("value", "ValueType", 0, 15),  # 4 bits
            ),
        )
        variants = get_sequence_variants(seq)
        # ABSENT variant: opt bitmap at bit 0, then req field
        output = _render_wire_format(variants[0], seq.optional_count)
        ranges = self._extract_bit_ranges(output)

        # First range is Bit 0 (opt bitmap)
        self.assertEqual(ranges[0], (0, 0))
        # Second range is field starting at bit 1
        self.assertEqual(ranges[1][0], 1)

    def test_last_bit_matches_total(self) -> None:
        """Last bit position in table equals total_bits - 1."""
        seq = make_sequence(
            fields=(
                make_integer_field("x", "IntX", 0, 255),  # 8
                make_integer_field("y", "IntY", 0, 15),  # 4
                make_integer_field("z", "IntZ", 0, 1),  # 1
            ),
        )
        variants = get_sequence_variants(seq)
        output = _render_wire_format(variants[0], seq.optional_count)
        ranges = self._extract_bit_ranges(output)

        # Get the header ranges (first len(fields))
        header_ranges = ranges[: len(variants[0].fields)]
        last_bit = header_ranges[-1][1]

        total = variants[0].total_bits
        self.assertIsInstance(total, int)
        assert isinstance(total, int)  # for type narrowing
        self.assertEqual(last_bit, total - 1)


# =============================================================================
# Tests — Template Selection Logic
# =============================================================================


class TestTemplateSelection(TestCase):
    """Test that the right template is used based on field count."""

    def test_small_type_uses_column_table(self) -> None:
        """Types with ≤6 fields should use column-based table."""
        seq = make_sequence(
            fields=tuple(make_integer_field(f"f{i}", f"T{i}", 0, 255) for i in range(6)),
        )
        variants = get_sequence_variants(seq)

        # Column table should render correctly
        output = _render_wire_format(variants[0], seq.optional_count, compact=False)
        # Column table has columns = field count
        lines = _get_content_lines(output)
        top_line = next(ln for ln in lines if "┌" in ln)
        self.assertEqual(len(_extract_row_widths(top_line)), 6)

    def test_large_type_uses_compact_table(self) -> None:
        """Types with >6 fields should use compact (row-based) table."""
        seq = make_sequence(
            fields=tuple(make_integer_field(f"f{i}", f"T{i}", 0, 255) for i in range(10)),
        )
        variants = get_sequence_variants(seq)

        # Compact table should render correctly
        output = _render_wire_format(variants[0], seq.optional_count, compact=True)
        # Compact table always has 2 columns
        lines = _get_content_lines(output)
        top_line = next(ln for ln in lines if "┌" in ln)
        self.assertEqual(len(_extract_row_widths(top_line)), 2)


# =============================================================================
# Tests — With Real Spec Fixtures
# =============================================================================


class TestTemplatesWithRealFixtures(TestCase):
    """Render templates using the conftest mock specs to catch integration issues."""

    def test_positional_accuracy_column_table(self) -> None:
        """PositionalAccuracy (3 fields) renders as column table."""
        output = _render_real_wire_format("PositionalAccuracy", make_nested_mock_spec(), 0)

        self.assertIn("semiMajor", output)
        self.assertIn("semiMinor", output)
        self.assertIn("orientation", output)
        self.assertIn("Bits 0-7", output)

    def test_intersection_reference_id_absent(self) -> None:
        """IntersectionReferenceID ABSENT variant renders correctly."""
        output = _render_real_wire_format("IntersectionReferenceID", make_optional_mock_spec(), 0)

        self.assertIn("Opt=0", output)
        self.assertIn("id", output)
        # region should NOT appear in ABSENT variant
        self.assertNotIn("region", output)

    def test_intersection_reference_id_present(self) -> None:
        """IntersectionReferenceID PRESENT variant renders correctly."""
        output = _render_real_wire_format("IntersectionReferenceID", make_optional_mock_spec(), 1)

        self.assertIn("Opt=1", output)
        self.assertIn("region", output)
        self.assertIn("id", output)

    def test_path_prediction_no_ext(self) -> None:
        """PathPrediction 'no extensions' variant renders correctly."""
        output = _render_real_wire_format("PathPrediction", make_extensible_mock_spec(), 0)

        self.assertIn("Ext=0", output)
        self.assertIn("radiusOfCurve", output)
        self.assertIn("confidence", output)

    def test_path_prediction_with_ext(self) -> None:
        """PathPrediction 'with extensions' variant renders correctly."""
        output = _render_real_wire_format("PathPrediction", make_extensible_mock_spec(), 1)

        self.assertIn("Ext=1", output)
        self.assertIn("(extension data)", output)

    def test_path_prediction_asn1_definition(self) -> None:
        """PathPrediction ASN.1 definition renders correctly."""
        output = _render_real_asn1("PathPrediction", make_extensible_mock_spec())

        self.assertIn("PathPrediction ::= SEQUENCE {", output)
        self.assertIn("radiusOfCurve", output)
        self.assertIn("confidence", output)
        self.assertIn("...", output)
        self.assertIn("signed", output)  # radiusOfCurve is signed
