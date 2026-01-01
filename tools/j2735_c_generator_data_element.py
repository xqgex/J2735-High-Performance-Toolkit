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
TODO
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from .j2735_c_generator_jinja import create_jinja_env, get_template
from .j2735_spec_constraints import BitStringConstraint

if TYPE_CHECKING:
    from .j2735_spec_parser import J2735Specification

_TEMPLATE_NAME = "assemble_de.j2"


def generate_data_element(type_name: str, spec: J2735Specification) -> str:
    """Generate a complete C header file for a J2735 Data Element.

    Args:
        type_name: The ASN.1 type name (e.g., "VehicleEventFlags").
        spec: The parsed J2735 specification.

    Returns:
        Complete C header file content as a string.

    Raises:
        TypeError: If the type is not a supported constraint type.
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")
    if not isinstance(typedef.constraint, BitStringConstraint):
        raise TypeError(
            f"generate_data_element currently only supports BitStringConstraint, "
            f"got {type(typedef.constraint).__name__}"
        )

    # Create Jinja environment and add pow() global for mask calculations
    env = create_jinja_env()
    template = get_template(env, _TEMPLATE_NAME)

    return template.render(
        # Type identification
        type_name=type_name,
        data_type=typedef.type_class.name.lower(),
        # Constraint properties
        is_extensible=typedef.constraint.is_extensible,
        root_size=typedef.constraint.root_size,
        ext_bits=typedef.constraint.ext_bits,
        # Wire encoding sizes
        read_bits=typedef.constraint.read_bits,
        # Named bits for accessor generation
        named_bits=typedef.constraint.named_bits,
    )
