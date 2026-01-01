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

Generates zero-copy C struct definitions with Doxygen wire format documentation
from parsed J2735 SEQUENCE types.

Example usage:
    from tools.j2735_dataframe_generator import generate_dataframe
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")
    code = generate_dataframe("BSMcoreData", spec)
    print(code)
"""

from __future__ import annotations

from pathlib import Path

from .j2735_c_generator_jinja import create_jinja_env, get_template
from .j2735_c_generator_wire_format import compute_wire_format
from .j2735_spec_parser import ASN1TypeClass, J2735Specification

_DATAFRAME_TEMPLATE_NAME = "dataframe_struct.j2"


def generate_dataframe(type_name: str, spec: J2735Specification) -> str:
    """Generate complete C struct definition for a Data Frame.

    This is the main entry point for Data Frame code generation.

    Args:
        type_name: Name of the SEQUENCE type (e.g., "BSMcoreData").
        spec: The parsed J2735 specification.

    Returns:
        Complete C struct definition with Doxygen documentation.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_dataframe("BSMcoreData", spec)
        >>> "J2735_BSMcoreData_t" in code
        True
        >>> "290 bits" in code
        True
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")
    if typedef.type_class != ASN1TypeClass.SEQUENCE:
        raise ValueError(f"Type '{type_name}' is not a SEQUENCE")

    wire_format = compute_wire_format(typedef)
    if not wire_format:
        raise ValueError(f"Cannot compute wire format for '{type_name}'")

    env = create_jinja_env()
    template = get_template(env, _DATAFRAME_TEMPLATE_NAME)

    return template.render(
        typedef=typedef,
        wire_format=wire_format,
    )


# =============================================================================
# CLI
# =============================================================================


def main() -> None:
    """Command-line interface."""
    from argparse import ArgumentParser

    from .j2735_spec_parser import parse_spec_file

    parser = ArgumentParser(description="Generate C struct for J2735 Data Frame")
    parser.add_argument("spec_file", type=Path, help="J2735 specification file")
    parser.add_argument("type_name", help="SEQUENCE type name (e.g., BSMcoreData)")
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
