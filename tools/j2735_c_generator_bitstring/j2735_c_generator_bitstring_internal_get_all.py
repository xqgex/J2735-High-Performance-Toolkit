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
J2735 BIT STRING Internal Get All Flags Generator.

Generates C macro for extracting all flag bits from pre-read raw value.

Pattern: J2735_INTERNAL_GET_ALL_{TYPE}(raw)

Example output:
    #define J2735_INTERNAL_GET_ALL_VEHICLE_EVENT_FLAGS(raw22) \\
      (J2735_INTERNAL_IS_EXTENSION_VEHICLE_EVENT_FLAGS(raw22) \\
           ? ((uint16_t)((raw22) & 0x3FFFU)) \\
           : ((uint16_t)(((raw22) >> 8U) & 0x1FFFU)))
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from tools.j2735_c_generator_jinja import create_jinja_env, get_template
from tools.j2735_spec_constraints import BitStringConstraint
from tools.j2735_spec_parser import ASN1TypeClass

if TYPE_CHECKING:
    from tools.j2735_spec_parser import J2735Specification

_TEMPLATE_NAME = "bitstring/bitstring_internal_get_all.j2"


def generate_bitstring_internal_get_all(type_name: str, spec: J2735Specification) -> str:
    """Generate C macro for extracting all flag bits.

    Args:
        type_name: Name of the BIT STRING type (e.g., "VehicleEventFlags").
        spec: The parsed J2735 specification.

    Returns:
        C code with get all flags macro definition.

    Raises:
        ValueError: If type_name is not found or not a BIT STRING.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_bitstring_internal_get_all("VehicleEventFlags", spec)
        >>> "J2735_INTERNAL_GET_ALL_VEHICLE_EVENT_FLAGS" in code
        True
        >>> "0x3FFFU" in code
        True
        >>> "0x1FFFU" in code
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
        ext_bits=typedef.constraint.ext_bits,
        # Wire encoding sizes
        read_bits=typedef.constraint.read_bits,
    )
