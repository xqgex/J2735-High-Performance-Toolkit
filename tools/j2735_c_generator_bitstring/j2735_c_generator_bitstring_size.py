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
J2735 BIT STRING Size Generator.

Generates public C macro for getting wire size of a BIT STRING in bits.

Pattern: J2735_{TYPE}_SIZE(buf)

Example output:
    #define J2735_VEHICLE_EVENT_FLAGS_SIZE(buf) \\
      (J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS( \\
           J2735_INTERNAL_RAW_READ_VEHICLE_EVENT_FLAGS(buf)) \\
           ? 22U \\
           : 14U)
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from tools.j2735_c_generator_jinja import create_jinja_env, get_template
from tools.j2735_spec_constraints import BitStringConstraint
from tools.j2735_spec_parser import ASN1TypeClass

if TYPE_CHECKING:
    from tools.j2735_spec_parser import J2735Specification

_TEMPLATE_NAME = "bitstring/bitstring_size.j2"


def generate_bitstring_size(type_name: str, spec: J2735Specification) -> str:
    """Generate public C macro for getting wire size in bits.

    Args:
        type_name: Name of the BIT STRING type (e.g., "VehicleEventFlags").
        spec: The parsed J2735 specification.

    Returns:
        C code with size macro definition.

    Raises:
        ValueError: If type_name is not found or not a BIT STRING.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_bitstring_size("VehicleEventFlags", spec)
        >>> "J2735_VEHICLE_EVENT_FLAGS_SIZE" in code
        True
        >>> "22U" in code
        True
        >>> "14U" in code
        True
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")
    if typedef.type_class != ASN1TypeClass.BIT_STRING:
        raise ValueError(f"Type '{type_name}' is not a BIT STRING")
    if not isinstance(typedef.constraint, BitStringConstraint):
        raise ValueError(f"Type '{type_name}' has unexpected constraint type")

    env = create_jinja_env()
    template = get_template(env, _TEMPLATE_NAME)

    return template.render(
        # Type identification
        type_name=type_name,
        # Constraint properties
        is_extensible=typedef.constraint.is_extensible,
        root_size=typedef.constraint.root_size,
        # Wire encoding sizes
        read_bits=typedef.constraint.read_bits,
    )
