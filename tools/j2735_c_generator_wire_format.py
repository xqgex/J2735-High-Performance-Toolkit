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
J2735 Wire Format Documentation Generator.

Minimal implementation that passes ASN1TypeDefinition directly to templates.
All complex formatting logic lives in Jinja templates + filters.
"""

from collections.abc import Sequence
from dataclasses import dataclass
from typing import TypedDict

from .j2735_spec_constraints import SequenceField, SequenceType

# =============================================================================
# SEQUENCE Wire Format
# =============================================================================


@dataclass(frozen=True, slots=True)
class SequenceWireVariant:
    """A wire format variant for a SEQUENCE type.

    For fixed types: single variant with all fields.
    For OPTIONAL types: variant with fields to include.
    For extensible types: variant with extension bit value.
    """

    name: str
    fields: tuple[SequenceField, ...]
    ext_bit: int | None  # None = no ext bit, 0/1 = ext bit value
    opt_bitmap: str  # "" = no bitmap, "0" or "1" or "0..0" etc.
    total_bits: int | str  # int for fixed, "variable" for ext=1


def _pluralize_bits(n: int) -> str:
    """Format a bit count with correct singular/plural grammar.

    Args:
        n: Number of bits.

    Returns:
        Formatted string: "1 bit" for singular, "N bits" for all other counts.

    Examples:
        >>> _pluralize_bits(0)
        '0 bits'
        >>> _pluralize_bits(1)
        '1 bit'
        >>> _pluralize_bits(16)
        '16 bits'
    """
    return f"{n} bit" if n == 1 else f"{n} bits"


def _sum_field_bits(fields: tuple[SequenceField, ...]) -> int:
    """Sum the UPER bit widths of the given fields.

    Args:
        fields: Tuple of SequenceField objects.

    Returns:
        Total bit width (treating ``None`` widths as 0).

    Examples:
        >>> _sum_field_bits(())
        0
    """
    return sum(f.type.uper_bit_width or 0 for f in fields)


def get_sequence_variants(constraint: SequenceType) -> list[SequenceWireVariant]:
    """Generate wire format variants for a SEQUENCE.

    Args:
        constraint: The SequenceType constraint.

    Returns:
        List of SequenceWireVariant objects to render.

    Examples:
        >>> from tools.j2735_spec_constraints import SequenceType, SequenceField
        >>> from tools.j2735_spec_constraints import IntegerConstraint
        >>> seq = SequenceType(fields=(
        ...     SequenceField(
        ...         name="a", type_name="TypeA",
        ...         type=IntegerConstraint(min_value=0, max_value=127),
        ...         is_optional=False, section_comment="", inline_comment="",
        ...     ),
        ... ), is_extensible=False)
        >>> variants = get_sequence_variants(seq)
        >>> len(variants)
        1
        >>> variants[0].name
        '7 bits'
    """
    is_ext = constraint.is_extensible
    opt_count = constraint.optional_count
    all_fields = constraint.fields
    required_fields = tuple(f for f in all_fields if not f.is_optional)
    optional_names = [f.name for f in all_fields if f.is_optional]

    # Case 1: Fixed SEQUENCE (no OPTIONAL, not extensible)
    if not is_ext and opt_count == 0:
        total = _sum_field_bits(all_fields)
        return [
            SequenceWireVariant(
                name=_pluralize_bits(total),
                fields=all_fields,
                ext_bit=None,
                opt_bitmap="",
                total_bits=total,
            )
        ]

    # Case 2: Extensible SEQUENCE with no OPTIONAL
    if is_ext and opt_count == 0:
        total_no_ext = 1 + _sum_field_bits(all_fields)  # 1 for ext bit
        return [
            SequenceWireVariant(
                name=f"no extensions, {_pluralize_bits(total_no_ext)}",
                fields=all_fields,
                ext_bit=0,
                opt_bitmap="",
                total_bits=total_no_ext,
            ),
            SequenceWireVariant(
                name="with extensions, variable",
                fields=all_fields,
                ext_bit=1,
                opt_bitmap="",
                total_bits="variable",
            ),
        ]

    # Case 3: SEQUENCE with OPTIONAL fields (may also be extensible)
    ext_prefix = 1 if is_ext else 0

    # Variant: all optional ABSENT
    absent_bits = ext_prefix + opt_count + _sum_field_bits(required_fields)
    absent_opt = "0" if opt_count == 1 else f"0..0 ({opt_count})"
    absent_name = (
        f"{optional_names[0]} ABSENT" if len(optional_names) == 1 else "all optional ABSENT"
    )

    # Variant: all optional PRESENT
    present_bits = ext_prefix + opt_count + _sum_field_bits(all_fields)
    present_opt = "1" if opt_count == 1 else f"1..1 ({opt_count})"
    present_name = (
        f"{optional_names[0]} PRESENT" if len(optional_names) == 1 else "all optional PRESENT"
    )

    return [
        SequenceWireVariant(
            name=f"{absent_name}, {_pluralize_bits(absent_bits)}",
            fields=required_fields,
            ext_bit=0 if is_ext else None,
            opt_bitmap=absent_opt,
            total_bits=absent_bits,
        ),
        SequenceWireVariant(
            name=f"{present_name}, {_pluralize_bits(present_bits)}",
            fields=all_fields,
            ext_bit=0 if is_ext else None,
            opt_bitmap=present_opt,
            total_bits=present_bits,
        ),
    ]


# =============================================================================
# CHOICE Wire Format
# =============================================================================


class ChoiceAlternativeDict(TypedDict):
    """Type definition for alternative entries in CHOICE generation."""

    name: str
    type_ref: str
    bit_width: int
    index: int
    shift: int
    needs_shift: bool


@dataclass(frozen=True, slots=True)
class ChoiceWireVariant:
    """A wire format variant for a CHOICE alternative.

    Each variant represents one possible alternative selection.
    """

    name: str
    index: int
    index_bits: int
    type_ref: str
    value_bits: int
    total_bits: int


def get_choice_variants(
    alternatives: Sequence[ChoiceAlternativeDict],
    index_bits: int,
) -> list[ChoiceWireVariant]:
    """Generate wire format variants for a CHOICE type.

    Produces one variant per alternative, carrying domain data for the
    template to render (parallel to ``get_sequence_variants``).

    Args:
        alternatives: List of alternative dicts with keys ``name``,
            ``type_ref``, ``bit_width``, and ``index``.
        index_bits: Number of bits for the CHOICE index.

    Returns:
        List of ChoiceWireVariant objects, one per alternative.

    Examples:
        >>> alts = [
        ...     {"name": "a", "type_ref": "TypeA", "bit_width": 4, "index": 0},
        ...     {"name": "b", "type_ref": "TypeB", "bit_width": 8, "index": 1},
        ... ]
        >>> variants = get_choice_variants(alts, index_bits=1)
        >>> len(variants)
        2
        >>> variants[0].name
        'a selected, 5 bits total'
        >>> variants[0].total_bits
        5
        >>> variants[0].index
        0
        >>> variants[0].type_ref
        'TypeA'
        >>> variants[0].value_bits
        4
    """
    return [
        ChoiceWireVariant(
            name=f"{alt['name']} selected, "
            f"{_pluralize_bits(index_bits + alt['bit_width'])} total",
            index=alt["index"],
            index_bits=index_bits,
            type_ref=alt["type_ref"],
            value_bits=alt["bit_width"],
            total_bits=index_bits + alt["bit_width"],
        )
        for alt in alternatives
    ]
