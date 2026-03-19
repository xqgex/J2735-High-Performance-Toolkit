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
from typing import Final, TypedDict

from .j2735_spec_constraints import (
    SequenceField,
    SequenceType,
    TypeReference,
)

# =============================================================================
# SEQUENCE Wire Format
# =============================================================================

_VARIABLE_BITS: Final[str] = (
    "variable"  # Sentinel for ``total_bits`` when wire size cannot be computed
)


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


def _build_optional_variants(
    all_fields: tuple[SequenceField, ...],
    is_ext: bool,
) -> list[SequenceWireVariant]:
    """Build ABSENT / PRESENT variants for a SEQUENCE with OPTIONAL fields.

    Args:
        all_fields: All fields in the SEQUENCE.
        is_ext: Whether the SEQUENCE is extensible.

    Returns:
        Two-element list: [absent_variant, present_variant].
    """
    opt_count = sum(1 for f in all_fields if f.is_optional)
    required_fields = tuple(f for f in all_fields if not f.is_optional)
    optional_names = [f.name for f in all_fields if f.is_optional]
    preamble = (1 if is_ext else 0) + opt_count
    ext_bit = 0 if is_ext else None

    absent_opt = "0" if opt_count == 1 else f"0..0 ({opt_count})"
    present_opt = "1" if opt_count == 1 else f"1..1 ({opt_count})"

    if len(optional_names) == 1:
        absent_name = f"{optional_names[0]} ABSENT"
        present_name = f"{optional_names[0]} PRESENT"
    else:
        absent_name = "all optional ABSENT"
        present_name = "all optional PRESENT"

    absent_total, absent_label = _total_and_label(
        required_fields,
        preamble,
        absent_name,
    )
    present_total, present_label = _total_and_label(
        all_fields,
        preamble,
        present_name,
    )

    return [
        SequenceWireVariant(
            name=absent_label,
            fields=required_fields,
            ext_bit=ext_bit,
            opt_bitmap=absent_opt,
            total_bits=absent_total,
        ),
        SequenceWireVariant(
            name=present_label,
            fields=all_fields,
            ext_bit=ext_bit,
            opt_bitmap=present_opt,
            total_bits=present_total,
        ),
    ]


def _has_variable_width(
    fields: tuple[SequenceField, ...],
) -> bool:
    """Return True when any field has a variable-width type.

    Variable-width types (e.g. ``SequenceOfType``) legitimately
    have ``uper_bit_width is None`` even after resolution.

    Args:
        fields: Tuple of SequenceField objects.

    Returns:
        ``True`` if at least one field has ``None`` bit width.
    """
    return any(f.type.uper_bit_width is None for f in fields)


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


def _sum_field_bits(
    fields: tuple[SequenceField, ...],
) -> int:
    """Sum the UPER bit widths of the given fields.

    Callers **must** check ``_has_variable_width()`` first; calling
    this function on variable-width fields is a programming error.

    Args:
        fields: Tuple of SequenceField objects.

    Returns:
        Total bit width.

    Raises:
        TypeError: If any ``uper_bit_width`` is ``None``
            (variable-width field present).

    Examples:
        >>> _sum_field_bits(())
        0
    """
    total = 0
    for f in fields:
        bw = f.type.uper_bit_width
        if bw is None:
            raise TypeError(
                f"Field '{f.name}' has variable-width "
                "type - call _has_variable_width() "
                "before _sum_field_bits()."
            )
        total += bw
    return total


def _total_and_label(
    fields: tuple[SequenceField, ...],
    prefix_bits: int,
    name: str,
) -> tuple[int | str, str]:
    """Compute ``(total_bits, label)`` for a set of fields.

    If *fields* contain a variable-width type the total is
    ``_VARIABLE_BITS``; otherwise it is the numeric sum of
    *prefix_bits* + field bit widths.

    When *name* is non-empty the label is ``"<name>, <total>"``;
    when empty the label is just the total portion.

    Args:
        fields: Tuple of SequenceField objects.
        prefix_bits: Fixed overhead (ext bit, opt bitmap, etc.).
        name: Human-readable variant name prefix.

    Returns:
        ``(total_bits, label)`` pair.
    """
    if _has_variable_width(fields):
        suffix = _VARIABLE_BITS
        total: int | str = _VARIABLE_BITS
    else:
        total = prefix_bits + _sum_field_bits(fields)
        suffix = _pluralize_bits(total)
    label = f"{name}, {suffix}" if name else suffix
    return total, label


def _validate_fields_resolved(
    fields: tuple[SequenceField, ...],
) -> None:
    """Reject fields whose type is still an unresolved TypeReference.

    This is a **precondition guard** -- it catches programming errors
    where type resolution was skipped or failed *before* any
    computation is attempted.

    Args:
        fields: Tuple of SequenceField objects.

    Raises:
        ValueError: If any field's type is an unresolved
            ``TypeReference``.
    """
    for f in fields:
        if isinstance(f.type, TypeReference):
            raise ValueError(
                f"Field '{f.name}' (type '{f.type_name}') "
                "still has an unresolved TypeReference. "
                "All type references must be resolved "
                "before computing wire format."
            )


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

    _validate_fields_resolved(all_fields)

    # Case 1: Fixed SEQUENCE (no OPTIONAL, not extensible)
    if not is_ext and opt_count == 0:
        total, label = _total_and_label(all_fields, 0, "")
        return [
            SequenceWireVariant(
                name=label,
                fields=all_fields,
                ext_bit=None,
                opt_bitmap="",
                total_bits=total,
            )
        ]

    # Case 2: Extensible SEQUENCE with no OPTIONAL
    if is_ext and opt_count == 0:
        total, label = _total_and_label(all_fields, 1, "no extensions")
        return [
            SequenceWireVariant(
                name=label,
                fields=all_fields,
                ext_bit=0,
                opt_bitmap="",
                total_bits=total,
            ),
            SequenceWireVariant(
                name=f"with extensions, {_VARIABLE_BITS}",
                fields=all_fields,
                ext_bit=1,
                opt_bitmap="",
                total_bits=_VARIABLE_BITS,
            ),
        ]

    # Case 3: SEQUENCE with OPTIONAL fields (may also be extensible)
    return _build_optional_variants(all_fields, is_ext)


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
