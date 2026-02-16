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

Generates zero-copy C header files for J2735 Data Frame types (SEQUENCE).
Both type classes use the DF_ prefix in J2735 as they represent composite structures.

Example usage:
    from tools.j2735_c_generator_data_frame import generate_data_frame
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")

    # SEQUENCE types
    code = generate_data_frame("BSMcoreData", spec)
"""

from .j2735_c_generator_jinja import (
    create_jinja_env,
    get_template,
)
from .j2735_c_generator_wire_format import get_sequence_variants
from .j2735_spec_constraints import SequenceType
from .j2735_spec_parser import (
    ASN1TypeClass,
    ASN1TypeDefinition,
    J2735Specification,
)

_SEQUENCE_TEMPLATE_NAME = "assemble_df_sequence.j2"

# Maximum wire bits for single I/O pattern (J2735_READ_BITS limit at worst alignment)
_MAX_SINGLE_IO_BITS = 57


def generate_data_frame(type_name: str, spec: J2735Specification) -> str:
    """Generate complete C header file for a Data Frame.

    This is the main entry point for Data Element code generation. Dispatches
    to the appropriate internal generator based on the ASN.1 type class.

    Supported types:
        - SEQUENCE: Struct container with field access macros.

    Args:
        type_name: Name of the type (e.g., "BSMcoreData", "ApproachOrLane").
        spec: The parsed J2735 specification.

    Returns:
        Complete C header file content as a string.

    Raises:
        ValueError: If type_name is not found.
        ValueError: If type is not a supported Data Frame type.
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")

    if typedef.type_class == ASN1TypeClass.SEQUENCE:
        return _generate_sequence(typedef)
    raise ValueError(
        f"Type '{type_name}' is {typedef.type_class.name}, which is not a supported "
        f"Data Frame type. Supported types: SEQUENCE"
    )


def _generate_sequence(typedef: ASN1TypeDefinition) -> str:
    """Generate C code for a SEQUENCE type.

    Internal helper for generate_data_frame().

    Args:
        typedef: The ASN.1 type definition for the SEQUENCE.

    Returns:
        Complete C header file content as a string.

    Raises:
        TypeError: If constraint is not SequenceType.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> typedef = spec.lookup_type("BSMcoreData")
        >>> code = _generate_sequence(typedef)
        >>> "J2735_INTERNAL_PREFIX_BITS_BSM_CORE_DATA" in code
        True
        >>> "J2735_BSM_CORE_DATA_GET_MSG_CNT" in code
        True
    """
    # Type narrowing for mypy - validate constraint type
    sequence = typedef.constraint
    if not isinstance(sequence, SequenceType):
        raise TypeError(
            f"Expected SequenceType for SEQUENCE type, " f"got {type(sequence).__name__}"
        )

    # Wire format variants for documentation tables
    variants = get_sequence_variants(sequence)

    template = get_template(create_jinja_env(), _SEQUENCE_TEMPLATE_NAME)

    return template.render(
        typedef=typedef,
        variants=variants,
        opt_count=sequence.optional_count,
    )
