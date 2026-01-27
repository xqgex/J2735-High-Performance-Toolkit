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
"""
J2735 Doxygen Wire Format Generator.

Computes byte-by-byte wire format representation for UPER-encoded SEQUENCEs.
Used to generate Doxygen documentation with visual wire format tables.
"""

from dataclasses import dataclass
from typing import Final

from .j2735_spec_constraints import SequenceType
from .j2735_spec_parser import ASN1TypeDefinition

# =============================================================================
# Constants
# =============================================================================

_BITS_PER_BYTE: Final[int] = 8  # 8 bits per byte/octet in OCTET STRING

# Format prefixes for human-readable bit range descriptions in wire format
# These must stay in sync with templates/wire_format.j2 which checks for "(B"
_BIT_SINGLE: Final[str] = "(Bit {high_bit})"  # e.g., "(Bit 7)"
_BIT_RANGE: Final[str] = "(Bits {high_bit}-{low_bit})"  # e.g., "(Bits 31-24)"


# =============================================================================
# Data Structures
# =============================================================================


@dataclass(frozen=True, kw_only=True, slots=True)
class ByteSegment:
    """A segment of a field within a single byte (for wire format display).

    When a field spans multiple bytes, it produces multiple ByteSegments.

    Attributes:
        byte_offset: Byte index from start of SEQUENCE.
        bit_start: Start bit within byte (0-7, 0 = MSB).
        bit_count: Number of bits in this byte (1-8).
        field_name: Name of the field this segment belongs to.
        type_name: The ASN.1 type name (e.g., "TemporaryID").
        field_bits: Human-readable bit range (e.g., "(Bits 31-24)" or "(7)").
        is_first: True if this is the first segment of the field.
        is_last: True if this is the last segment of the field.
    """

    byte_offset: int
    bit_start: int
    bit_count: int
    field_name: str
    type_name: str
    field_bits: str
    is_first: bool
    is_last: bool


# =============================================================================
# Helper Functions
# =============================================================================


def _format_field_bits(
    bit_width: int,
    bits_remaining: int,
    bits_in_this_byte: int,
) -> str:
    """Format human-readable bit range description for a segment.

    Args:
        bit_width: Total bits in the field.
        bits_remaining: Bits still to be processed (including this segment).
        bits_in_this_byte: Bits in the current byte segment.

    Returns:
        Formatted string like "(Bits 31-24)", "(Bit 7)", "(8)", or "".

    Examples:
        >>> _format_field_bits(1, 1, 1)  # Single bit field
        ''
        >>> _format_field_bits(8, 8, 8)  # Fits in one byte
        '(8)'
        >>> _format_field_bits(32, 32, 8)  # First byte of 32-bit
        '(Bits 31-24)'
        >>> _format_field_bits(32, 24, 8)  # Second byte of 32-bit
        '(Bits 23-16)'
        >>> _format_field_bits(32, 8, 8)  # Last byte of 32-bit
        '(Bits 7-0)'
        >>> _format_field_bits(9, 1, 1)  # Single bit at end
        '(Bit 0)'
    """
    if bit_width == 1:
        return ""
    if bit_width == bits_in_this_byte:  # Entire field fits in one segment
        return f"({bit_width})"

    bits_consumed = bit_width - bits_remaining
    high_bit = bit_width - 1 - bits_consumed
    low_bit = high_bit - bits_in_this_byte + 1

    if high_bit == low_bit:
        return _BIT_SINGLE.format(high_bit=high_bit)
    return _BIT_RANGE.format(high_bit=high_bit, low_bit=low_bit)


def _generate_field_segments(
    field_name: str,
    type_name: str,
    bit_width: int,
    start_bit: int,
) -> tuple[ByteSegment, ...]:
    """Generate ByteSegments for a single field.

    Args:
        field_name: Name of the field.
        type_name: ASN.1 type name for display.
        bit_width: Total bit width of the field.
        start_bit: Starting bit position in the SEQUENCE.

    Returns:
        Tuple of ByteSegments spanning the field.

    Examples:
        >>> segs = _generate_field_segments("id", "TemporaryID", 32, 0)
        >>> len(segs)  # 32 bits = 4 bytes
        4
        >>> segs[0].byte_offset, segs[0].bit_count, segs[0].field_bits
        (0, 8, '(Bits 31-24)')
        >>> segs[3].byte_offset, segs[3].is_last, segs[3].field_bits
        (3, True, '(Bits 7-0)')
        >>> segs = _generate_field_segments("msgCnt", "MsgCount", 8, 0)
        >>> len(segs)  # 8 bits = 1 byte
        1
        >>> segs[0].is_first, segs[0].is_last, segs[0].field_bits
        (True, True, '(8)')
        >>> segs = _generate_field_segments("flag", "BOOLEAN", 1, 7)
        >>> segs[0].byte_offset, segs[0].bit_start, segs[0].field_bits
        (0, 7, '')
    """
    segments: list[ByteSegment] = []
    bit_pos = start_bit
    bits_remaining = bit_width
    is_first = True

    while bits_remaining > 0:
        byte_idx = bit_pos // _BITS_PER_BYTE
        bit_in_byte = bit_pos % _BITS_PER_BYTE
        bits_in_this_byte = min(_BITS_PER_BYTE - bit_in_byte, bits_remaining)
        is_last = bits_remaining == bits_in_this_byte

        field_bits = _format_field_bits(bit_width, bits_remaining, bits_in_this_byte)

        segments.append(
            ByteSegment(
                byte_offset=byte_idx,
                bit_start=bit_in_byte,
                bit_count=bits_in_this_byte,
                field_name=field_name,
                type_name=type_name,
                field_bits=field_bits,
                is_first=is_first,
                is_last=is_last,
            )
        )

        bit_pos += bits_in_this_byte
        bits_remaining -= bits_in_this_byte
        is_first = False

    return tuple(segments)


# =============================================================================
# Core Functions
# =============================================================================


def compute_wire_format(typedef: ASN1TypeDefinition) -> tuple[tuple[ByteSegment, ...], ...]:
    """Compute byte-by-byte wire format representation of a SEQUENCE.

    For each byte, produces a tuple of ByteSegments showing which fields
    occupy which bits. Computes bit offsets inline without intermediate structures.

    Args:
        typedef: The SEQUENCE type definition.

    Returns:
        Tuple of tuples, one per byte. Empty if any field has unknown width.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> bsm = spec.lookup_type("BSMcoreData")
        >>> wire = compute_wire_format(bsm)
        >>> len(wire)  # 37 bytes
        37
        >>> wire[0][0].type_name
        'MsgCount'
    """
    # Must be a SEQUENCE with resolved fields
    if not isinstance(typedef.constraint, SequenceType):
        return ()

    fields = typedef.constraint.fields

    # First pass: compute total bits and validate all fields
    total_bits = 0
    field_widths: list[int] = []

    for field in fields:
        if field.is_optional:
            return ()  # Can't compute for OPTIONAL fields
        # Use the resolved field type's bit-width directly
        width = field.type.uper_bit_width if field.type else None
        if width is None:
            return ()
        field_widths.append(width)
        total_bits += width

    if total_bits == 0:
        return ()

    # Second pass: generate ByteSegments
    total_bytes = (total_bits + _BITS_PER_BYTE - 1) // _BITS_PER_BYTE
    result: list[list[ByteSegment]] = [[] for _ in range(total_bytes)]
    current_bit = 0

    for field, bit_width in zip(fields, field_widths, strict=True):
        segments = _generate_field_segments(
            field_name=field.name,
            type_name=field.type_name,
            bit_width=bit_width,
            start_bit=current_bit,
        )
        for segment in segments:
            result[segment.byte_offset].append(segment)
        current_bit += bit_width

    return tuple(tuple(byte_segments) for byte_segments in result)
