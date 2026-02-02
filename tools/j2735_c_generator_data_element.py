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
J2735 Data Element C Code Generator.

Generates zero-copy C header files for J2735 Data Element types (BIT STRING,
INTEGER, ENUMERATED, etc.). Data Elements use the DE_ prefix in J2735 as they
represent atomic/primitive types.

Example usage:
    from tools.j2735_c_generator_data_element import generate_data_element
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")

    # BIT STRING types
    code = generate_data_element("VehicleEventFlags", spec)
"""

from .j2735_c_generator_jinja import create_jinja_env, get_template
from .j2735_spec_constraints import BitStringConstraint
from .j2735_spec_parser import ASN1TypeClass, ASN1TypeDefinition, J2735Specification

_BITSTRING_TEMPLATE_NAME = "assemble_de_bitstring.j2"


def generate_data_element(type_name: str, spec: J2735Specification) -> str:
    """Generate complete C header file for a Data Element.

    This is the main entry point for Data Element code generation. Dispatches
    to the appropriate internal generator based on the ASN.1 type class.

    Supported types:
        - BIT STRING: Fixed-size bit fields with named bits

    Args:
        type_name: Name of the type (e.g., "VehicleEventFlags").
        spec: The parsed J2735 specification.

    Returns:
        Complete C header file content as a string.

    Raises:
        ValueError: If type_name is not found.
        ValueError: If type is not a supported Data Element type.
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")

    if typedef.type_class == ASN1TypeClass.BIT_STRING:
        return _generate_bitstring(typedef)
    raise ValueError(
        f"Type '{type_name}' is {typedef.type_class.name}, which is not a supported "
        f"Data Element type. Supported types: BIT_STRING"
    )


def _generate_bitstring(typedef: ASN1TypeDefinition) -> str:
    """Generate C code for a BIT STRING type.

    Internal helper for generate_data_element().

    Args:
        typedef: The ASN.1 type definition for the BIT STRING.

    Returns:
        Complete C header file content as a string.

    Raises:
        TypeError: If constraint is not BitStringConstraint.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> typedef = spec.lookup_type("VehicleEventFlags")
        >>> code = _generate_bitstring(typedef)
        >>> "J2735_VEHICLE_EVENT_FLAGS_GET_EVENT_HAZARD_LIGHTS" in code
        True
    """
    # Type narrowing for mypy - validate constraint type
    bitstring = typedef.constraint
    if not isinstance(bitstring, BitStringConstraint):
        raise TypeError(
            f"Expected BitStringConstraint for BIT STRING type, got {type(bitstring).__name__}"
        )

    template = get_template(create_jinja_env(), _BITSTRING_TEMPLATE_NAME)

    return template.render(typedef=typedef)
