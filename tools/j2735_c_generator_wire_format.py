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

from dataclasses import dataclass

from .j2735_spec_constraints import SequenceField, SequenceType

# =============================================================================
# Wire Variant Helper (the ONE thing that needs Python)
# =============================================================================


@dataclass(frozen=True, slots=True)
class WireVariant:
    """A wire format variant for rendering.

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


def get_sequence_variants(constraint: SequenceType) -> list[WireVariant]:
    """Generate wire format variants for a SEQUENCE.

    Args:
        constraint: The SequenceType constraint.

    Returns:
        List of WireVariant objects to render.

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
            WireVariant(
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
            WireVariant(
                name=f"no extensions, {_pluralize_bits(total_no_ext)}",
                fields=all_fields,
                ext_bit=0,
                opt_bitmap="",
                total_bits=total_no_ext,
            ),
            WireVariant(
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
        WireVariant(
            name=f"{absent_name}, {_pluralize_bits(absent_bits)}",
            fields=required_fields,
            ext_bit=0 if is_ext else None,
            opt_bitmap=absent_opt,
            total_bits=absent_bits,
        ),
        WireVariant(
            name=f"{present_name}, {_pluralize_bits(present_bits)}",
            fields=all_fields,
            ext_bit=0 if is_ext else None,
            opt_bitmap=present_opt,
            total_bits=present_bits,
        ),
    ]
