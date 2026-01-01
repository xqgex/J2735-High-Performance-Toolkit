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
J2735 Has-Extension Macro C Code Generator.

Generates #define J2735_<SEQUENCE>_HAS_EXTENSION(buf) macros for extensible
SEQUENCE types. These wrap the core J2735_INTERNAL_HAS_EXTENSION macro with a
type-specific name for better code readability.

Example usage:
    from tools.j2735_c_generator_has_extension import generate_has_extension
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")
    code = generate_has_extension("PathPrediction", spec)
    print(code)
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from tools.j2735_c_generator_jinja import create_jinja_env, get_template
from tools.j2735_spec_constraints import SequenceType
from tools.j2735_spec_parser import ASN1TypeClass

if TYPE_CHECKING:
    from tools.j2735_spec_parser import J2735Specification

_TEMPLATE_NAME = "sequence/sequence_has_extension.j2"


def generate_sequence_has_extension(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define macro for checking extension bit in a SEQUENCE.

    Only generates output for extensible SEQUENCE types.

    Args:
        type_name: Name of the SEQUENCE type (e.g., "PathPrediction").
        spec: The parsed J2735 specification.

    Returns:
        C code with #define macro, or empty string if not extensible.

    Raises:
        ValueError: If type_name is not found or not a SEQUENCE.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_sequence_has_extension("PathPrediction", spec)
        >>> "@brief Check if PathPrediction has extension" in code
        True
        >>> "#define J2735_PATH_PREDICTION_HAS_EXTENSION(buf)" in code
        True
        >>> "J2735_INTERNAL_HAS_EXTENSION(buf)" in code
        True
        >>> generate_sequence_has_extension("BSMcoreData", spec)
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

    env = create_jinja_env()
    template = get_template(env, _TEMPLATE_NAME)

    return template.render(type_name=type_name)
