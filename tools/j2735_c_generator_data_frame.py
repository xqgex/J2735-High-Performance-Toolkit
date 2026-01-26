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
J2735 Data Frame C Code Generator.

Generates zero-copy C header files for J2735 Data Frame types (SEQUENCE and CHOICE).
Both type classes use the DF_ prefix in J2735 as they represent composite structures.

Example usage:
    from tools.j2735_c_generator_dataframe import generate_dataframe
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")

    # SEQUENCE types
    code = generate_dataframe("BSMcoreData", spec)

    # CHOICE types
    code = generate_dataframe("ApproachOrLane", spec)
"""

from pathlib import Path
from typing import TypedDict

from .j2735_c_generator_jinja import create_jinja_env, get_template
from .j2735_c_generator_wire_format import compute_wire_format
from .j2735_spec_constraints import ChoiceType, SequenceType
from .j2735_spec_parser import ASN1TypeClass, ASN1TypeDefinition, J2735Specification


class _ChoiceAlternativeDict(TypedDict):
    """Type definition for alternative entries in CHOICE generation."""

    name: str
    type_ref: str
    bit_width: int
    index: int
    shift: int
    needs_shift: bool


_SEQUENCE_TEMPLATE_NAME = "assemble_df.j2"
_CHOICE_TEMPLATE_NAME = "assemble_choice.j2"

# Maximum wire bits for single I/O pattern (J2735_READ_BITS limit at worst alignment)
_MAX_SINGLE_IO_BITS = 57


def generate_dataframe(type_name: str, spec: J2735Specification) -> str:
    """Generate complete C header file for a Data Frame (SEQUENCE or CHOICE).

    This is the main entry point for Data Frame code generation. Both SEQUENCE
    and CHOICE types are Data Frames in J2735 (DF_ prefix) as they represent
    composite structures.

    For SEQUENCE types: Generates struct container with field access macros.
    For CHOICE types: Generates single I/O macros for optimal performance.

    Args:
        type_name: Name of the type (e.g., "BSMcoreData", "ApproachOrLane").
        spec: The parsed J2735 specification.

    Returns:
        Complete C header file content as a string.

    Raises:
        ValueError: If type_name is not found.
        ValueError: If type is not SEQUENCE or CHOICE.
        ValueError: For CHOICE: if extensible or max_wire_bits > 57.
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")

    if typedef.type_class == ASN1TypeClass.SEQUENCE:
        return _generate_sequence(typedef)
    if typedef.type_class == ASN1TypeClass.CHOICE:
        return _generate_choice(typedef, spec)
    raise ValueError(f"Type '{type_name}' is {typedef.type_class}, not SEQUENCE or CHOICE")


def _generate_sequence(typedef: ASN1TypeDefinition) -> str:
    """Generate C code for a SEQUENCE type.

    Internal helper for generate_dataframe().

    Args:
        typedef: The ASN.1 type definition for the SEQUENCE.
        spec: The full specification (unused, kept for API consistency with _generate_choice).

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_dataframe("BSMcoreData", spec)
        >>> "J2735_PREFIX_BITS_BSM_CORE_DATA" in code
        True
        >>> "J2735_BSM_CORE_DATA_GET_MSG_CNT" in code
        True
    """
    # Wire format is empty for types with OPTIONAL fields (variable bit-width)
    wire_format = compute_wire_format(typedef)

    # Type narrowing for mypy - we know it's SequenceType because generate_dataframe checked
    constraint = typedef.constraint
    if not isinstance(constraint, SequenceType):
        raise TypeError(f"Expected SequenceType, got {type(constraint).__name__}")

    # Compute additional context for sub-templates that expect direct variables
    # (sequence_has_extension.j2, sequence_root_size.j2, sequence_size_func.j2)
    root_size_bits = constraint.preamble_bits
    field_type_names: list[str] = []
    for field in constraint.fields:
        field_bits = field.type.uper_bit_width
        if field_bits is None:
            raise ValueError(f"Field '{field.name}' has variable bit-width")
        root_size_bits += field_bits
        field_type_names.append(field.type_name)

    env = create_jinja_env()
    template = get_template(env, _SEQUENCE_TEMPLATE_NAME)

    return template.render(
        typedef=typedef,
        wire_format=wire_format,
        # Additional context for sub-templates that expect direct variables
        type_name=typedef.name,
        root_size_bits=root_size_bits,
        field_type_names=field_type_names,
    )


def _generate_choice(typedef: ASN1TypeDefinition, spec: J2735Specification) -> str:
    """Generate C code for a CHOICE type.

    Internal helper for generate_dataframe(). Uses single I/O pattern:
    read max bits once, then pure computation.

    Passes typedef directly to template, computing additional values inline.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_dataframe("ApproachOrLane", spec)
        >>> "J2735_APPROACH_OR_LANE_RAW_READ" in code
        True
        >>> "J2735_APPROACH_OR_LANE_WHICH" in code
        True
    """
    # Type narrowing for mypy - we know it's ChoiceType because generate_dataframe checked
    choice = typedef.constraint
    if not isinstance(choice, ChoiceType):
        raise TypeError(f"Expected ChoiceType, got {type(choice).__name__}")

    # Get index_bits from ChoiceType.uper_bit_width
    index_bits = choice.uper_bit_width
    if index_bits is None:
        raise ValueError(
            f"Extensible CHOICE type '{typedef.name}' is not yet supported. "
            "Phase 3b will add extensible CHOICE support."
        )

    # Resolve each alternative's bit-width by looking up the type reference
    # Build list of dicts with resolved info for template
    alternatives: list[_ChoiceAlternativeDict] = []
    max_alt_bits = 0
    for idx, (alt_name, type_ref) in enumerate(choice.alternatives.items()):
        alt_typedef = spec.lookup_type(type_ref)
        if alt_typedef is None:
            raise ValueError(f"Alternative '{alt_name}' references unknown type '{type_ref}'")
        if alt_typedef.constraint is None:
            raise ValueError(f"Alternative '{alt_name}' type '{type_ref}' has no constraint")
        alt_bit_width = alt_typedef.constraint.uper_bit_width
        if alt_bit_width is None:
            raise ValueError(f"Alternative '{alt_name}' type '{type_ref}' has variable bit-width")
        alternatives.append(
            {
                "name": alt_name,
                "type_ref": type_ref,
                "bit_width": alt_bit_width,
                "index": idx,
                "shift": 0,  # Computed below
                "needs_shift": False,  # Computed below
            }
        )
        max_alt_bits = max(max_alt_bits, alt_bit_width)

    # Calculate max wire bits
    max_wire_bits = index_bits + max_alt_bits

    # Validate single I/O pattern is possible
    if max_wire_bits > _MAX_SINGLE_IO_BITS:
        raise ValueError(
            f"CHOICE type '{typedef.name}' has max_wire_bits={max_wire_bits}, "
            f"which exceeds single I/O limit of {_MAX_SINGLE_IO_BITS} bits. "
            "Two-step pattern required (not yet implemented)."
        )

    # Find max alternative and compute shifts
    max_alternative = max(alternatives, key=lambda a: a["bit_width"])
    for alt in alternatives:
        alt["shift"] = max_wire_bits - index_bits - alt["bit_width"]
        alt["needs_shift"] = alt["shift"] > 0

    template = get_template(create_jinja_env(), _CHOICE_TEMPLATE_NAME)

    return template.render(
        typedef=typedef,  # Pass typedef directly, like SEQUENCE does
        alternatives=alternatives,
        index_bits=index_bits,
        max_wire_bits=max_wire_bits,
        max_alternative=max_alternative,
    )


# =============================================================================
# CLI
# =============================================================================


def main() -> None:
    """Command-line interface for Data Frame generation."""
    # pylint: disable=import-outside-toplevel
    # Imports here to avoid loading argparse/spec_parser when module is imported
    from argparse import ArgumentParser

    from .j2735_spec_parser import parse_spec_file

    parser = ArgumentParser(description="Generate C header for J2735 Data Frames")
    parser.add_argument("spec_file", type=Path, help="J2735 specification file")
    parser.add_argument(
        "type_name",
        help="Data Frame type name (e.g., BSMcoreData, ApproachOrLane)",
    )
    parser.add_argument("-o", "--output", type=Path, help="Output file (default: stdout)")
    args = parser.parse_args()

    spec = parse_spec_file(args.spec_file)
    code = generate_dataframe(args.type_name, spec)

    if args.output:
        args.output.write_text(code)
        print(f"Generated: {args.output}")
    else:
        print(code)


if __name__ == "__main__":
    main()
