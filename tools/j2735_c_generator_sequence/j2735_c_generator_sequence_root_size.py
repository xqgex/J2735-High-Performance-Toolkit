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
J2735 Root Size Constants C Code Generator.

Generates #define J2735_ROOT_SIZE_BITS_<SEQUENCE> constants for extensible
SEQUENCE types. The root size is the bit count of the preamble plus all
non-OPTIONAL root fields, used to know where extensions start.

Example usage:
    from tools.j2735_c_generator_root_size import generate_root_size
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")
    code = generate_root_size("PathPrediction", spec)
    print(code)
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from tools.j2735_c_generator_jinja import create_jinja_env, get_template
from tools.j2735_spec_constraints import SequenceType
from tools.j2735_spec_parser import ASN1TypeClass

if TYPE_CHECKING:
    from tools.j2735_spec_parser import J2735Specification

_TEMPLATE_NAME = "sequence/sequence_root_size.j2"


def generate_sequence_root_size(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define constant for root size of an extensible SEQUENCE.

    Only generates output for extensible SEQUENCE types with fixed-width
    root components (no OPTIONAL fields).

    Args:
        type_name: Name of the SEQUENCE type (e.g., "PathPrediction").
        spec: The parsed J2735 specification.

    Returns:
        C code with #define constant, or empty string if not applicable.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_sequence_root_size("PathPrediction", spec)
        >>> "J2735_ROOT_SIZE_BITS_PATH_PREDICTION" in code
        True
        >>> "J2735_PREFIX_BITS_PATH_PREDICTION" in code
        True
        >>> "J2735_BW_RADIUS_OF_CURVATURE" in code
        True
        >>> generate_sequence_root_size("BSMcoreData", spec)
        ''
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")
    if typedef.type_class != ASN1TypeClass.SEQUENCE:
        raise ValueError(f"Type '{type_name}' is not a SEQUENCE")
    if not isinstance(typedef.constraint, SequenceType):
        raise ValueError(f"Type '{type_name}' has no SequenceType constraint")

    # Skip if not extensible
    if not typedef.constraint.is_extensible:
        return ""

    # Skip if root size is not fixed (has OPTIONAL fields or variable-width fields)
    root_size = typedef.constraint.root_uper_bit_width
    if root_size is None:
        return ""

    env = create_jinja_env()
    template = get_template(env, _TEMPLATE_NAME)

    # Build list of field type names for symbolic expression
    field_type_names = [
        field.type_name for field in typedef.constraint.fields if not field.is_optional
    ]

    return template.render(
        type_name=type_name,
        root_size_bits=root_size,
        field_type_names=field_type_names,
    )
