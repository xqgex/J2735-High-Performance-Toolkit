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
J2735 Sequence Has-Field Macros C Code Generator.

Generates #define J2735_<SEQUENCE>_HAS_<FIELD>(buf) macros for OPTIONAL fields.
These check the presence bitmap (preamble) to determine if a field is present.

Example usage:
    from tools.j2735_c_generator_has_field import generate_has_field
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")
    code = generate_has_field("IntersectionReferenceID", spec)
    print(code)
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from tools.j2735_c_generator_jinja import create_jinja_env, get_template
from tools.j2735_spec_constraints import SequenceType
from tools.j2735_spec_parser import ASN1TypeClass

if TYPE_CHECKING:
    from tools.j2735_spec_parser import J2735Specification

_TEMPLATE_NAME = "sequence/sequence_has_field.j2"


def generate_sequence_has_field(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define has-field macros for OPTIONAL fields in a SEQUENCE.

    Only generates output for SEQUENCE types that have OPTIONAL fields.
    Returns empty string for SEQUENCEs with no OPTIONAL fields.

    Args:
        type_name: Name of the SEQUENCE type (e.g., "IntersectionReferenceID").
        spec: The parsed J2735 specification.

    Returns:
        C code with #define has-field macros, or empty string if none.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_sequence_has_field("IntersectionReferenceID", spec)
        >>> "J2735_INTERSECTION_REFERENCE_ID_HAS_REGION" in code
        True
        >>> "J2735_INTERNAL_HAS_FIELD" in code
        True
        >>> code = generate_sequence_has_field("BSMcoreData", spec)
        >>> code.strip()  # No OPTIONAL fields, empty output
        ''
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")
    if typedef.type_class != ASN1TypeClass.SEQUENCE:
        raise ValueError(f"Type '{type_name}' is not a SEQUENCE")
    if not isinstance(typedef.constraint, SequenceType):
        raise ValueError(f"Type '{type_name}' has no SequenceType constraint")

    # Skip if no OPTIONAL fields
    if typedef.constraint.optional_count == 0:
        return ""

    env = create_jinja_env()
    template = get_template(env, _TEMPLATE_NAME)

    return template.render(typedef=typedef)
