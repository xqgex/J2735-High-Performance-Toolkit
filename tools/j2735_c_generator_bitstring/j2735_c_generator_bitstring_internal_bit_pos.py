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
J2735 BIT STRING Internal Bit Position Constants Generator.

Generates C #define constants for ASN.1 BIT STRING named bit positions.

Pattern: J2735_INTERNAL_BIT_{TYPE}_{FLAG}

Example output:
    #define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_HAZARD_LIGHTS 0U
    #define J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_EVENT_JACK_KNIFE    13U
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from tools.j2735_c_generator_jinja import create_jinja_env, get_template
from tools.j2735_spec_constraints import BitStringConstraint
from tools.j2735_spec_parser import ASN1TypeClass

if TYPE_CHECKING:
    from tools.j2735_spec_parser import J2735Specification

_TEMPLATE_NAME = "bitstring/bitstring_internal_bit_pos.j2"


def generate_bitstring_internal_bit_pos(type_name: str, spec: J2735Specification) -> str:
    """Generate C #define constants for BIT STRING bit positions.

    Args:
        type_name: Name of the BIT STRING type (e.g., "VehicleEventFlags").
        spec: The parsed J2735 specification.

    Returns:
        C code with #define constants for each named bit.

    Raises:
        ValueError: If type_name is not found or not a BIT STRING.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_bitstring_internal_bit_pos("VehicleEventFlags", spec)
        >>> "J2735_INTERNAL_BIT_VEHICLE_EVENT_FLAGS_" in code
        True
        >>> "0U" in code
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
        # Named bits for accessor generation
        named_bits=typedef.constraint.named_bits,
    )
